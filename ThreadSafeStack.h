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
        size = stack.size();

    }

    void push(T &&new_value) {
        std::lock_guard<std::mutex> lock(gate);
        stack.emplace(std::move(new_value));
        cv.notify_one();
        size = stack.size();

    }

    void wait_and_pop(T &val) {
        std::unique_lock<std::mutex> lock(gate);
        cv.wait(lock,
                [this]{ return !stack.empty(); });
        val = stack.top();
        stack.pop();
        size = stack.size();

    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(gate);
        cv.wait(lock,
                [this]{ return !stack.empty(); });

        std::shared_ptr<T> const res(std::make_shared<T>(stack.top()));
        stack.pop();
        size = stack.size();

        return res;
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lock(gate);
        if (stack.empty()) return false;
        val = stack.front();
        stack.pop();
        size = stack.size();

        return true;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(gate);
        size = stack.size();
        return stack.empty();
    }

private:
    std::stack<T> stack;
    std::mutex gate;
    std::condition_variable cv;
    int size;
};


#endif //ASSIGNMENT_1_THREADSAFESTACK_H
