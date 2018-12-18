#include "thread_pool.h"

void thread_pool::fetch() {
	while (!to_stop) {
		task t;

		std::unique_lock<std::mutex> lock(mtx);
		cv_task.wait(lock, [this]() {return this->to_stop.load() || !this->tasks.empty(); });

		if (to_stop && tasks.empty())
			return;

		t = std::move(tasks.front());
		tasks.pop();

		idle_thread--;
		t();
		idle_thread++;
	}
}

thread_pool::thread_pool(size_t num)
	:to_stop(false) {

	if (num < 1 || num > 256)
		throw std::runtime_error("thread num should be between 1-256.");

	idle_thread = num;

	for (size_t i = 0; i < num; ++i)
		pool.emplace_back(std::bind(&thread_pool::fetch, this));
}

thread_pool::~thread_pool() {
	to_stop.store(true);
	cv_task.notify_all();

	for (auto &thread : pool)
		if (thread.joinable())
			thread.join();
}
