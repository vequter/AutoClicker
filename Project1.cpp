#include <Windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>

std::atomic<bool> uhlb(false), run(true), tabPressed(false);
HHOOK mh, kh;
int minc = 10, maxc = 20;

void show_info() {
    std::cout << "================================\n";
    std::cout << " AutoClicker v1.0\n";
    std::cout << " Developer: @vequter (Telegram)\n";
    std::cout << "================================\n\n";
}

void sendMouseClick(bool down) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

LRESULT CALLBACK mlp(int n, WPARAM w, LPARAM l) {
    if (n == HC_ACTION) {
        auto mi = (MSLLHOOKSTRUCT*)l;
        if ((w == WM_LBUTTONDOWN || w == WM_LBUTTONUP) && !(mi->flags & LLMHF_INJECTED)) {
            uhlb = (w == WM_LBUTTONDOWN);
        }
    }
    return CallNextHookEx(mh, n, w, l);
}

LRESULT CALLBACK klp(int n, WPARAM w, LPARAM l) {
    if (n == HC_ACTION) {
        KBDLLHOOKSTRUCT* ki = (KBDLLHOOKSTRUCT*)l;
        if (ki->vkCode == VK_TAB) {
            bool keyDown = (w == WM_KEYDOWN || w == WM_SYSKEYDOWN);
            bool keyUp = (w == WM_KEYUP || w == WM_SYSKEYUP);

            if (keyDown && !tabPressed.exchange(true)) {
                sendMouseClick(true);
                uhlb = true;
            }
            else if (keyUp && tabPressed.exchange(false)) {
                sendMouseClick(false);
                uhlb = false;
            }
        }
    }
    return CallNextHookEx(kh, n, w, l);
}

void clickThread() {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(minc, maxc);

    while (run) {
        if (uhlb && !tabPressed) {
            sendMouseClick(true);
            sendMouseClick(false);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / dis(gen)));
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Уменьшена задержка
        }
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    show_info();

    std::cout << "Min CPS (10-100): ";
    while (!(std::cin >> minc) || minc < 1 || minc > 100) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid input. Min CPS (1-100): ";
    }

    std::cout << "Max CPS (" << minc << "-100): ";
    while (!(std::cin >> maxc) || maxc < minc || maxc > 100) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid input. Max CPS (" << minc << "-100): ";
    }

    mh = SetWindowsHookEx(WH_MOUSE_LL, mlp, NULL, 0);
    kh = SetWindowsHookEx(WH_KEYBOARD_LL, klp, NULL, 0);
    if (!mh || !kh) {
        std::cerr << "Error: Failed to set hooks\n";
        return 1;
    }

    std::thread t(clickThread);
    std::cout << "\nRunning! Hold LMB to auto-click (" << minc << "-" << maxc << " CPS)\n";
    std::cout << "Press TAB to hold LMB, release TAB to release LMB\n";

    MSG msg;
    while (run && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) run = false;
    }

    t.join();
    UnhookWindowsHookEx(mh);
    UnhookWindowsHookEx(kh);
    return 0;
}