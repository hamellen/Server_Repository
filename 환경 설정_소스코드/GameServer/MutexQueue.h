#pragma once

#include <mutex>

template<typename T>
class MutexQueue
{
public:

	MutexQueue() {}

	void Push(T value) {

		lock_guard<mutex> lock(_mutex);//�ٸ�  ������ ���� ���� 
		_queue.push(move(value));
		_cv.notify_one();
	}

	bool TryPop(T& value) {// ���� �ݺ���

		lock_guard<mutex> lock(_mutex);
		if (_queue.empty()) {

			return false;
		}

		value = move(_queue.front());
		_queue.pop();
		return true;
	}

	void WaitPop(T& value) {//pop�� �� �����⸦ ���ÿ� 

		unique_lock<mutex> lock(_mutex);
		_cv.wait(lock, [this] {return _queue.empty() == false; });//���ٹ��� true�϶����� ��ٸ� 
		value = move(_queue.front());
		_queue.pop();


	}

private:

	queue<T> _queue;
	mutex _mutex;
	condition_variable _cv;

};

