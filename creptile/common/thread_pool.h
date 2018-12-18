#pragma once

#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class thread_pool {
	using task = std::function<void()>;

	std::vector<std::thread> pool;
	std::queue<task> tasks;
	std::mutex mtx;
	std::condition_variable cv_task;
	std::atomic<bool> to_stop;
	std::atomic<int> idle_thread;

	void fetch();

public:
	thread_pool(size_t num);
	~thread_pool();

	template<class F, class... Args>
	auto commit(F&& f, Args&&... args)->std::future<decltype(f(args...))> {
		if (to_stop)
			throw std::runtime_error("commit on thread pool which is to stop.");

		using RetType = decltype(f(args...));
		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		std::future<RetType> f = task->get_future();

		std::lock_guard<std::mutex> lock(mtx);
		tasks.emplace([task]() { (*task)(); });
		cv_task.notify_one();
		return f;
	}
};
