//
// Created by Jake Meegan on 14/11/2019.
//

#ifndef ASSIGNMENT_1_THREADSAFEQUEUE_H
#define ASSIGNMENT_1_THREADSAFEQUEUE_H

#include <stack>
#include <mutex>
#include <queue>

struct empty_stack: std::exception {
    const char* what() const throw() {
        return "Stack is empty";
    }
};

template <typename T>
class ThreadSafeQueue {
public:
    explicit ThreadSafeQueue() = default;


    void push(T &&new_value) {
        std::lock_guard<std::mutex> lock(gate);
        queue.push(std::move(new_value));
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lock(gate);
        if (queue.empty()) return false;
        val(std::move(queue.front()));
        queue.pop();
        return true;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(gate);
        return queue.empty();
    }

private:
    std::queue<T> queue;
    std::mutex gate;
};


#endif //ASSIGNMENT_1_THREADSAFEQUEUE_H
