#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP
#include "common.hpp"
#include <memory>
#include <functional>
#include <queue>
#include <mutex>
#include <variant>

using callback_t = std::function<void()>;
struct Owning_Coroutine {
    std::shared_ptr<common::Ref> reference;
    state_t thread;
    constexpr Owning_Coroutine(): reference(nullptr), thread(nullptr) {}
    explicit Owning_Coroutine(state_t thread): thread(thread) {
        lua_pushthread(thread);
        reference = std::make_shared<common::Ref>(thread, -1);
        lua_pop(thread, 1);
    }
    constexpr void invalidate() {
        reference->release();
        reference.reset();
    }
    constexpr bool is_invalid() const {
        return not reference or not thread;
    }
    constexpr operator bool() const {
        return !is_invalid();
    }
};
struct Task {
    Owning_Coroutine coroutine;
    callback_t callback;
};
struct Task_Scheduler {
    void add_task(callback_t callback) {
        std::lock_guard<std::mutex> lock(task_mutex_);
        task_queue_.push({Owning_Coroutine{}, callback});
    }
    void add_task(state_t coroutine) {
        std::lock_guard<std::mutex> lock(task_mutex_);
        task_queue_.push({Owning_Coroutine{coroutine}, nullptr});
    }
    void add_task(Task&& task) {
        std::lock_guard<std::mutex> lock(task_mutex_);
        task_queue_.push(std::move(task));
    }
    void run_tasks() {
        std::lock_guard<std::mutex> lock(task_mutex_);
        size_t count = task_queue_.size();
        while (count--) {
            Task current = task_queue_.front();
            task_queue_.pop();
            if (current.coroutine) {
                int status = lua_resume(current.coroutine.thread, nullptr, 0);
                if (status == LUA_YIELD) task_queue_.push(current);
                else current.coroutine.invalidate();
            }
            if (current.callback) current.callback();
        }
    }
    auto no_tasks_left() -> bool const {
        return task_queue_.empty();
    }
private:
    std::queue<Task> task_queue_;
    std::mutex task_mutex_;
};

#endif
