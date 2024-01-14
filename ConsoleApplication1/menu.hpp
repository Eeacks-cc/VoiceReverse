#pragma once

/*

    code here all copied from imgui official example code besides menu

*/

// imgui
#include "3rdParty/imgui/imgui.h"
#include "3rdParty/imgui/imgui_impl_win32.h"
#include "3rdParty/imgui/imgui_impl_dx11.h"
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

#include "Globals.hpp"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
void CALLBACK waveOutProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

namespace menu
{
	ID3D11Device* g_pd3dDevice = NULL;
	ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
	IDXGISwapChain* g_pSwapChain = NULL;
	ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void KeyBind(const char* pName, DWORD* pKey, ImVec2 v2Size = ImVec2(100, 40));

	int Render() // imgui official code  main();
	{
        WNDCLASSEX wc = { sizeof(WNDCLASSEX), 0, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"VoiceReverse", NULL };
        RegisterClassEx(&wc);
        HWND hwnd = CreateWindow(wc.lpszClassName, L"VoiceReverse", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

        if (!CreateDeviceD3D(hwnd))
        {
            CleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        io.FontDefault = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

        io.Fonts->Build();

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // 锁帧叫chatgpt写的 不想重新造轮子了
        double frameTime = 1.0 / config::fMaxFps;
        LARGE_INTEGER frequency;
        LARGE_INTEGER lastFrameTime;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastFrameTime);
        // Main loop
        while (!config::bEnd)
        {
            MSG msg;
            while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    config::bEnd = true;
            }
            if (config::bEnd)
                break;

            LARGE_INTEGER currentFrameTime;
            QueryPerformanceCounter(&currentFrameTime);
            double deltaTime = static_cast<double>(currentFrameTime.QuadPart - lastFrameTime.QuadPart) / frequency.QuadPart;
            lastFrameTime = currentFrameTime;
            frameTime = 1.f / config::fMaxFps;

            // Start the Dear ImGui frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(width, height));


            switch(config::iSetupStage)
            {
                case 0: // setup input/output device
                {
                    ImGui::Begin("VoiceReverse Setup", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

                    ImGui::Text(u8"选择设备:");
                    
                    int firstWindowHeight = 0;
                    if (ImGui::BeginChild("##Child", ImVec2(width / 2 - 30, 400), true))
                    {
                        ImGui::Text(u8"输入设备:");
                        ImGui::Separator();
                        firstWindowHeight = ImGui::GetWindowPos().y;
                        for (int n = 0; n < config::m_vInputDevices.size(); n++)
                        {
                            bool is_selected = (config::iSelectedInputDevice == config::m_vInputDevices[n].first);
                            if (ImGui::Selectable(config::m_vInputDevices[n].second.c_str(), is_selected))
                                config::iSelectedInputDevice = config::m_vInputDevices[n].first;
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndChild();
                    }
                    ImGui::SetNextWindowPos(ImVec2(width / 2, firstWindowHeight));
                    if (ImGui::BeginChild("##Child2", ImVec2(width / 2 - 30, 400), true))
                    {
                        ImGui::Text(u8"输出设备:");
                        ImGui::Separator();
                        for (int n = 0; n < config::m_vOutputDevices.size(); n++)
                        {
                            bool is_selected = (config::iSelectedOutputDevice == config::m_vOutputDevices[n].first);
                            if (ImGui::Selectable(config::m_vOutputDevices[n].second.c_str(), is_selected))
                                config::iSelectedOutputDevice = config::m_vOutputDevices[n].first;
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndChild();
                    }

                    if (ImGui::Button(u8"刷新"))
                    {
                        wwrapper::GetOutputDevices(config::m_vOutputDevices);
                        wwrapper::GetInputDevices(config::m_vInputDevices);
                    }

                    if (ImGui::Button(u8"确认"))
                    {
                        wwrapper::InitializeOut(&waveOutProc, config::iSelectedOutputDevice);
                        SpeedMultiplier::LoadPresetMultiplier(&waveOutProc, config::iSelectedOutputDevice);
                        wwrapper::Initialize(&waveInProc, config::iSelectedInputDevice);
                        config::bSelectingDevice = false;
                        config::iSetupStage = 1;
                        config::AutoLoad();
                    }

                    ImGui::End();
                }
                break;
                case 1:
                {
                    ImGui::Begin("VoiceReverse", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

                    KeyBind(u8"声音反转热键", &config::dwReverseStartKey);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"[声音反转]鼠标单击之后开始记录按键，按下任意键将绑定该按键为热键，绑定时按下ESC来清除绑定");
                    ImGui::NewLine();
                    /*
                    ImGui::SliderFloat(u8"声音加速倍数", &config::fSpeedMultiplier, 0.1f, 10.f);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"越高越快，越低越慢");
                    */

                    ImGui::Text(u8"声音加速(作用于全部效果)");
                    if (ImGui::BeginChild("##ChildSoundMultiplier", ImVec2(width / 2 - 30, 220), true))
                    {
                        ImGui::Text(u8"预设倍率：");
                        ImGui::Separator();
                        for (int i = 0; i < 4; i++)
                        {
                            bool is_selected = (config::iSelectedSpeedMultiplier == i);
                            if (ImGui::Selectable(SpeedMultiplier::m_vMultiplierString[i].first.c_str(), is_selected))
                            {
                                config::fSpeedMultiplier = SpeedMultiplier::m_vMultiplierString[i].second;
                                config::iSelectedSpeedMultiplier = i;
                            }
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::NewLine();
                        KeyBind(u8"声音加速热键", &config::dwAccelerateStartKey);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                            ImGui::SetTooltip(u8"[声音加速]鼠标单击之后开始记录按键，按下任意键将绑定该按键为热键，绑定时按下ESC来清除绑定");
                    }
                    ImGui::EndChild();
                    ImGui::NewLine();

                    ImGui::Text(u8"留声");
                    if (ImGui::BeginChild("##ChildSoundRecord", ImVec2(width / 2 - 30, 400), true))
                    {
                        ImGui::Text(u8"已留存：");
                        ImGui::Separator();
                        if (ImGui::BeginTable(u8"已留存：", 4))
                        {
                            for (int i = 0; i < config::vSavedClips.size(); i++)
                            {
                                ImGui::TableNextColumn();
                                ImGui::Text(config::vSavedClips[i].m_sName.c_str());
                                ImGui::TableNextColumn();
                                KeyBind(0, &config::vSavedClips[i].m_dwHotKey, ImVec2(60, 25));
                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                                    ImGui::SetTooltip(u8"按下热键后以声音加速中选中的倍率播放该留声");

                                ImGui::TableNextColumn();
                                KeyBind(0, &config::vSavedClips[i].m_dwHotKeyReverse, ImVec2(60, 25));
                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                                    ImGui::SetTooltip(u8"按下热键后以声音加速中选中的倍率以倒放模式播放该留声");

                                ImGui::TableNextColumn();
                                if (ImGui::Button(u8"删除"))
                                {
                                    free(config::vSavedClips[i].m_pBuffer);
                                    config::vSavedClips.erase(config::vSavedClips.begin() + i);
                                }
                            }
                        }
                        ImGui::EndTable();
                        ImGui::NewLine();
                        KeyBind(u8"新建留声热键", &config::dwRecordStartKey);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                            ImGui::SetTooltip(u8"[新建留声热键]鼠标单击之后开始记录按键，按下任意键将绑定该按键为热键，绑定时按下ESC来清除绑定");

                    }
                    ImGui::EndChild();
                    ImGui::NewLine();

                    ImGui::Text(u8"炸麦乘数");
                    ImGui::SliderFloat(u8"##炸麦乘数", &config::fTrashMicMultiplier, 0.1f, 5.f);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"炸麦乘数是在处理音频的时候对波形乘以的数值，只要数值不为1.f，就会产生崩坏效果；基于数值的不同，只略微影响声音变化。警告：输出将会变得非常大声，请注意音量");

                    KeyBind(u8"炸麦热键", &config::dwTrashMicStartKey);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"[炸麦]鼠标单击之后开始记录按键，按下任意键将绑定该按键为热键，绑定时按下ESC来清除绑定");

                    ImGui::NewLine();

                    ImGui::Checkbox(u8"回环", &config::bEnableLoopback);
                    if(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"打开回环之后除非正在录制倒放，否则麦克风输入将会被直接传输到输出麦克风设备，您将直接听到自己的声音");

                    ImGui::NewLine();
                    
                    if (ImGui::Checkbox(u8"启用控制台", &config::bShowConsole))
                    {
                        ShowWindow(config::hConsoleHWND, (config::bShowConsole ? SW_SHOW : SW_HIDE));
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"打开/关闭控制台，可以查看日志");

                    ImGui::NewLine();
                    
                    if (ImGui::Button(u8"更改设备"))
                    {
                        config::iSetupStage = 0;
                        wwrapper::End();
                    }

                    ImGui::NewLine();

                    ImGui::Checkbox(u8"自动加载设置", &config::bAutoLoadSettings);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"打开此项后需要保存设置才能够在下次启动时自动加载");

                    ImGui::NewLine();

                    ImGui::Text(u8"锁帧");
                    if (ImGui::SliderFloat("##锁帧", &config::fMaxFps, 1.f, 300.f))
                    {
                        frameTime = 1.f / config::fMaxFps;
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"锁帧能够轻微的减少UI在不必要的高帧率下渲染时带来的性能损失，对性能影响不大");

                    ImGui::NewLine();

                    if (ImGui::Button(u8"加载设置"))
                    {
                        config::Load();
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"加载此页面所有设置");
                    
                    ImGui::SameLine();

                    if (ImGui::Button(u8"保存设置"))
                    {
                        config::Save();
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(u8"保存此页面所有设置，注意：不包括设备选择，考虑到设备可能会新增/减少，会导致设备id偏移");

                    ImGui::End();
                }
                break;
            }


            // Rendering
            ImGui::Render();
            const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            if (deltaTime < frameTime) {
                double sleepTime = frameTime - deltaTime;
                DWORD sleepMilliseconds = static_cast<DWORD>(sleepTime * 1000.0);
                if (sleepMilliseconds > 0) {
                    Sleep(sleepMilliseconds);
                }
            }
            
            g_pSwapChain->Present(1, 0); // Present with vsync
            //g_pSwapChain->Present(0, 0); // Present without vsync
        }

        // Cleanup
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);

        wwrapper::End();
        exit(0);

        return 0;
	}

	void Initialize()
	{
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Render, 0, 0, 0);
	}

    bool CreateDeviceD3D(HWND hWnd)
    {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    }

    void CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void CleanupRenderTarget()
    {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
    }

    DWORD* m_pBindingKey;
    void KeyBind(const char* pName, DWORD* pKey, ImVec2 v2Size)
    {
        if(pName)
            ImGui::Text(pName);
        ImGui::PushID((int)pKey);
        if (ImGui::Button(((m_pBindingKey && m_pBindingKey == pKey) ? "[...]" : sKeyCodes[*pKey]), v2Size))
        {
            m_pBindingKey = pKey;
        }
        ImGui::PopID();
    }

    // Win32 message handler
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (m_pBindingKey)
        {
            switch (msg)
            {
            case WM_LBUTTONDOWN:
                *m_pBindingKey = VK_LBUTTON;
                m_pBindingKey = nullptr;
                break;
            case WM_RBUTTONDOWN:
                *m_pBindingKey = VK_RBUTTON;
                m_pBindingKey = nullptr;
                break;
            case WM_MBUTTONDOWN:
                *m_pBindingKey = VK_MBUTTON;
                m_pBindingKey = nullptr;
                break;
            case WM_KEYDOWN:
                *m_pBindingKey = (wParam != VK_ESCAPE ? wParam : 0);
                m_pBindingKey = nullptr;
                break;
            case WM_XBUTTONDOWN:
            {
                DWORD Button = GET_XBUTTON_WPARAM(wParam);
                switch (Button)
                {
                case VK_LBUTTON:
                    Button = VK_XBUTTON1;
                    break;
                case VK_RBUTTON:
                    Button = VK_XBUTTON2;
                    break;
                }
                *m_pBindingKey = Button;
                m_pBindingKey = nullptr;
            }
            break;
            }
        }
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }
}