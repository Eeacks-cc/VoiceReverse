#pragma once

namespace config
{
	int iSetupStage = 0;

	bool bEnableLoopback;

	std::vector<std::pair<int, std::string>> m_vInputDevices;
	std::vector<std::pair<int, std::string>> m_vOutputDevices;

	int iSelectedInputDevice;
	int iSelectedOutputDevice;

	volatile bool bEnd = false;
	volatile bool bSelectingDevice = true;

	DWORD dwReverseStartKey = 0;
	DWORD dwAccelerateStartKey = 0;
	DWORD dwTrashMicStartKey = 0;
	
	struct SavedClip
	{
		std::string m_sName;
		void* m_pBuffer;
		size_t m_szBuffer;
		
		DWORD m_dwHotKey;
		DWORD m_dwHotKeyReverse;

		SavedClip(std::string sName, void* pBuffer, size_t szBuffer, DWORD dwHotKey, DWORD dwHotKeyReverse) : 
			m_sName(sName), m_pBuffer(pBuffer), m_szBuffer(szBuffer), m_dwHotKey(dwHotKey), m_dwHotKeyReverse(dwHotKeyReverse) {}
	};
	std::vector<SavedClip> vSavedClips;
	
	int iSelectedSavedClips;
	int iRecordIndexToPlay = 0;

	DWORD dwRecordStartKey = 0;

	int iSelectedSpeedMultiplier = 1;
	float fSpeedMultiplier = 1.0f;
	float fTrashMicMultiplier = 1.1f;

	HWND hConsoleHWND;
	bool bShowConsole = false;

	float fMaxFps = 30.f;

	bool bAutoLoadSettings = false;
}
