#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

class SpinLock {

public:


private:


};




atomic<int32> sum = 0;//공유 데이터, 서로 다른 쓰레드가 쓰기를 할려할시 이미 점유한 쓰레드가 있으면 거부 당함
//쓰레드는 병렬로 실행되므로 무엇이 먼저 끝나는지는 알수 없음 

	//thread t;//쓰레드 생성 및 실질적인 쓰레드 연결 없음 
	//t=thread(HelloThread2,10);//쓰레드 연결
	//t.join();//메인 쓰레드가 종료되기 이전에 부속쓰레드의 작동이 보장될수 있도록 메인쓰레드를 기다리도록 하는  함수
	//t.get_id();//쓰레드마다의 id;
	//t.joinable();//쓰레드 객체가 실질적인 연결이 되어있는 객체인지 판별 ,true 가 나오면 연결이 되어있는거임 
	//if (t.joinable()) {
	//	t.join();//부속 쓰레드의 실질적 발동
	//}

void HelloThread() {//entry point ,쓰레드가 실행할 함수 
	cout << "Hello Thread" << endl;
}

void HelloThread2(int32 num) {//entry point ,쓰레드가 실행할 함수 
	cout << "value의 값 : "<<num<< endl;
}

void Add() {

	for (int32 i = 0; i < 1000; i++) {
		sum++;
	}
}

void Sub() {
	for (int32 i = 0; i < 1000; i++) {
		sum--;
	}
}
int value = 0;
vector<int32> v;
mutex m;//lock 기능, lock 과 unlock 기능을 자주 쓸경우 느려질수 있음 

template<typename T>//커스텀 lockguard
class LockGuard {

public :

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


void Push() {
	
	for (int32 i = 0; i < 1000; i++) {

		lock_guard<mutex> lockguard(m);//자동 자물쇠 
		
		//m.lock();//자물쇠 잠그기
		v.push_back(i);
		//m.unlock();//자물쇠 해체 
	}
}

void Plus(){
	
	for (int32 i = 0; i < 1000; i++) {
		lock_guard<mutex> guard(m);
		value++;
	}

}

void Minus() {

	for (int32 i = 0; i < 1000; i++) {
		lock_guard<mutex> guard(m);
		value--;
	}

}

int main()//메인 쓰레드 
{
	thread t1(Plus);
	thread t2(Minus);
	
	t1.join();
	t2.join();

	cout << value << endl;
}