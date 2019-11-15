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
    std::promise<int> *sol;

    explicit ProblemState(chessboard ld, chessboard cols, chessboard rd, std::promise<int> *sol)
            : ld(ld), cols(cols), rd(rd), sol(sol) {}
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
    ThreadSafeStack<ProblemState> states;

    void wait_and_solve();
    int solve(ProblemState c_state);
    std::shared_future<int> nqueen(chessboard ld, chessboard cols, chessboard rd, int level);
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
