#pragma once

#define WIN32_LEAN_AND_MEAN // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")//ServerCore ���� ���̺귯�� ���� �������� 
#else
#pragma comment(lib, "Release\\ServerCore.lib")
#endif

#include "CorePch.h"