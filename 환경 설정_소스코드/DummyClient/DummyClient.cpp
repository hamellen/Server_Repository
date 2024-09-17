#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
int main()
{
	//윈도우 소켓 초기화 ,블로킹 버전
	//WSAData wsData;
	//::WSAStartup(MAKEWORD(2, 2), &wsData);

	//SOCKET clientSocket=::socket(AF_INET,SOCK_STREAM,0);

	//if (clientSocket == INVALID_SOCKET) {//소켓 생성 실패 
	//	return 0;
	//}
	////연결할 서버 설정(ip주소 및 포트번호)
	//SOCKADDR_IN serverAddr;//서버 주소 ipv4
	//::memset(&serverAddr, 0, sizeof(serverAddr));
	//serverAddr.sin_family = AF_INET;
	////serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
	//::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);//서버 주소 
	//serverAddr.sin_port = ::htons(7777);//서버쪽에서 열어준 포트번호로 설정

	//::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));//서버에 연결시도

	////연결 성공
	//cout << "Connected to Server" << endl;

	//while (true) {

	//	char buffer[100] = "Hello Hamellen";
	//	::send(clientSocket, buffer, sizeof(buffer), 0);//데이터 전송

	//	//cout << "Send Data of len : " << sizeof(buffer) << endl;
	//	this_thread::sleep_for(2s);
	//}
	//::closesocket(clientSocket);//소켓 정리 
	//::WSACleanup();//윈도우 소켓 종료  

	//논블로킹
	WSAData wsData;
	::WSAStartup(MAKEWORD(2, 2), &wsData);

	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);

	u_long on = 1;
	::ioctlsocket(clientSocket, FIONBIO, &on);//논블로킹 모드 작동

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777);

	while (true) {

		if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {

			if (::WSAGetLastError() == WSAEWOULDBLOCK) {//문제 상황아님 
				continue;
			}

			if (::WSAGetLastError() == WSAEISCONN) {// 이미 연결된 상태 
				break;
			}
			break;//진짜 문제
		}

	}
	cout << "Connected to Server" << endl;

	char sendBuffer[100]="Hello Hamellen";

	while (true) {

		if (::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0) == SOCKET_ERROR) {

			if (::WSAGetLastError() == WSAEWOULDBLOCK) {
				continue;
			}

			break;//진짜 에러 
		}

		cout << "Send Data Len= " << sizeof(sendBuffer) << endl;
		
		while (true) {

			char recvBuffer[1000];
			int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

			if (recvLen == SOCKET_ERROR) {

				if (::WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				break;
			}
			else if (recvLen == 0) {

				break;
			}

			cout << "Recv Data Len= " << recvLen << endl;
			break;

		}


		this_thread::sleep_for(1s);
	}

	


	::closesocket(clientSocket);//소켓 정리 
	::WSACleanup();//윈도우 소켓 종료  
}