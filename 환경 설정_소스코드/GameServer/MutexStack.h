#pragma once

#include <mutex>

using namespace std;
template<typename T>
class MutexStack
{

public:
	MutexStack() {}

	void Push(T value) {

		lock_guard<mutex> lock(_mutex);
		_stack.push(move(value));
		_cv.notify_one();
	}

	bool TryPop(T& value) {// 무한 반복문

		lock_guard<mutex> lock(_mutex);
		if (_stack.empty()) {

			return false;
		}

		value = move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value) {

		unique_lock<mutex> lock(_mutex);
		_cv.wait(lock, [this] {return _stack.empty() == false; });//람다문이 true일때까지 기다림 
		value = move(_stack.top());
		_stack.pop();


	}
private:

	stack<T> _stack;
	mutex _mutex;
	condition_variable _cv;
};

