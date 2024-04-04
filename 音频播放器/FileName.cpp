#include <Windows.h>
#include <CommCtrl.h>
#include <iostream>
#include <string>

#define ID_PLAY_PAUSE_BUTTON 1001
#define ID_TIMER_PROGRESS 1002 // 新增的定时器 ID

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hwndProgressBar;
bool isPlaying = false;
WCHAR filePath[MAX_PATH];

int main() {
    // 创建窗口类
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MusicPlayer";
    wc.style = CS_DROPSHADOW; // 启用拖放功能
    RegisterClass(&wc);

    // 创建窗口
    HWND hwnd = CreateWindow(L"MusicPlayer", L"Music Player", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (hwnd == NULL) {
        return 0;
    }

    // 创建播放/暂停按钮
    CreateWindow(L"BUTTON", L"Play/Pause", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 10, 100, 30, hwnd, (HMENU)ID_PLAY_PAUSE_BUTTON, GetModuleHandle(NULL), NULL);

    // 创建进度条控件
    hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE,
        10, 50, 360, 20, hwnd, NULL, NULL, NULL);
    SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    // 启用窗口接收拖放文件事件
    DragAcceptFiles(hwnd, TRUE);

    // 显示窗口
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    // 创建定时器，每100毫秒触发一次
    SetTimer(hwnd, ID_TIMER_PROGRESS, 100, NULL);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 销毁定时器
    KillTimer(hwnd, ID_TIMER_PROGRESS);

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_PLAY_PAUSE_BUTTON: {
            // 播放/暂停音乐
            if (!isPlaying) {
                // 开始播放
                std::wstring command = L"open \"" + std::wstring(filePath) + L"\" type mpegvideo alias music";
                mciSendStringW(command.c_str(), NULL, 0, NULL);

                command = L"play music";
                mciSendStringW(command.c_str(), NULL, 0, NULL);
                isPlaying = true;
            }
            else {
                // 暂停/继续播放
                std::wstring command = L"pause music";
                mciSendStringW(command.c_str(), NULL, 0, NULL);
                isPlaying = !isPlaying;
            }
            break;
        }
        }
        break;
    }
    case WM_DROPFILES: {
        // 获取拖放的文件信息
        HDROP hDrop = (HDROP)wParam;
        UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
        if (nFiles > 0) {
            // 获取拖放的第一个文件路径
            DragQueryFile(hDrop, 0, filePath, MAX_PATH);

            // 播放音乐文件
            std::wstring command = L"open \"" + std::wstring(filePath) + L"\" type mpegvideo alias music";
            mciSendStringW(command.c_str(), NULL, 0, NULL);

            command = L"play music";
            mciSendStringW(command.c_str(), NULL, 0, NULL);
            isPlaying = true;
        }
        DragFinish(hDrop);
        return 0;
    }
    case WM_TIMER: {
        if (isPlaying) {
            // 更新进度条
            // 获取音乐文件的总长度
            MCI_STATUS_PARMS statusParams;
            statusParams.dwItem = MCI_STATUS_LENGTH;
            mciSendCommand(0, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&statusParams);
            int totalLength = statusParams.dwReturn;

            // 获取当前播放位置
            MCI_STATUS_PARMS positionParams;
            positionParams.dwItem = MCI_STATUS_POSITION;
            mciSendCommand(0, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&positionParams);
            int currentPosition = positionParams.dwReturn;

            // 计算进度百分比并更新进度条
            int progress = 0;
            if (totalLength > 0) {
                progress = (int)(((double)currentPosition / (double)totalLength) * 100.0);
            }
            SendMessage(hwndProgressBar, PBM_SETPOS, progress, 0);
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}



