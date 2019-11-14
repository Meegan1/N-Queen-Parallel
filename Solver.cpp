//
// Created by Jake Meegan on 14/11/2019.
//

#include "Solver.h"

Solver::Solver(int n_threads) : n_threads(n_threads), start_time(0), end_time(0), isComplete(false) {
    time(&start_time);

    // spawn threads
    for (int i = 0; i < n_threads; i++) {
        threads.emplace_back(&Solver::wait_and_solve, this);

        if (!threads[i].joinable())
            throw std::logic_error("no thread");
    }
}

Solver::~Solver() {
    for (int i = 0; i < n_threads; i++) {
        threads[i].join();
    }
    time(&end_time);
    std::cout << "Total Time Elapsed: " << end_time - start_time << "s" << std::endl;

}

void Solver::wait_and_solve() {
    while(!isComplete.load()) {
        std::shared_ptr<ProblemState> c_state;
        std::unique_lock<std::mutex> gate(m);
        cv.wait(gate,
                [this] { return !states.isEmpty(); });
        c_state = states.pop();
        gate.unlock();
        solve(c_state);

//        if(states.isEmpty())
//            isComplete.store(true);
    }
}


int Solver::solve(const std::shared_ptr<ProblemState>& c_state) {
    if(c_state->cols == all) {
        c_state->sol.set_value(1);
        return true;
    }

    chessboard pos = ~(c_state->ld | c_state->cols | c_state->rd) & all;  // Possible positions for the queen on the current row
    chessboard next;
    int sol = 0;


    while (pos != 0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
        next = pos & (-pos);                    // next possible position
        pos -= next;                             // update the possible position

        std::promise<int> p;
        ProblemState n_state((c_state->ld | next) << 1, c_state->cols | next, (c_state->rd | next) >> 1,
                             p);
        states.push(n_state); // recursive call for the `next' position

        cv.notify_one();

        sol += n_state.sol.get_future().get();

    }
//    c_state->sol.set_value(1);
    return sol;
}


int Solver::solve(int n_queens) {
    all = (1 << n_queens) - 1;            // set N bits on, representing number of columns

    std::lock_guard<std::mutex> gate(m);
    std::promise<int> p;
    ProblemState first_state(0, 0, 0, p);
    states.push(first_state);
    cv.notify_one();


    return 0;
}
