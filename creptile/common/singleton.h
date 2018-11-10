#pragma once

template <typename T>
class singleton {
private:
	static T *ginstance;

public:
	singleton() {
		if (!ginstance)
			ginstance = new T;
	}

	~singleton() {
		delete ginstance;
		ginstance = nullptr;
	}

	static T *instance() {
		return ginstance;
	}
};

template<typename T>
T *singleton<T>::ginstance = nullptr;