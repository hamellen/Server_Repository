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
	//윈도우 소켓 초기화 
	WSAData wsData;
	::WSAStartup(MAKEWORD(2, 2), &wsData);

	SOCKET clientSocket=::socket(AF_INET,SOCK_STREAM,0);

	if (clientSocket == INVALID_SOCKET) {//소켓 생성 실패 
		return 0;
	}
	//연결할 서버 설정(ip주소 및 포트번호)
	SOCKADDR_IN serverAddr;//서버 주소 ipv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);//서버 주소 
	serverAddr.sin_port = ::htons(7777);//서버쪽에서 열어준 포트번호로 설정

	::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));//서버에 연결시도

	//연결 성공
	cout << "Connected to Server" << endl;

	while (true) {

		char buffer[100] = "Hello Hamellen";
		::send(clientSocket, buffer, sizeof(buffer), 0);//데이터 전송

		//cout << "Send Data of len : " << sizeof(buffer) << endl;
		this_thread::sleep_for(2s);
	}
	::closesocket(clientSocket);//소켓 정리 
	::WSACleanup();//윈도우 소켓 종료  

}