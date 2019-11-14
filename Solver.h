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
    std::shared_future<int> sol;

    explicit ProblemState(chessboard ld, chessboard cols, chessboard rd) : ld(ld), cols(cols), rd(rd), sol() {}

//    ProblemState(const ProblemState &other) = delete;
//    ProblemState(ProblemState &other) : ld(other.ld), cols(other.cols), rd(other.rd), sol(std::move(other.sol)) {}
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
    std::atomic<bool> isComplete;
    ThreadSafeStack<ProblemState> states;

    void wait_and_solve();
    int solve(const std::shared_ptr<ProblemState>& c_state);
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
