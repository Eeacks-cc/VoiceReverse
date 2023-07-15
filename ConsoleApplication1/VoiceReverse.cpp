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

int m_iCount = 0;
int m_iCount1 = 0;

bool bInPlaying = false;
bool bInCapture = false;
bool bPlayWithNormal = false;

DWORD64 dwStartTime;
std::vector<unsigned char> vBuffer;

void swap_byte(std::vector<unsigned char> &vTarget, int a, int b)
{
    if (bPlayWithNormal) return;
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


void Thread()
{
    UINT deviceIndex;
    std::vector<std::pair<int, std::string>> m_vDevices;
    wwrapper::GetOutputDevices(m_vDevices);

    std::cout << "Available speaker devices:" << std::endl;
    for (UINT i = 0; i < m_vDevices.size(); i++)
    {
        std::cout << m_vDevices[i].first << ". ";
        std::cout << m_vDevices[i].second << std::endl;
    }

    LOG("Select Output device:");
    std::cin >> deviceIndex;
    //deviceIndex = 2;

    wwrapper::InitializeOut(&waveOutProc,deviceIndex);
    m_vDevices.clear();
    wwrapper::GetInputDevices(m_vDevices);

    std::cout << "Available microphone devices:" << std::endl;
    for (UINT i = 0; i < m_vDevices.size(); i++)
    {
        std::cout << m_vDevices[i].first << ". ";
        std::cout << m_vDevices[i].second << std::endl;
    }

    LOG("Select input device:");
    std::cin >> deviceIndex;
    //deviceIndex = 0;
    wwrapper::Initialize(&waveInProc, deviceIndex);

    LOG("Enable Loopback? (0 for off, 1 for on)");
    std::cin >> config::bEnableLoopback;

    LOG("waiting for end(press INS to end)...");

    bInCapture = false;
    while (!GetAsyncKeyState(VK_F10))
    {
        bool Mode0 = GetAsyncKeyState(VK_F1) != 0; // backward only
        bool Mode1 = GetAsyncKeyState(VK_F2) != 0; // normal + backward
        
        if (Mode0 || Mode1)
        {
            if (!bInCapture)
            {
                dwStartTime = GetTickCount64();
                bPlayWithNormal = Mode1;
                LOG("start recording...");
                bInCapture = true;
            }
        }
        if (!Mode0 && !Mode1)
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

    wwrapper::End();
}

int main()
{
    LOG("Initializing GUI...");
    //Sleep(1300);
    LOG("just kidding, no gui atm");

    HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Thread, 0, 0, 0);
    WaitForSingleObject(hThread, 0xFFFFFFFF);

    return 0;
}