#pragma once

#include <iostream>
#include <Windows.h>
#include <Mmsystem.h>
#include <dsound.h>

#include <vector>
#include <deque>
#include <tuple>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dsound.lib")

#include "config.hpp"

#define LOG(...) std::cout << __VA_ARGS__ << std::endl;