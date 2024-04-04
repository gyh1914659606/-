#include <Windows.h>
#include <CommCtrl.h>
#include <iostream>
#include <string>

#define ID_PLAY_PAUSE_BUTTON 1001
#define ID_TIMER_PROGRESS 1002 // �����Ķ�ʱ�� ID

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hwndProgressBar;
bool isPlaying = false;
WCHAR filePath[MAX_PATH];

int main() {
    // ����������
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MusicPlayer";
    wc.style = CS_DROPSHADOW; // �����ϷŹ���
    RegisterClass(&wc);

    // ��������
    HWND hwnd = CreateWindow(L"MusicPlayer", L"Music Player", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (hwnd == NULL) {
        return 0;
    }

    // ��������/��ͣ��ť
    CreateWindow(L"BUTTON", L"Play/Pause", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 10, 100, 30, hwnd, (HMENU)ID_PLAY_PAUSE_BUTTON, GetModuleHandle(NULL), NULL);

    // �����������ؼ�
    hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE,
        10, 50, 360, 20, hwnd, NULL, NULL, NULL);
    SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    // ���ô��ڽ����Ϸ��ļ��¼�
    DragAcceptFiles(hwnd, TRUE);

    // ��ʾ����
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    // ������ʱ����ÿ100���봥��һ��
    SetTimer(hwnd, ID_TIMER_PROGRESS, 100, NULL);

    // ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ���ٶ�ʱ��
    KillTimer(hwnd, ID_TIMER_PROGRESS);

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_PLAY_PAUSE_BUTTON: {
            // ����/��ͣ����
            if (!isPlaying) {
                // ��ʼ����
                std::wstring command = L"open \"" + std::wstring(filePath) + L"\" type mpegvideo alias music";
                mciSendStringW(command.c_str(), NULL, 0, NULL);

                command = L"play music";
                mciSendStringW(command.c_str(), NULL, 0, NULL);
                isPlaying = true;
            }
            else {
                // ��ͣ/��������
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
        // ��ȡ�Ϸŵ��ļ���Ϣ
        HDROP hDrop = (HDROP)wParam;
        UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
        if (nFiles > 0) {
            // ��ȡ�Ϸŵĵ�һ���ļ�·��
            DragQueryFile(hDrop, 0, filePath, MAX_PATH);

            // ���������ļ�
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
            // ���½�����
            // ��ȡ�����ļ����ܳ���
            MCI_STATUS_PARMS statusParams;
            statusParams.dwItem = MCI_STATUS_LENGTH;
            mciSendCommand(0, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&statusParams);
            int totalLength = statusParams.dwReturn;

            // ��ȡ��ǰ����λ��
            MCI_STATUS_PARMS positionParams;
            positionParams.dwItem = MCI_STATUS_POSITION;
            mciSendCommand(0, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&positionParams);
            int currentPosition = positionParams.dwReturn;

            // ������ȰٷֱȲ����½�����
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



