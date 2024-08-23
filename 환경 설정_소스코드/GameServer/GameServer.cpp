#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")
class SpinLock {//무한대기

public:

	void lock() {

		bool expected = false;//_locked 값이 되기를 바라는 값 
		bool desired=true;//expected 와는 다른 값
		//_locked 값이 expected 값이 같아지는 순간 _locked에 desired 값 삽입 
		while (_locked.compare_exchange_strong(expected, desired)==false) {//_locked 값이 expected 값과 다르다면 false를 반환하며 무한 뺑뺑이 
			expected = false;
			this_thread::sleep_for(chrono::milliseconds(100));//1초후에 다시 작업하러 옴,컨텍스트 스위칭 발생
		}
	}

	void unlock() {

		_locked.store(false);
	}

private:
	atomic<bool> _locked = false;

};




atomic<int32> sum = 0;//공유 데이터, 서로 다른 쓰레드가 쓰기를 할려할시 이미 점유한 쓰레드가 있으면 거부 당함
//쓰레드는 병렬로 실행되므로 무엇이 먼저 끝나는지는 알수 없음 
//생각보다 많이 느림,꼭 필요할때만 써야됨
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
SpinLock spinlock;
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
		lock_guard<SpinLock> guard(spinlock);
		value++;
	}

}

void Minus() {

	for (int32 i = 0; i < 1000; i++) {
		lock_guard<SpinLock> guard(spinlock);
		value--;
	}

}

queue<int32> q;
HANDLE handle;
condition_variable cv;//커널 아닌 유저 오브젝트 
//스핀락은 무한뺑뺑이,sleep은 몇초간 잠든후에 다시 가능한지 확인이라서 운빨
//이벤트

void Producer()
{
	while (true) {

		{
			unique_lock<mutex> lock(m);
			q.push(100);
			
		}
		//cv.notify_all();//자고 있는 모든 쓰레드 깨우기 
		cv.notify_one();//자고 있는 하나의 쓰레드 깨우기 
		//::SetEvent(handle);//시그널 상태로 바꿈 
		//this_thread::sleep_for(100ms);
	}
}

void Consumer()
{
	while (true) {

		//::WaitForSingleObject(handle, INFINITE);//대기 모드 ,시그널 모드가 되면 대기 모드 해체,자동리셋모드 일시 시그널 모드가 되면 바로 논시그널 모드로 자동전환 
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() {return q.empty() == false; });//lock을 잡고 조건이 맞으면 빠져나와서 코드 진행,
		//조건이 맞지 않는다면 lock을 풀어주고 대기 모드 
		if (q.empty() == false) {
			int32 data = q.front();
			q.pop();
			cout << data << endl;
		}

	}
}

thread_local int32 TLS_storage=0;//쓰레드마다의 고유공간
int main()//메인 쓰레드 
{
	
	//handle=::CreateEvent(NULL, FALSE, FALSE, NULL);//커널 오브젝트/자동리셋 모드,논 시그널 상태 
	//condition_variable 덕분에 쓸일이 없어짐

	/*thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();*/

	//::CloseHandle(handle);

	//동기-메인 쓰레드가 호출된 즉시 함수 내부로 들어가 return 후 다시 밖으로 나와 작업개시 
	//비동기
	//future<int64> future=async(launch::async,함수이름)
	// async- 별도의 쓰레드를 만들어 시행 | deferred - 단일 쓰레드 모드로 get호출시 함수 실행
	//future.get() - 결과물 얻기 
	//future_status - future 함수 객체의 완료 상태 
	//promise,packaged_task 보조 기능

	//atomic < bool > flag = false;
	//flag.is_lock_free() true이면 lock없이 변경 가능 false이면 lock 설정하면서 변경해야하며 atomic에서 자동으로 해줌
	//flag.store(true, memory_order_seq_cst);
	//flag.load(memory_order_seq_cst);

	//bool prev=flag.exchange(true) - 이전 값 추출과 값 수정을 동시에 하는 함수 

	//memory model 
	//Sequentially Consistent(seq_cst)-  컴파일러 최적화 여지 적음-직관적-가시성 및 코드 재배치 문제 해결 
	//Acquire-Release(acquire,release)
	//Relaxed(relaxed)- 컴파일러 최적화 여지 많음,코드 재배치 자주 일어남 ,거의 활용하지 않음

	//atomic 클래스로 만든 변수를 이용해 멀티 쓰레드를 돌릴경우 일반변수의 가시성 확보 
	// 
	// 
	
	//윈도우 소켓 초기화 
	WSAData wsData;
	::WSAStartup(MAKEWORD(2, 2), &wsData);

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);//클라이언트와 최초로 연결될 소켓 /안내원 


	//서버 주소 설정(ip주소 및 포트번호)
	SOCKADDR_IN serverAddr;//서버 주소 ipv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);//서버에서 열어준 포트번호

	::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));//리슨소켓과 포트번호 동기화 

	::listen(listenSocket, 10);//서버에 들어올려는 대기열 한도

	while (true) {
		SOCKADDR_IN clientAddr;//서버에 접속한 클라이언트 주소 
		::memset(&serverAddr, 0, sizeof(serverAddr));
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket=::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);//요청 수락 및 클라이언트의 소켓 와 연결된 소켓 

		//실질적 연결됨 
		char ipAddress[16];
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "Client Connected IP= " << ipAddress << endl;

		while (true) {
			char recvBuffer[100];
			int32 recvlen=::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

			cout << "Recv Data! len: " << recvlen << endl;
			cout << "Recv Data! data: " << recvBuffer << endl;
		}
	}

	::WSACleanup();//윈도우 소켓 종료  
}