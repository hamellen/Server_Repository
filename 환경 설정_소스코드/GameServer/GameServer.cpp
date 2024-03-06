#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>//표준 스레드 헤더
#include <atomic>
#include <mutex>
//void Testfuc() {
//	cout << "hi sub thread" << endl;
//}

//void Testfuc_2(int32 num) {//인자 있는 함수 
//	cout << num << endl;
//}

//atomic<int32> sum = 0;//원자성 공유메모리 


//void Add() {
//	for (int32 i = 0; i < 1000000; i++) {
//		sum.fetch_add(1);
//		//sum++;
//	}
//}

//void Sub() {
//	for (int32 i = 0; i < 1000000; i++) {
//		sum.fetch_sub(1);
//		//sum--;
//	}
//}

vector<int32> v;
mutex m;//자물쇠  기능

template<typename T>
class LockGuard {

public:
	LockGuard(T& m) {
		_mutex = &m;
		_mutex->lock();
	}
	~LockGuard() {
		_mutex->unlock();
	}


private:

	T* _mutex;

};

//deadlock- lock 과  unlock 을 쌍으로 맞춰놓지 않았을경우 
void Push() {
	lock_guard<mutex> lockguard(m);
	for (int32 i = 0; i < 10000; i++) {
		v.push_back(i);
	}
}

int main()//메인스레드 ,메인스레드와 부가 스레드는 병렬로 작동
{
	
	thread t1(Push);
	thread t2(Push);
	t1.join();
	t2.join();
	cout << v.size() << endl;

	/*std::thread t1(Add);
	std::thread t2(Sub);
	t1.join();
	t2.join();
	cout << sum << endl;*///원자성 메모리 

	//vector<std::thread> threads;

	/*for (int32 i = 0; i < 10; i++) {
		threads.push_back(std::thread(Testfuc_2, i));
	}*/

	/*for (int32 i = 0; i < 10; i++) {
		if (threads[i].joinable()) {
			threads[i].join();
		}
	}*/

	//thread t(Testfuc_2,10);//부가스레드에 함수 부착
	
	cout << "hi main thread" << endl;

	//int32 count=t.hardware_concurrency();//cpu 코어 개수에 대한 힌트 
	//auto id=t.get_id();//쓰레드 마다의 id
	//t.detach();//join 과는 반대의 작용으로 독립적으로 스레드를 운용,쓸일 없음 
	//t.joinable();//스레드에 부착된 함수가  있는지 판별 ,함수가 부착되는 즉시 id 활성화 
	//t.join();//메인스레드가 부가 스레드의 활성이  끝날때까지 기다려줌 
	//부가 스레드는 병렬적으로 수행되기 때문에 부가 스레드의 출력결과가 일정한 순서가 없음
	
}