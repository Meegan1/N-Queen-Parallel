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
        sol += (*futures.wait_and_pop()).get();
    }

    std::cout << "Number of Solutions: " << sol << std::endl;

    time(&end_time);
    std::cout << "Total Time Elapsed: " << end_time - start_time << "s" << std::endl;

}

void Solver::wait_and_solve() {
    while(!is_complete) {
        solve(*states.wait_and_pop());
    }
}


void Solver::solve(std::shared_ptr<ProblemState> c_state) {
    nqueen(c_state, 6);
}

void Solver::nqueen(const std::shared_ptr<ProblemState>& state, int level) {
    int sol = 0;

    if (state->cols == all) {                            // A solution is found
        state->promise.set_value(1);
        futures.push(state->promise.get_future());
        return;
    }

    chessboard pos = ~(state->ld | state->cols | state->rd) & all;  // Possible posstions for the queen on the current row
    chessboard next;

    while (pos != 0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
        next = pos & (-pos);                    // next possible position
        pos -= next;                             // update the possible position

        if(level <= 5) {
            std::shared_ptr<ProblemState> problem(std::make_shared<ProblemState>(ProblemState((state->ld | next) << 1, state->cols | next, (state->rd | next) >> 1)));
            nqueen(problem, level + 1); // recursive call for the `next' position
        }
        else {
            std::shared_ptr<ProblemState> problem(std::make_shared<ProblemState>(ProblemState((state->ld | next) << 1, state->cols | next, (state->rd | next) >> 1)));
            states.push(problem);

            std::unique_lock<std::mutex> gate(m);
            cv.notify_one();
            gate.unlock();
        }
    }
}


int Solver::solve(int n_queens) {
    time(&start_time);
    all = (1 << n_queens) - 1;            // set N bits on, representing number of columns

    std::shared_ptr<ProblemState> problem(std::make_shared<ProblemState>(0, 0, 0));
    nqueen(problem, 0);

//    is_complete = true;
//    cv.notify_all();
    return 0;
}
