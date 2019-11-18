#include <iostream>
#include <queue>
#include <future>

typedef int chessboard;

template <typename T>
class ThreadSafeQueue {
public:
    explicit ThreadSafeQueue() = default;


    void push(T &&new_value) {
        std::lock_guard<std::mutex> lock(gate);
        queue.push(std::move(new_value));
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lock(gate);
        if (queue.empty()) return false;
        val(std::move(queue.front()));
        queue.pop();
        return true;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(gate);
        return queue.empty();
    }

private:
    std::queue<T> queue;
    std::mutex gate;
};

struct ProblemState {
    chessboard ld, cols, rd;
    std::promise<int> promise;

    explicit ProblemState(chessboard ld, chessboard cols, chessboard rd)
            : ld(ld), cols(cols), rd(rd) {}

    ProblemState(const ProblemState & other) = delete;
    ProblemState(ProblemState && other) : ld(other.ld), cols(other.cols), rd(other.rd), promise(std::move(other.promise)) {}

    void operator()(ProblemState && other) {
        ld = other.ld;
        cols = other.cols;
        rd = other.rd;
        promise = std::move(other.promise);
    }

    ~ProblemState() = default;
};



class Solver {
public:
    /*
     * Constructor for Solver
     */
    Solver(int n_threads, int n_level) : n_threads(n_threads), start_time(0), end_time(0), n_level(n_level) {
        // spawn threads
        for (int i = 0; i < n_threads; i++) {
            threads.emplace_back(&Solver::wait_and_solve, this); // add thread to vector data structure

            if (!threads[i].joinable()) // check thread was created
                throw std::logic_error("no thread");
        }
    }

    /*
     * Destructor for Solver
     */
    ~Solver() {
        shutdown(); // execute shutdown of threads
        time(&end_time); // get end time of algorithm
        std::cout << "Total Time Elapsed: " << end_time - start_time << "s" << std::endl; // print out time elapsed

    }

    /*
     * Solve for n queens
     */
    void solve(int queens) {
        n_queens = queens; // set n-queens
        time(&start_time); // get starting time

        all = (1 << n_queens) - 1;            // set N bits on, representing number of columns

        ProblemState problem(0, 0, 0); // create initial problem state
        std::shared_future<int> future(problem.promise.get_future()); // get future for initial state

        states.push(std::move(problem)); // push state to the queue, triggering threads to solve the problem

        std::cout << "Number of Solutions: " << future.get() << std::endl; // get the solution from the future and print to console
    }
private:
    int n_threads; // n number of threads to spawn
    int n_level; // n number of levels until switching to sequential algorithm
    int n_queens; // n number of queens to solve for
    time_t start_time; // start time of program
    time_t end_time; // end time of program

    chessboard all;

    std::vector<std::thread> threads; // vector of threads
    ThreadSafeQueue<ProblemState> states; // queue of problem states

    bool m_shutdown; // breaks threads from infinite loop
    std::condition_variable m_condition; // conditional variable for when problem state is pushed to queue
    std::mutex m_mutex; // mutex for queue

    /*
     * Starting function for threads to continually solve problems pushed to the queue
     */
    void wait_and_solve() {
        while(!m_shutdown) { // infinite loop until shutdown request
            try_and_solve(); // try and solve a problem state
        }
    }

    /*
     * try and solve a problem
     */
    void try_and_solve() {
        ProblemState state(0, 0, 0); // generate blank temp state
        bool success; // create bool for try_pop
        {
            std::unique_lock<std::mutex> lock(m_mutex); // lock mutex
            if(states.isEmpty()) { // if states is empty
                m_condition.wait(lock, [this]{return states.isEmpty() || m_shutdown;}); // wait for notification that states isn't empty OR shutdown request
            }
            success = states.try_pop(state); // try and pop a state from the queue
        }
        if(success) { // if state successfully pop
            solve(std::move(state)); // solve problem state
        }
    }


    /*
     * Parallel version of algorithm
     */
    void solve(ProblemState &&state) {
        if (state.cols == all) {                            // A solution is found
            state.promise.set_value(1); // set value of current problem state to 1
            return;
        }

        chessboard pos = ~(state.ld | state.cols | state.rd) & all;  // Possible positions for the queen on the current row
        chessboard next;

        // get current level of problem
        std::bitset<sizeof(chessboard) * CHAR_BIT> b(state.cols);
        int level = b.count();

        int sol = 0;

        // if current remaining levels is less than n_levels argument, run sequentially
        if(n_queens - level <= n_level) {
            sol += seq_nqueen(state.ld, state.cols, state.rd);
        }
        else { // else calculate next problem and push to ProblemState queue
            std::queue<std::shared_future<int>> futures_queue; // queue of futures

            while (pos != 0) {                          // Iterate over all possible positions and push to states queue
                next = pos & (-pos);                    // next possible position
                pos -= next;                             // update the possible position


                ProblemState problem((state.ld | next) << 1, state.cols | next, (state.rd | next) >> 1); // create problem state
                futures_queue.emplace(problem.promise.get_future()); // get future of problem state and push to futures queue
                states.push(std::move(problem)); // push new problem to queue of problem states
            }

            // loop through futures queue
            while(!futures_queue.empty()) {
                // get future
                std::shared_future<int> future(futures_queue.front());
                futures_queue.pop();

                while (future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready) { // while future not ready
                    try_and_solve(); // solve another problem from problem state queue
                }
                sol += future.get(); // get future when ready and add result to sol

            }
        }

        state.promise.set_value(sol); // set value of promise for current state to sol
    }


    /*
     * Sequential version of algorithm
     */
    int seq_nqueen(chessboard ld, chessboard cols, chessboard rd) {
        int sol = 0;

        if (cols == all)                            // A solution is found
            return 1;

        chessboard pos = ~(ld | cols | rd) & all;  // Possible posstions for the queen on the current row
        chessboard next;
        while (pos !=
               0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
            next = pos & (-pos);                    // next possible position
            pos -= next;                             // update the possible position
            sol += seq_nqueen((ld | next) << 1, cols | next, (rd | next) >> 1); // recursive call for the `next' position
        }
        return sol;
    }

    /*
     * Shutdown threads
     */
    void shutdown() {
        // unblock all threads waiting
        m_shutdown = true;
        m_condition.notify_all();

        // loop through threads and join
        for (int i = 0; i < n_threads; i++) {
            if (threads[i].joinable())
                threads[i].join();
        }
    }
};


int main(int argc, char **argv) {
    if (argc < 4) {
        std::cout << "You have to provide: \n 1) Number of Queens \n 2) Number of threads \n 3) Number of levels left before switching to sequential" << std::endl;
        return 0;
    }

    int qn = std::stoi(argv[1]); // get n queens
    int tn = std::stoi(argv[2]); // get n threads
    int ln = std::stoi(argv[3]); // get n levels

    Solver solver(tn, ln); // create solver object with n threads and n levels before sequential
    solver.solve(qn); // solve for n queens

    return 0;
}