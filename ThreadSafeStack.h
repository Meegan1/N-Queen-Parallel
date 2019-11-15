//
// Created by Jake Meegan on 14/11/2019.
//

#ifndef ASSIGNMENT_1_THREADSAFESTACK_H
#define ASSIGNMENT_1_THREADSAFESTACK_H

#include <stack>
#include <mutex>

struct empty_stack: std::exception {
    const char* what() const throw() {
        return "Stack is empty";
    }
};

template <typename T>
class ThreadSafeStack {
public:
    explicit ThreadSafeStack() = default;


    void push(T &new_value) {
        std::lock_guard<std::mutex> lock(gate);
        stack.emplace(new_value);
        cv.notify_one();
    }

    void push(T &&new_value) {
        std::lock_guard<std::mutex> lock(gate);
        stack.emplace(std::move(new_value));
        cv.notify_one();
    }

    bool wait_and_pop(T &val) {
        std::unique_lock<std::mutex> lock(gate);
        cv.wait(lock,
                [this]{ return !stack.empty() || is_complete; });
        if(is_complete)
            return false;
        val = stack.top();
        stack.pop();
        return true;
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(gate);
        cv.wait(lock,
                [this]{ return !stack.empty(); });

        std::shared_ptr<T> const res(std::make_shared<T>(stack.top()));
        stack.pop();
        return res;
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lock(gate);
        if (stack.empty()) return false;
        val = stack.top();
        stack.pop();
        return true;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(gate);
        return stack.empty();
    }

    void complete() {
        is_complete = true;
        cv.notify_all();
    }

private:
    std::stack<T> stack;
    std::mutex gate;
    std::condition_variable cv;
    bool is_complete;
};


#endif //ASSIGNMENT_1_THREADSAFESTACK_H
