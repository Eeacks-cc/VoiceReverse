#pragma once

#include <iostream>
#include <Windows.h>
#include <Mmsystem.h>
#include <dsound.h>

#include <vector>
#include <deque>
#include <tuple>
#include <fstream>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dsound.lib")

#include "config.hpp"
#include "3rdParty/json.hpp"

#define LOG(...) std::cout << __VA_ARGS__ << std::endl;

namespace config
{
	nlohmann::json json;
	void Save()
	{
		json["bEnableLoopback"] = bEnableLoopback;
		json["dwStartKey"] = dwStartKey;
		json["bShowConsole"] = bShowConsole;
		json["bAutoLoadSettings"] = bAutoLoadSettings;
		json["fMaxFps"] = fMaxFps;

		std::ofstream f("settings.json");
		if (!f.is_open())
		{
			LOG("Save() failed due to settings.json is not opened");
			return;
		}
		f << json.dump();
		f.close();
	}
	void Load()
	{
		std::ifstream f("settings.json");
		if (!f.is_open())
		{
			LOG("Load() failed due to settings.json is not opened");
			return;
		}
		json = nlohmann::json::parse(f);

		bEnableLoopback = json["bEnableLoopback"].get<bool>();
		dwStartKey = json["dwStartKey"].get<DWORD>();
		bShowConsole = json["bShowConsole"].get<bool>();
		fMaxFps = json["fMaxFps"].get<float>();
		ShowWindow(hConsoleHWND, (bShowConsole ? SW_SHOW : SW_HIDE));

		bAutoLoadSettings = json["bAutoLoadSettings"].get<bool>();
	}
	void AutoLoad()
	{
		std::ifstream f("settings.json");
		if (!f.is_open())
		{
			LOG("AutoLoad() failed due to settings.json is not opened");
			return;
		}
		json = nlohmann::json::parse(f);

		if (json["bAutoLoadSettings"].get<bool>())
		{
			Load();
		}
	}
}

