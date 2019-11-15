//
// Created by Jake Meegan on 14/11/2019.
//

#ifndef ASSIGNMENT_1_SOLVER_H
#define ASSIGNMENT_1_SOLVER_H


#include <thread>
#include <vector>
#include <iostream>
#include <future>
#include "ThreadSafeStack.h"

typedef int chessboard;
struct ProblemState {
    chessboard ld, cols, rd;
    std::promise<int> promise;

    explicit ProblemState(chessboard ld, chessboard cols, chessboard rd)
            : ld(ld), cols(cols), rd(rd) {}
};

class Solver {
public:
    explicit Solver(int n_threads);
    ~Solver();

    int solve(int n_queens);

private:
    int n_threads;
    time_t start_time;
    time_t end_time;

    chessboard all;

    std::vector<std::thread> threads;
    std::mutex m;
    std::condition_variable cv;
    bool is_complete;
    ThreadSafeStack<std::shared_ptr<ProblemState>> states;
    ThreadSafeStack<std::shared_future<int>> futures;

    void wait_and_solve();
    void solve(std::shared_ptr<ProblemState> c_state);
    void nqueen(const std::shared_ptr<ProblemState>& state, int level);
};

//class SolverThread {
//public:
//    void operator()() {
//        wait_and_solve();
//    }
//    void wait_and_solve() {
//        states.pop();
//    };
//};


#endif //ASSIGNMENT_1_SOLVER_H
