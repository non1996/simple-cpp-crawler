#pragma once

#include <vector>	
#include <memory>
#include <functional>

template <typename ... Args>
class invokable {
public:
	virtual void operator () (Args&&... args) = 0;
};

template <typename ... Args>
class slot
	: public invokable<Args...> {
private:
	std::function<void(Args&&...)> func = nullptr;

public:
	slot(const std::function<void(Args&&...)> &_func)
		:func(_func){

	}

	virtual void operator () (Args&&... args) {
		func(std::forward<Args>(args)...);
	}
};

template <typename ... Args>
class signal 
	: public invokable<Args...> {
private:
	std::vector<std::shared_ptr<invokable<Args...>>> slots;

public:
	void connect(const std::function<void(Args&&...)> &func) {
		slots.emplace_back(new slot<Args...>(func));
	}

	void connect(const std::shared_ptr<signal<Args...>> &inv) {
		slots.push_back(inv);
	}

	virtual void operator () (Args&&... args) {
		for (auto &iter : slots)
			(*iter)(std::forward<Args>(args)...);
	}

	void emit(Args&&... args) {
		(*this)(std::forward<Args>(args)...);
	}
};

using namespace std::placeholders;
using std::shared_ptr;
using std::make_shared;