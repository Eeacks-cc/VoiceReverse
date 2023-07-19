#pragma once

#include "Globals.hpp"

#define BUFFER_ARRAY_SIZE 10

// wave wrapper
namespace wwrapper
{
	HWAVEIN hWave;
	HWAVEOUT hWaveOut;

	WAVEHDR pWaveBuffer[BUFFER_ARRAY_SIZE];

    void* pBuffer;

    namespace HeaderQueue { void ThreadUnpapare(); };

	bool Initialize(void* pCallback, UINT uDeviceID)
	{
        WAVEFORMATEX waveFormat;
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = 1;
        waveFormat.nSamplesPerSec = 44100;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;

        if (waveInOpen(&hWave, uDeviceID, &waveFormat, (DWORD_PTR)pCallback, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
        {
            return false;
        }

        pBuffer = VirtualAlloc(0x0, 4410 * BUFFER_ARRAY_SIZE, MEM_COMMIT, PAGE_READWRITE);

        for (int i = 0; i < BUFFER_ARRAY_SIZE; i++)
        {
            memset(&pWaveBuffer[i], 0x0, sizeof(WAVEHDR));
            pWaveBuffer[i].lpData = (char*)((uintptr_t)pBuffer + i * 4410);
            pWaveBuffer[i].dwBufferLength = 4410;
            waveInPrepareHeader(hWave, &pWaveBuffer[i], sizeof(WAVEHDR));
        }

        if (waveInAddBuffer(hWave, &pWaveBuffer[0], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            waveInClose(hWave);
            return false;
        }

        if (waveInStart(hWave) != MMSYSERR_NOERROR)
        {
            waveInClose(hWave);
            return false;
        }

        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HeaderQueue::ThreadUnpapare, 0, 0, 0);
        return true;
	}

    void InitializeOut(void* pCallback, UINT uDeviceID)
    {
        WAVEFORMATEX waveFormat;
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = 1;
        waveFormat.nSamplesPerSec = 44100;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;
        waveOutOpen(&hWaveOut, uDeviceID, &waveFormat, (DWORD_PTR)pCallback, 0, CALLBACK_FUNCTION);

    }

    void Make_Header(WAVEHDR* pWHeader, void* pBuffer, size_t sSize)
    {
        pWHeader->lpData = (LPSTR)pBuffer;
        pWHeader->dwBufferLength = sSize;
        waveInPrepareHeader(hWave, pWHeader, sizeof(WAVEHDR));
    }

    void End()
    {
        waveInClose(hWave);
        waveOutClose(hWaveOut);
    }

    void GetInputDevices(std::vector<std::pair<int, std::string>>& pOut)
    {
        pOut.clear();
        UINT iDevicesCount = waveInGetNumDevs();
        if (iDevicesCount == 0)
        {
            pOut.clear();
            return;
        }
        
        for (UINT i = 0; i < iDevicesCount; i++)
        {
            WAVEINCAPSA deviceCaps;
            if (waveInGetDevCapsA(i, &deviceCaps, sizeof(deviceCaps)) == MMSYSERR_NOERROR)
            {
                std::pair<int, std::string> pPair = { i , ConvertAnsiToUtf8(deviceCaps.szPname) };
                pOut.push_back(pPair);
            }
        };
    }

    void GetOutputDevices(std::vector<std::pair<int, std::string>>& pOut)
    {
        pOut.clear();
        UINT iDevicesCount = waveOutGetNumDevs();
        if (iDevicesCount == 0)
        {
            pOut.clear();
            return;
        }

        for (UINT i = 0; i < iDevicesCount; i++)
        {
            WAVEOUTCAPSA deviceCaps;
            if (waveOutGetDevCapsA(i, &deviceCaps, sizeof(deviceCaps)) == MMSYSERR_NOERROR)
            {
                std::pair<int, std::string> pPair = { i , ConvertAnsiToUtf8(deviceCaps.szPname) };
                pOut.push_back(pPair);
            }
        };
    }

    namespace HeaderQueue
    {
        std::deque<std::pair<WAVEHDR*, DWORD64>> m_dQueue; 

        void ThreadUnpapare()
        {
            while (true)
            {
                DWORD64 dwTickCount = GetTickCount64();
                for (int i = 0; i < m_dQueue.size(); i++)
                {
                    auto& queue = m_dQueue[i];
                    if (dwTickCount > queue.second)
                    {
                        if (waveInUnprepareHeader(wwrapper::hWave, queue.first, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
                            LOG("Excepted while unprepare header");
                        delete queue.first;
                        m_dQueue.erase(m_dQueue.begin() + i);
                    }
                }
                Sleep(10);
            }
        }

        WAVEHDR* Create(std::vector<unsigned char>* pBuffer, DWORD dwAliveTime)
        {
            WAVEHDR* wHeader = new WAVEHDR;
            memset(wHeader, 0x0, sizeof(WAVEHDR));

            Make_Header(wHeader, pBuffer->data(), pBuffer->size());

            m_dQueue.push_back({ wHeader, GetTickCount64() + dwAliveTime});

            return wHeader;
        }

    }
}