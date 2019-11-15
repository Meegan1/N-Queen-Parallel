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


    void push(T &new_value) {
        std::lock_guard<std::mutex> lock(gate);
        queue.emplace(new_value);
        cv.notify_one();
    }

    void push(T &&new_value) {
        std::lock_guard<std::mutex> lock(gate);
        queue.emplace(std::move(new_value));
        cv.notify_one();
    }

    bool wait_and_pop(T &val) {
        std::unique_lock<std::mutex> lock(gate);
        cv.wait(lock,
                [this]{ return !queue.empty() || is_complete; });
        if(is_complete)
            return false;
        val(std::move(queue.front()));
        queue.pop();
        return true;
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lock(gate);
        if (queue.empty()) return false;
        val = queue.front();
        queue.pop();
        return true;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(gate);
        return queue.empty();
    }

    void complete() {
        is_complete = true;
        cv.notify_all();
    }

private:
    std::queue<T> queue;
    std::mutex gate;
    std::condition_variable cv;
    bool is_complete;
};


#endif //ASSIGNMENT_1_THREADSAFEQUEUE_H
