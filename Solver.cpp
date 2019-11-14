//
// Created by Jake Meegan on 14/11/2019.
//

#include "Solver.h"

Solver::Solver(int n_queens, int n_threads) : n_queens(n_queens), n_threads(n_threads), start_time(0), end_time(0), isComplete(false) {
    time(&start_time);
    for (int i = 0; i < n_threads; i++) {
        threads.emplace_back(&Solver::wait_and_solve, this);

        if (!threads[i].joinable())
            throw std::logic_error("no thread");
    }

    for (int i = 0; i < 999; i++) {
        std::lock_guard<std::mutex> gate(m);
        states.push(i);
        cv.notify_one();
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
        std::unique_lock<std::mutex> gate(m);

        cv.wait(gate,
                [this] { return !states.isEmpty(); });

        std::shared_ptr<int> l = states.pop();
        std::cout << *l << std::endl;
        loop();

//        if(states.isEmpty())
//            isComplete.store(true);
    }
}
