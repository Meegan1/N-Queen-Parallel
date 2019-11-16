//
// Created by Jake Meegan on 14/11/2019.
//

#ifndef ASSIGNMENT_1_SOLVER_H
#define ASSIGNMENT_1_SOLVER_H


#include <thread>
#include <vector>
#include <iostream>
#include <future>
#include "ThreadSafeQueue.h"

typedef int chessboard;
struct ProblemState {
    chessboard ld, cols, rd;
    std::promise<int> promise;

    explicit ProblemState(chessboard ld, chessboard cols, chessboard rd)
            : ld(ld), cols(cols), rd(rd) {}

    ProblemState(const ProblemState & other) = delete;
    ProblemState(ProblemState && other) : ld(other.ld), cols(other.cols), rd(other.rd), promise(std::move(other.promise)) {}

    void operator()(ProblemState && other) {
        ld = other.ld;
        cols = other.cols;
        rd = other.rd;
        promise = std::move(other.promise);
    }

    ~ProblemState() = default;
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
    ThreadSafeQueue<ProblemState> states;
    ThreadSafeQueue<std::shared_future<int>> futures;

    void wait_and_solve();
    void solve(ProblemState &&state);
    int seq_nqueen(chessboard ld, chessboard cols, chessboard rd);
    bool m_shutdown;
    std::condition_variable m_condition;
    std::mutex m_mutex;
    void shutdown();
//    int seq_nqueen(int ld, int col, int rd, int level);
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
