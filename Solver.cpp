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
    for (int i = 0; i < n_threads; i++) {
        threads[i].join();
    }

    time(&end_time);
    std::cout << "Total Time Elapsed: " << end_time - start_time << "s" << std::endl;

}

void Solver::wait_and_solve() {
    while(true) {
        std::shared_ptr<ProblemState> state;
        if(states.wait_and_pop(state))
            solve(state);
        else
            return;
    }
}


void Solver::solve(const std::shared_ptr<ProblemState> &c_state) {
    nqueen(c_state, 14);
}

int Solver::nqueen(const std::shared_ptr<ProblemState> &state, int level) {
    if (state->cols == all) {                            // A solution is found
        state->promise.set_value(1);
        return true;
    }

    chessboard pos = ~(state->ld | state->cols | state->rd) & all;  // Possible positions for the queen on the current row
    chessboard next;

    int sol = 0;

    while (pos != 0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
        next = pos & (-pos);                    // next possible position
        pos -= next;                             // update the possible position

        std::shared_ptr<ProblemState> problem(std::make_shared<ProblemState>(
                ProblemState((state->ld | next) << 1, state->cols | next, (state->rd | next) >> 1)));

        if (level < 13) {
            sol += nqueen(problem, level + 1); // recursive call for the `next' position
        } else {
            states.push(problem);
            std::shared_future<int> future(problem->promise.get_future());

            while (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
                wait_and_solve();
            }
            sol += future.get();
        }
    }

    state->promise.set_value(sol);
    return sol;
}


int Solver::solve(int n_queens) {
    time(&start_time);
    all = (1 << n_queens) - 1;            // set N bits on, representing number of columns

    std::shared_ptr<ProblemState> problem(std::make_shared<ProblemState>(0, 0, 0));
//    states.push(problem);
    nqueen(problem, 0);

    std::cout << "Number of Solutions: " << problem->promise.get_future().get() << std::endl;

    states.complete();
    return 0;
}
