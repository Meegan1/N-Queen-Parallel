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


    void push(T new_value) {
        std::lock_guard<std::mutex> lock(gate);
        stack.push(new_value);
    }

    T &pop() {
        std::lock_guard<std::mutex> lock(gate);
        if (stack.empty())
            throw empty_stack();

        T &res = stack.top();
        stack.pop();
        return res;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(gate);
        return stack.empty();
    }

private:
    std::stack<T> stack;
    std::mutex gate;
};


#endif //ASSIGNMENT_1_THREADSAFESTACK_H
