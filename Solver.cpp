//
// Created by Jake Meegan on 14/11/2019.
//

#include "Solver.h"

Solver::Solver(int n_threads) : n_threads(n_threads), start_time(0), end_time(0) {
    // spawn threads
    for (int i = 0; i < n_threads; i++) {
        threads.emplace_back(&Solver::wait_and_solve, this);

        if (!threads[i].joinable())
            throw std::logic_error("no thread");
    }
}

Solver::~Solver() {
    shutdown();
    time(&end_time);
    std::cout << "Total Time Elapsed: " << end_time - start_time << "s" << std::endl;

}

void Solver::wait_and_solve() {
    while(!m_shutdown) {
        ProblemState state(0, 0, 0);
        bool success;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(states.isEmpty()) {
                m_condition.wait(lock, [this]{return states.isEmpty() || m_shutdown;});
            }
            success = states.try_pop(state);
        }
        if(success) {
            solve(std::move(state));
        }
    }
}


void Solver::solve(ProblemState &&state) {
    if (state.cols == all) {                            // A solution is found
        state.promise.set_value(1);
        return;
    }

    chessboard pos = ~(state.ld | state.cols | state.rd) & all;  // Possible positions for the queen on the current row
    chessboard next;

    std::bitset<sizeof(chessboard) * CHAR_BIT> b(state.cols);
    int level = b.count();

    int sol = 0;

    if(level >= 3) {
        sol += seq_nqueen(state.ld, state.cols, state.rd);
    }
    else {
        std::queue<std::shared_future<int>> futures_queue;

        while (pos != 0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
            next = pos & (-pos);                    // next possible position
            pos -= next;                             // update the possible position


            ProblemState problem((state.ld | next) << 1, state.cols | next, (state.rd | next) >> 1);

            futures_queue.emplace(problem.promise.get_future());
            states.push(std::move(problem));
        }

        while(!futures_queue.empty()) {
            std::shared_future<int> future(futures_queue.front());
            futures_queue.pop();

            while (future.wait_for(std::chrono::nanoseconds(35)) != std::future_status::ready) {
                ProblemState state(0, 0, 0);
                bool success;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    if(states.isEmpty()) {
                        m_condition.wait(lock, [this]{return states.isEmpty() || m_shutdown;});
                    }
                    success = states.try_pop(state);
                }
                if(success) {
                    solve(std::move(state));
                }
            }
            sol += future.get();

        }
    }

    state.promise.set_value(sol);
}


int Solver::seq_nqueen(chessboard ld, chessboard cols, chessboard rd) {
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


int Solver::solve(int n_queens) {
    time(&start_time);
    all = (1 << n_queens) - 1;            // set N bits on, representing number of columns

    ProblemState problem(0, 0, 0);
    std::shared_future<int> future(problem.promise.get_future());

    states.push(std::move(problem));

    std::cout << "Number of Solutions: " << future.get() << std::endl;


    return 0;
}

void Solver::shutdown() {
    m_shutdown = true;
    m_condition.notify_all();
    for (int i = 0; i < n_threads; i++) {
        if (threads[i].joinable())
            threads[i].join();
    }
}