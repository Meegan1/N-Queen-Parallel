//
// Created by Jake Meegan on 14/11/2019.
//

#include "Solver.h"

Solver::Solver(int n_threads) : n_threads(n_threads), start_time(0), end_time(0), is_complete(false) {
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

    int sol = 0;
    while(!futures.isEmpty()) {
        sol += futures.top().get();
        futures.pop_back();
    }

    std::cout << "Number of Solutions: " << sol << std::endl;

    time(&end_time);
    std::cout << "Total Time Elapsed: " << end_time - start_time << "s" << std::endl;

}

void Solver::wait_and_solve() {
    while(!is_complete) {
        std::unique_lock<std::mutex> gate(m);

        cv.wait(gate,
                [this] { return !states.isEmpty() || is_complete; });

        if(is_complete)
            return;
        ProblemState c_state = states.pop();
        gate.unlock();

        solve(c_state);
    }
}


int Solver::solve(ProblemState c_state) {
    nqueen(c_state.ld, c_state.cols, c_state.rd, 6);
}

void Solver::nqueen(chessboard ld, chessboard cols, chessboard rd, int level) {
    int sol = 0;
    std::promise<int> &promise = promises.emplace_and_get();
    futures.push(promise.get_future());

    if (cols == all) {                            // A solution is found
        promise.set_value(1);
        return;
    }

    chessboard pos = ~(ld | cols | rd) & all;  // Possible posstions for the queen on the current row
    chessboard next;

    while (pos != 0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
        next = pos & (-pos);                    // next possible position
        pos -= next;                             // update the possible position

        if(level <= 5) {
            nqueen((ld | next) << 1, cols | next, (rd | next) >> 1, level + 1); // recursive call for the `next' position
        }
        else {
            states.push(ProblemState((ld | next) << 1, cols | next, (rd | next) >> 1));

            std::unique_lock<std::mutex> gate(m);
            cv.notify_one();
            gate.unlock();
        }
    }

    promise.set_value(sol);
}


int Solver::solve(int n_queens) {
    time(&start_time);
    all = (1 << n_queens) - 1;            // set N bits on, representing number of columns

    nqueen(0, 0, 0, 0);

    is_complete = true;
    cv.notify_all();
    return 0;
}
