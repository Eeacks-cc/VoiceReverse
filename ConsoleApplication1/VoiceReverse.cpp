/*
*   VoiceReverse.cpp  
*   ----------------------
*   ExtremeBlackLiu 2023/7/16
* 
*   this used waveIn/Out api which is old api that is deprecated
* 
*/


#include "Includes.hpp"
#include "waveWrapper.hpp"

#include "menu.hpp"

int m_iCount = 0;
int m_iCount1 = 0;

bool bInPlaying = false;
bool bInCapture = false;

DWORD64 dwStartTime;
std::vector<unsigned char> vBuffer;

void swap_byte(std::vector<unsigned char> &vTarget, int a, int b)
{
    const unsigned char temp = vTarget[b];
    vTarget[b] = vTarget[a];
    vTarget[a] = temp;
}

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (uMsg == WIM_DATA)
    {
        if (!bInPlaying)
        {
            if (bInCapture)
            {
                size_t oldSize = vBuffer.size();
                vBuffer.resize(oldSize + wwrapper::pWaveBuffer[m_iCount].dwBufferLength);
                unsigned char* buf = vBuffer.data();
                memcpy(buf + oldSize, wwrapper::pWaveBuffer[m_iCount].lpData, wwrapper::pWaveBuffer[m_iCount].dwBufferLength);
            }
            else
            {
                if (config::bEnableLoopback)
                {
                    if (waveOutWrite(wwrapper::hWaveOut, &wwrapper::pWaveBuffer[m_iCount], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
                        LOG("Excepted while waveOut, bInCapture = true");
                }
            }
        }

        m_iCount++;

        if (m_iCount >= BUFFER_ARRAY_SIZE)
            m_iCount = 0;

        auto Err = waveInAddBuffer(wwrapper::hWave, &wwrapper::pWaveBuffer[m_iCount], sizeof(WAVEHDR));
        if (Err != MMSYSERR_NOERROR)
        {
            LOG("Excepted while waveIn");
            std::cout << "excepted with code: " << Err << std::endl;
            char errtext[MAXERRORLENGTH];
            waveInGetErrorTextA(Err, errtext, MAXERRORLENGTH);
            std::cout << errtext << std::endl;
        }
    }
}

bool bLastPlayDone = false;
void CALLBACK waveOutProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (uMsg == WOM_DONE && bInPlaying)
    {
        bLastPlayDone = true;
        vBuffer.clear();
    }
}

int main()
{
    LOG("Initializing GUI...");

    wwrapper::GetOutputDevices(config::m_vOutputDevices);
    wwrapper::GetInputDevices(config::m_vInputDevices);
    menu::Initialize();

    bInCapture = false;
    while (!config::bEnd)
    {
        while (!config::bSelectingDevice)
        {
            bool Mode0 = GetAsyncKeyState(config::dwStartKey) != 0;
            if (menu::m_pBindingKey)
                Mode0 = false;
            if (Mode0)
            {
                if (!bInCapture)
                {
                    dwStartTime = GetTickCount64();
                    LOG("start recording...");
                    bInCapture = true;
                }
            }
            else
            {
                DWORD64 delta = GetTickCount64() - dwStartTime;
                if (vBuffer.size() > 0)
                {
                    std::cout << "playing...( " << delta << "ms captured)" << std::endl;
                    WAVEHDR* wHeader = nullptr;

                    for (int i = 0; i < vBuffer.size() / 2; i += 2)
                    {
                        int reverse_pos = vBuffer.size() - i - 1;

                        swap_byte(vBuffer, i, reverse_pos - 1);
                        swap_byte(vBuffer, i + 1, reverse_pos);
                    }

                    wHeader = wwrapper::HeaderQueue::Create(&vBuffer, delta + 1000);
                    bInPlaying = true;

                    if (waveOutWrite(wwrapper::hWaveOut, wHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
                        LOG("Excepted while playing backward");

                    while (!bLastPlayDone)
                        Sleep(1);

                    bInPlaying = false;
                    bLastPlayDone = false;
                    bInCapture = false;
                }
            }
        }
    }

    return 0;
}