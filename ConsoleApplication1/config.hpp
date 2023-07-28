#pragma once

namespace config
{
	int iSetupStage = 0;

	bool bEnableLoopback;

	// ͵����Щ���ݶ������namespace����
	std::vector<std::pair<int, std::string>> m_vInputDevices;
	std::vector<std::pair<int, std::string>> m_vOutputDevices;

	int iSelectedInputDevice;
	int iSelectedOutputDevice;

	volatile bool bEnd = false;
	volatile bool bSelectingDevice = true; // ����,��ʹ�ǹ�����õ�clang�������ڲ���volatile���η��������Ҳ���Ż�ʧ�ܵ��´���δ����Ԥ�ڵ�����

	DWORD dwStartKey = 0;

	HWND hConsoleHWND;
	bool bShowConsole = false;

	bool bAutoLoadSettings = false;
}
