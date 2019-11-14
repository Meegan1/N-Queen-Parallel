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
    std::promise<int> sol;

    ProblemState(chessboard ld, chessboard cols, chessboard rd) : ld(ld), cols(cols), rd(rd) {}
};

class Solver {
public:
    explicit Solver(int n_queens, int n_threads);

    ~Solver();

    static void loop() {
        for(int i = 0; i < 1000; i++) {
//            std::cout << i << std::endl;
        }
    }
private:
    int n_threads;
    int n_queens;
    time_t start_time;
    time_t end_time;

    std::vector<std::thread> threads;
    std::mutex m;
    std::condition_variable cv;
    std::atomic<bool> isComplete;
    ThreadSafeStack<int> states;

    void wait_and_solve();
};

//class SolverThread {
//public:
//    void operator()() {
//        wait_and_solve();
//    }
//    void wait_and_solve() {
////        states.pop();
//    };
//};


#endif //ASSIGNMENT_1_SOLVER_H
