#pragma once

namespace config
{
	int iSetupStage = 0;

	bool bEnableLoopback;

	// 偷懒这些数据都丢这个namespace里了
	std::vector<std::pair<int, std::string>> m_vInputDevices;
	std::vector<std::pair<int, std::string>> m_vOutputDevices;

	int iSelectedInputDevice;
	int iSelectedOutputDevice;

	volatile bool bEnd = false;
	volatile bool bSelectingDevice = true; // 哈哈,即使是公认最好的clang编译器在不加volatile修饰符的情况下也会优化失败导致代码未按照预期的运行

	DWORD dwStartKey = 0;

	HWND hConsoleHWND;
	bool bShowConsole = false;

	bool bAutoLoadSettings = false;
}
