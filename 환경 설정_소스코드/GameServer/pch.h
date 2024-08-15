#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")//ServerCore 에서 라이브러리 파일 가져오기 
#else
#pragma comment(lib, "Release\\ServerCore.lib")
#endif

#include "CorePch.h"