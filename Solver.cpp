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
        ProblemState state(0, 0, 0);
        if(states.wait_and_pop(state))
            solve(std::move(state));
        else
            return;
    }
}


void Solver::solve(ProblemState &&c_state) {
    nqueen(std::move(c_state), 0);
}

int Solver::nqueen(ProblemState &&state, int level, int is_sequential) {
    if (state.cols == all) {                            // A solution is found
        state.promise.set_value(1);
        return true;
    }

    chessboard pos = ~(state.ld | state.cols | state.rd) & all;  // Possible positions for the queen on the current row
    chessboard next;

    int sol = 0;

    std::queue<std::shared_future<int>> futures_queue;

    while (pos != 0) {                          // Iterate over all possible positions and solve the (N-1)-queen in each case
        next = pos & (-pos);                    // next possible position
        pos -= next;                             // update the possible position


        ProblemState problem((state.ld | next) << 1, state.cols | next, (state.rd | next) >> 1);

        if (level < 12) {
            sol += nqueen(std::move(problem), level + 1, true); // recursive call for the `next' position
        } else {
            futures_queue.emplace(problem.promise.get_future());
            states.push(std::move(problem));
        }
    }

    while(!futures_queue.empty()) {
        std::shared_future<int> future(futures_queue.front());
        futures_queue.pop();

        while (future.wait_for(std::chrono::nanoseconds(35)) != std::future_status::ready) {
            wait_and_solve();
        }
        sol += future.get();
    }

    if(!is_sequential)
        state.promise.set_value(sol);
    return sol;
}


int Solver::solve(int n_queens) {
    time(&start_time);
    all = (1 << n_queens) - 1;            // set N bits on, representing number of columns

    ProblemState problem(0, 0, 0);
    std::shared_future<int> future(problem.promise.get_future());

    states.push(std::move(problem));

    std::cout << "Number of Solutions: " << future.get() << std::endl;

    states.complete();
    return 0;
}
