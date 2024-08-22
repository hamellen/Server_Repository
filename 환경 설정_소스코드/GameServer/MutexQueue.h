#pragma once

#include <mutex>

template<typename T>
class MutexQueue
{
public:

	MutexQueue() {}

	void Push(T value) {

		lock_guard<mutex> lock(_mutex);//다른  쓰레드 접근 금지 
		_queue.push(move(value));
		_cv.notify_one();
	}

	bool TryPop(T& value) {// 무한 반복문

		lock_guard<mutex> lock(_mutex);
		if (_queue.empty()) {

			return false;
		}

		value = move(_queue.front());
		_queue.pop();
		return true;
	}

	void WaitPop(T& value) {//pop과 값 꺼내기를 동시에 

		unique_lock<mutex> lock(_mutex);
		_cv.wait(lock, [this] {return _queue.empty() == false; });//람다문이 true일때까지 기다림 
		value = move(_queue.front());
		_queue.pop();


	}

private:

	queue<T> _queue;
	mutex _mutex;
	condition_variable _cv;

};

