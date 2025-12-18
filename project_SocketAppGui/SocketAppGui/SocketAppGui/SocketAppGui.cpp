#include "framework.h"
#include "SocketAppGui.h"
#include "ChatClient.h"

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

constexpr int IDC_EDIT_IP = 103;
constexpr int IDC_EDIT_PORT = 104;
constexpr int IDC_BTN_START = 105;
constexpr int IDC_BTN_STOP = 106;
constexpr int IDC_EDIT_CHAT = 107;
constexpr int IDC_EDIT_MSG = 108;
constexpr int IDC_BTN_SEND = 109;

const UINT WM_NEW_MSG = WM_APP + 1;

HWND hUser;
HWND hEditChat, hEditMsg, hIP, hPort;
HWND g_hWnd = nullptr;

ChatClient g_client;

void AppendChatText(const std::wstring& txt) {
    int len = GetWindowTextLengthW(hEditChat);
    SendMessageW(hEditChat, EM_SETSEL, len, len);
    SendMessageW(hEditChat, EM_REPLACESEL, FALSE, (LPARAM)txt.c_str());
}

std::string WideToUtf8(const wchar_t* wstr) {
    int n = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string s(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &s[0], n, nullptr, nullptr);
    return s;
}

void OnStartClient(HWND hwnd) {
    char ip[64], portBuf[16];
    GetWindowTextA(hIP, ip, sizeof(ip));
    GetWindowTextA(hPort, portBuf, sizeof(portBuf));
    int port = atoi(portBuf);
    if (port <= 0) port = 5000;

    g_client.setOnMessage([](const std::string& msg) {
        PostMessageA(g_hWnd, WM_NEW_MSG, (WPARAM)new std::string(msg), 0);
        });

    g_client.setOnDisconnect([]() {
        PostMessageA(g_hWnd, WM_NEW_MSG, 0, 0);
        });

    if (!g_client.connectTo(ip, port)) {
        MessageBoxA(hwnd, "Connection failed", "Error", MB_OK);
        return;
    }

    AppendChatText(L"Connected to server.\r\n");

    wchar_t wname[256];
    GetWindowTextW(hUser, wname, _countof(wname));
    if (wcslen(wname) == 0)
        wcscpy_s(wname, L"WinClient");

    std::string uname = WideToUtf8(wname);

    std::string joinJson =
        "{\"type\":\"join\",\"user\":\"" + uname + "\"}";

    g_client.sendJson(joinJson);
}

void OnStop(HWND) {
    g_client.disconnect();
    SetWindowTextW(hEditChat, L"");
    AppendChatText(L"Disconnected.\r\n");
}

void OnSend(HWND) {
    wchar_t wmsg[2048];
    GetWindowTextW(hEditMsg, wmsg, _countof(wmsg));
    if (wcslen(wmsg) == 0) return;

    wchar_t wname[256];
    GetWindowTextW(hUser, wname, _countof(wname));
    if (wcslen(wname) == 0)
        wcscpy_s(wname, L"WinClient");

    std::string text = WideToUtf8(wmsg);
    std::string uname = WideToUtf8(wname);

    std::string json =
        "{\"type\":\"chat\",\"user\":\"" + uname +
        "\",\"text\":\"" + text + "\"}";

    g_client.sendJson(json);

    SetWindowTextW(hEditMsg, L"");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_CREATE:
        g_hWnd = hwnd;

        CreateWindowW(L"STATIC", L"Socket Chat",
            WS_CHILD | WS_VISIBLE,
            8, 8, 600, 24,
            hwnd, nullptr, nullptr, nullptr);

        CreateWindowW(L"STATIC", L"Name:",
            WS_CHILD | WS_VISIBLE,
            8, 40, 40, 22,
            hwnd, nullptr, nullptr, nullptr);

        hUser = CreateWindowW(L"EDIT", L"MyName",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            55, 40, 120, 22,
            hwnd, nullptr, nullptr, nullptr);

        hIP = CreateWindowW(L"EDIT", L"127.0.0.1",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            180, 40, 160, 22,
            hwnd, nullptr, nullptr, nullptr);

        hPort = CreateWindowW(L"EDIT", L"5000",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            350, 40, 80, 22,
            hwnd, nullptr, nullptr, nullptr);

        CreateWindowW(L"BUTTON", L"Start",
            WS_CHILD | WS_VISIBLE,
            440, 40, 80, 22,
            hwnd, (HMENU)IDC_BTN_START, nullptr, nullptr);

        CreateWindowW(L"BUTTON", L"Stop",
            WS_CHILD | WS_VISIBLE,
            525, 40, 80, 22,
            hwnd, (HMENU)IDC_BTN_STOP, nullptr, nullptr);

        hEditChat = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE |
            ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            8, 70, 720, 360,
            hwnd, nullptr, nullptr, nullptr);

        hEditMsg = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE,
            8, 440, 620, 28,
            hwnd, nullptr, nullptr, nullptr);

        CreateWindowW(L"BUTTON", L"Send",
            WS_CHILD | WS_VISIBLE,
            640, 440, 88, 28,
            hwnd, (HMENU)IDC_BTN_SEND, nullptr, nullptr);
        break;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_START: OnStartClient(hwnd); break;
        case IDC_BTN_STOP:  OnStop(hwnd); break;
        case IDC_BTN_SEND:  OnSend(hwnd); break;
        }
        break;

    case WM_NEW_MSG: {
        std::string* p = (std::string*)wp;
        if (!p) {
            AppendChatText(L"[Disconnected]\r\n");
            break;
        }

        std::string raw = *p;
        delete p;

        size_t uPos = raw.find("\"user\":\"");
        size_t tPos = raw.find("\"text\":\"");

        if (uPos != std::string::npos && tPos != std::string::npos) {
            size_t uStart = uPos + 8;
            size_t uEnd = raw.find("\"", uStart);
            size_t tStart = tPos + 8;
            size_t tEnd = raw.find("\"", tStart);

            std::string user = raw.substr(uStart, uEnd - uStart);
            std::string text = raw.substr(tStart, tEnd - tStart);
            std::string line = user + ": " + text;

            int wlen = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, nullptr, 0);
            std::wstring wmsg(wlen - 1, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, &wmsg[0], wlen);
            AppendChatText(wmsg + L"\r\n");
        }
        break;
    }

    case WM_CLOSE:
        OnStop(hwnd);
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SocketChatWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(
        wc.lpszClassName,
        L"Socket Chat",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        760, 520,
        nullptr, nullptr,
        hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
