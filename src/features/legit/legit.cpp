#include "legit.h"
#include "aimbot.h"
#include "triggerbot.h"
#include "../../game/pattern.h"
#include "../visuals/visuals.h"
#include <iostream>
#include <Windows.h>
#include <cstring>
#include <cmath>

LegitSettings Legit::settings;

void Legit::Initialize() {
    std::cout << "[Legit] Initialized\n";
    
    Aimbot::Initialize();
    Triggerbot::Initialize();
}

void Legit::Update() {
    if (settings.isSettingKey) {
        int pressedKey = DetectPressedKey();
        if (pressedKey != 0) {
            settings.aimbotKey = pressedKey;
            settings.isSettingKey = false;
            Sleep(200);
        }
        return;
    }
    
    if (settings.isSettingTriggerbotKey) {
        int pressedKey = DetectPressedKey();
        if (pressedKey != 0) {
            settings.triggerbotKey = pressedKey;
            settings.isSettingTriggerbotKey = false;
            Sleep(200);
        }
        return;
    }
    
    Aimbot::Update();
    Triggerbot::Update();
}

void Legit::Render() {
    Aimbot::Render();
}

bool Legit::IsKeyPressed(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

int Legit::DetectPressedKey() {
    static DWORD lastDetectionTime = 0;
    DWORD currentTime = GetTickCount();
    
    // Prevent too frequent detection
    if (currentTime - lastDetectionTime < 100) {
        return 0;
    }
    
    // Check mouse buttons first (but not left click while in menu)
    if (IsKeyPressed(VK_RBUTTON)) { lastDetectionTime = currentTime; return VK_RBUTTON; }
    if (IsKeyPressed(VK_MBUTTON)) { lastDetectionTime = currentTime; return VK_MBUTTON; }
    if (IsKeyPressed(VK_XBUTTON1)) { lastDetectionTime = currentTime; return VK_XBUTTON1; }
    if (IsKeyPressed(VK_XBUTTON2)) { lastDetectionTime = currentTime; return VK_XBUTTON2; }
    
    // Check special keys
    if (IsKeyPressed(VK_SHIFT)) { lastDetectionTime = currentTime; return VK_SHIFT; }
    if (IsKeyPressed(VK_CONTROL)) { lastDetectionTime = currentTime; return VK_CONTROL; }
    if (IsKeyPressed(VK_MENU)) { lastDetectionTime = currentTime; return VK_MENU; }
    if (IsKeyPressed(VK_SPACE)) { lastDetectionTime = currentTime; return VK_SPACE; }
    if (IsKeyPressed(VK_TAB)) { lastDetectionTime = currentTime; return VK_TAB; }
    
    // Check letter keys
    for (int key = 0x41; key <= 0x5A; key++) { // A-Z
        if (IsKeyPressed(key)) {
            lastDetectionTime = currentTime;
            return key;
        }
    }
    
    // Check function keys
    for (int key = VK_F1; key <= VK_F12; key++) {
        if (IsKeyPressed(key)) {
            lastDetectionTime = currentTime;
            return key;
        }
    }
    
    // Left mouse button as last resort (avoid conflicts with menu clicks)
    if (IsKeyPressed(VK_LBUTTON)) { 
        lastDetectionTime = currentTime; 
        return VK_LBUTTON; 
    }
    
    return 0; // No key pressed
}