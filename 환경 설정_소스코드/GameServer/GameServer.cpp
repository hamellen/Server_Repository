﻿#include "pch.h"
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

const int32 BufferSize = 1000;
struct Session {

	SOCKET socket=INVALID_SOCKET;
	char recvBuffer[BufferSize] = {};
	int32 recvBytes = 0;
	int32 sendBytes = 0;
};

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
	
	//소켓 옵션설정
	//level-옵션을 해석하고 처리할 주체(소켓 코드-SOL_SOCKET,IPV4-IPPROTO_IP,TCP_프로토콜-IPPROTO_TCP)
	//optname(SO_KEEPALIVE-TCP 전용으로 주기적으로 연결상태 확인 여부 체크,SO_LINGER-closesocket을 할시 송신버퍼에 있는 데이터를 보낼 여유 설정  )
	//(SO_SNDBUF-송신버퍼크기,SO_RCVBUF-수신버퍼크기)
	//::shutdown()-closesocket()같이 완전히 끊지 않고 데이터를 보내지 않는다는 의사를 표시할때 사용
	//TCP_NODELAY-데이터가 충분히 크면 보내고,그렇지않으면 데이터가 충분히 쌓일때까지 대기 

	//블로킹 소켓(accept-접속한 클라가 있을때,connect-서버 접속 성공했을때,send-요청한 데이터를 송신버퍼에 복사,recv-수신버퍼에 도착한 데이터를 유저레벨에 복사)



	//윈도우 소켓 초기화 //tcp 블로킹버전
	//WSAData wsData;
	//::WSAStartup(MAKEWORD(2, 2), &wsData);

	//SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);//클라이언트와 최초로 연결될 소켓 /안내원 



	////서버 주소 설정(ip주소 및 포트번호)
	//SOCKADDR_IN serverAddr;//서버 주소 ipv4
	//::memset(&serverAddr, 0, sizeof(serverAddr));
	//serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	//serverAddr.sin_port = ::htons(7777);//서버에서 열어준 포트번호

	//::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));//리슨소켓과 포트번호 동기화 

	//::listen(listenSocket, 10);//서버에 들어올려는 대기열 한도

	//while (true) {
	//	SOCKADDR_IN clientAddr;//서버에 접속한 클라이언트 주소 
	//	::memset(&serverAddr, 0, sizeof(serverAddr));
	//	int addrLen = sizeof(clientAddr);
	//	SOCKET clientSocket=::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);//요청 수락 및 클라이언트의 소켓 와 연결된 소켓 

	//	//실질적 연결됨 
	//	char ipAddress[16];
	//	::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
	//	cout << "Client Connected IP= " << ipAddress << endl;

	//	while (true) {
	//		char recvBuffer[100];
	//		int32 recvlen=::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

	//		cout << "Recv Data! len: " << recvlen << endl;
	//		cout << "Recv Data! data: " << recvBuffer << endl;
	//	}
	//}

	//::WSACleanup();//윈도우 소켓 종료  

	
	WSAData wsData;
	::WSAStartup(MAKEWORD(2, 2), &wsData);

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);

	u_long on = 1;
	::ioctlsocket(listenSocket, FIONBIO, &on);//논블로킹 모드 작동

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	::listen(listenSocket, SOMAXCONN);


	//TCP 논블로킹 버전
	//SOCKADDR_IN clientAddr;//서버에 접속한 클라이언트 주소 
	//int32 addrLen = sizeof(clientAddr);
	//
	//while (true) { 무한반복
	//	SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

	//	if (clientSocket == INVALID_SOCKET) {

	//		if (::WSAGetLastError() == WSAEWOULDBLOCK) {//문제 상황아님 
	//			continue;
	//		}
	//		break;
	//	}
	//	cout << "client connected" << endl;

	//	while (true) {무한반복
	//	
	//		char recvBuffer[1000];
	//		int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
	//		if (recvLen == SOCKET_ERROR) {
	//			
	//			if (::WSAGetLastError() == WSAEWOULDBLOCK) {

	//				continue;
	//			}
	//			break; 
	//		}
	//		else if (recvLen == 0) {//연결끊김

	//			break;
	//		}
	//		cout << "Recv Data Len= " << recvLen << endl;
	//		cout << "Recv Data= "<< recvBuffer<<endl;
	//	}

	//}

	//-------select 모델---------------------
	// 효율이 좋지는 않음,동기식
	//write 나 read 를 하기전에 자동으로 가능한때를 알려주는  select 함수 
	//FD_ZERO-fd_set 비우기 ,FD_SET-fd_set에 소켓 넣기 ,FD_CLR-소켓 제거 

	vector<WSAEVENT> wsaEvents;
	vector<Session> sessions;
	sessions.reserve(100);

	WSAEVENT listenEvent = ::WSACreateEvent();
	wsaEvents.push_back(listenEvent);
	sessions.push_back(Session{listenSocket});
	if (WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)//관찰시작
		return 0;

	
	while (true) {
	
		
		int32 index=::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);//하나라도 준비되면 활성화 
		if (index == WSA_WAIT_FAILED) {
			continue;
		}

		index -= WSA_WAIT_EVENT_0;


		WSANETWORKEVENTS networkEvents;
		if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)//준비된 이벤트 추출
			continue;

		if (networkEvents.lNetworkEvents & FD_ACCEPT) {

			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				continue;

			SOCKADDR_IN clientAddr;
			int32 addrLen = sizeof(clientAddr);

			SOCKET clientSocket=::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET) {//연결 확정
				cout << "client connected" << endl;

				WSAEVENT clientEvent = ::WSACreateEvent();
				wsaEvents.push_back(clientEvent);
				sessions.push_back(Session{ clientSocket });
				if (WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)//관찰시작
					return 0;
			}
		}
		//client session 체크 
		if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE) {

			if ((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT]!=0))
				continue;

			if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] != 0))
				continue;

			Session& s = sessions[index];

			if (s.recvBytes == 0) {

				int32 recvLen = ::recv(s.socket, s.recvBuffer,BUFSIZ, 0);
				if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK) {
					continue;
				}

				s.recvBytes = recvLen;
				cout << "Recv Data Len= " << recvLen << endl;
			}

			if (s.recvBytes > s.sendBytes) {

				int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
				if (sendLen == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
					continue;
				}

				s.sendBytes += sendLen;

				if (s.recvBytes == s.sendBytes) {
					s.recvBytes = 0;
					s.sendBytes = 0;
				}
				cout << "Send Data Len=" << sendLen << endl;
			}
		}

		//FD_CLOSE
		
		if (networkEvents.lNetworkEvents & FD_CLOSE) {
		
			
		}

	}
	//WSAEventSelect 모델,비동기식
	//생성-WSACreateEvent 삭제-WSACloseEvent 신호상태 감지-WSAWaitForMultipleEvents
	//구체적인 네트워크 이벤트-WSAEnumNetworkEvents
	::WSACleanup();
}