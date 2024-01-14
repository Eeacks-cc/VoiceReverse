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

bool bInPlaying = false;
bool bInCapture = false;

DWORD64 dwStartTime;
std::vector<unsigned char> vBuffer;

enum eMode {
    VR_NONE,
    VR_Reverse,
    VR_SpeedMultiplier,
    VR_TrashMic,
    VR_Record,
    VR_PlayRecord,
    VR_PlayRecordReverse,
};

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
                        LOG("Excepted while waveOut, bInCapture = false");
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

void process_audio_reverse()
{
    DWORD64 delta = GetTickCount64() - dwStartTime;
    std::cout << "playing...( " << delta << "ms captured)" << std::endl;
    WAVEHDR* wHeader = nullptr;

    // processing audio
    for (int i = 0; i < vBuffer.size() / 2; i += 2)
    {
        int reverse_pos = vBuffer.size() - i - 1;

        swap_byte(vBuffer, i, reverse_pos - 1);
        swap_byte(vBuffer, i + 1, reverse_pos);
    }

    wHeader = wwrapper::HeaderQueue::Create(&vBuffer, delta + 1000);
    bInPlaying = true;

    if (waveOutWrite(SpeedMultiplier::hWaveOut[config::iSelectedSpeedMultiplier], wHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        LOG("Excepted while playing backward");

    while (!bLastPlayDone)
        Sleep(1);

    bInPlaying = false;
    bLastPlayDone = false;
    bInCapture = false;
}

void processing_audio_speed_multiplier()
{
    DWORD64 delta = GetTickCount64() - dwStartTime;
    std::cout << "playing...( " << delta << "ms captured)" << std::endl;
    WAVEHDR* wHeader = nullptr;

    wHeader = wwrapper::HeaderQueue::Create(&vBuffer, delta + 1000);
    bInPlaying = true;

    if (waveOutWrite(SpeedMultiplier::hWaveOut[config::iSelectedSpeedMultiplier], wHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        LOG("Excepted while playing backward");

    while (!bLastPlayDone)
        Sleep(1);

    bInPlaying = false;
    bLastPlayDone = false;
    bInCapture = false;
}

void processing_audio_trashmic()
{
    DWORD64 delta = GetTickCount64() - dwStartTime;
    std::cout << "playing...( " << delta << "ms captured)" << std::endl;
    WAVEHDR* wHeader = nullptr;

    // convert to float wave
    std::vector<float> fltBuffer;
    fltBuffer.resize(vBuffer.size());
    for (int i = vBuffer.size() - 1; i > 0; i--)
    {
        fltBuffer[i] = (float)(vBuffer[i] / 128.f - 1.f) * config::fTrashMicMultiplier;
    }
    // convert back
    vBuffer.resize(fltBuffer.size());
    for (int i = fltBuffer.size() - 1; i > 0; i--)
    {
        double _temp = fltBuffer[i] * 2147483648.f;
        if (_temp > 2147483648.f)
        {
            vBuffer[i] = (unsigned char)255;
            continue;
        }
        vBuffer[i] = (unsigned char)((lrint(_temp) >> 24) + 0x80);
    }
    
    wHeader = wwrapper::HeaderQueue::Create(&vBuffer, delta + 1000);
    bInPlaying = true;

    if (waveOutWrite(SpeedMultiplier::hWaveOut[config::iSelectedSpeedMultiplier], wHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        LOG("Excepted while playing accelerate");

    while (!bLastPlayDone)
        Sleep(1);

    bInPlaying = false;
    bLastPlayDone = false;
    bInCapture = false;
}

void processing_record()
{
    DWORD64 delta = GetTickCount64() - dwStartTime;
    std::cout << "playing...( " << delta << "ms captured)" << std::endl;
    WAVEHDR* wHeader = nullptr;

    bInPlaying = true;

    void* clip = malloc(vBuffer.size());
    memcpy(clip, vBuffer.data(), vBuffer.size());
    
    char buf[25];
    sprintf(buf, u8"留声 %d", time(NULL));
    
    config::vSavedClips.push_back({ buf, clip, vBuffer.size(), 0, 0});

    bInPlaying = false;
    bLastPlayDone = false;
    bInCapture = false;
}

void processing_play_record()
{
    DWORD64 delta = GetTickCount64() - dwStartTime;
    std::cout << "playing...( " << delta << "ms captured)" << std::endl;
    WAVEHDR* wHeader = nullptr;

    wHeader = wwrapper::HeaderQueue::Create(config::vSavedClips[config::iRecordIndexToPlay].m_pBuffer, config::vSavedClips[config::iRecordIndexToPlay].m_szBuffer, delta + 1000);
    bInPlaying = true;

    if (waveOutWrite(SpeedMultiplier::hWaveOut[config::iSelectedSpeedMultiplier], wHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        LOG("Excepted while playing backward");

    while (!bLastPlayDone)
        Sleep(1);

    bInPlaying = false;
    bLastPlayDone = false;
    bInCapture = false;
}

void processing_play_record_reverse()
{
    DWORD64 delta = GetTickCount64() - dwStartTime;
    std::cout << "playing...( " << delta << "ms captured)" << std::endl;
    WAVEHDR* wHeader = nullptr;

    auto& SavedClip = config::vSavedClips[config::iRecordIndexToPlay];

    vBuffer.resize(SavedClip.m_szBuffer);
    memcpy(vBuffer.data(), SavedClip.m_pBuffer, SavedClip.m_szBuffer);
    for (int i = 0; i < vBuffer.size() / 2; i += 2)
    {
        int reverse_pos = vBuffer.size() - i - 1;

        swap_byte(vBuffer, i, reverse_pos - 1);
        swap_byte(vBuffer, i + 1, reverse_pos);
    }

    wHeader = wwrapper::HeaderQueue::Create(&vBuffer, delta + 1000);
    bInPlaying = true;

    if (waveOutWrite(SpeedMultiplier::hWaveOut[config::iSelectedSpeedMultiplier], wHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        LOG("Excepted while playing backward");

    while (!bLastPlayDone)
        Sleep(1);

    bInPlaying = false;
    bLastPlayDone = false;
    bInCapture = false;
}


eMode key_trigger()
{
    if (GetAsyncKeyState(config::dwReverseStartKey) != 0)
        return VR_Reverse;
    if (GetAsyncKeyState(config::dwAccelerateStartKey) != 0)
        return VR_SpeedMultiplier;
    if (GetAsyncKeyState(config::dwTrashMicStartKey) != 0)
        return VR_TrashMic;
    if (GetAsyncKeyState(config::dwRecordStartKey) != 0)
        return VR_Record;
    for (int i = 0; i < config::vSavedClips.size(); i++)
    {
        if (GetAsyncKeyState(config::vSavedClips[i].m_dwHotKey) != 0)
        {
            config::iRecordIndexToPlay = i;
            return VR_PlayRecord;
        }
    }
    for (int i = 0; i < config::vSavedClips.size(); i++)
    {
        if (GetAsyncKeyState(config::vSavedClips[i].m_dwHotKeyReverse) != 0)
        {
            config::iRecordIndexToPlay = i;
            return VR_PlayRecordReverse;
        }
    }
    return VR_NONE;
}

eMode iLastMode;

int main()
{
    srand(time(NULL));

    config::hConsoleHWND = GetConsoleWindow();
    LOG("Initializing GUI...");
    ShowWindow(config::hConsoleHWND, SW_HIDE);

    wwrapper::GetOutputDevices(config::m_vOutputDevices);
    wwrapper::GetInputDevices(config::m_vInputDevices);
    menu::Initialize();

    bInCapture = false;
    while (!config::bEnd)
    {
        while (!config::bSelectingDevice)
        {
            if (menu::m_pBindingKey) // do not trigger when key is binding
            {
                Sleep(1000);
                continue;
            }
            eMode iMode = key_trigger();
            if (iMode > VR_NONE)
            {
                if (!bInCapture)
                {
                    dwStartTime = GetTickCount64();
                    LOG("start recording...");
                    bInCapture = true;
                    iLastMode = iMode;
                }
                continue; // dont process when recording
            }
            if (vBuffer.size() > 0)
            {
                switch (iLastMode) // it just for read-friendly, use function array is better...
                {
                case VR_Reverse:
                {
                    process_audio_reverse();
                }
                break;
                case VR_SpeedMultiplier:
                {
                    processing_audio_speed_multiplier();
                }
                break;
                case VR_TrashMic:
                {
                    processing_audio_trashmic();
                }
                break;
                case VR_Record:
                {
                    processing_record();
                }
                break;
                case VR_PlayRecord:
                {
                    processing_play_record();
                }
                break;
                case VR_PlayRecordReverse:
                {
                    processing_play_record_reverse();
                }
                break;
                }
                iLastMode = VR_NONE;
            }
            
            Sleep(1);
        }
        Sleep(1);
    }

    return 0;
}