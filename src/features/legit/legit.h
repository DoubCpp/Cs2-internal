#pragma once
#include "../../core/math.h"
#include "../../core/memory.h"
#include "../../game/entities.h"
#include "../../../dependencies/imgui/imgui.h"
#include <Windows.h>

enum TargetHitbox {
    HITBOX_HEAD = 0,
    HITBOX_NECK = 1,
    HITBOX_CHEST = 2,
    HITBOX_STOMACH = 3,
    HITBOX_NEAREST = 4
};

enum TargetPriority {
    PRIORITY_CROSSHAIR = 0,
    PRIORITY_HEALTH = 1,
    PRIORITY_DISTANCE = 2
};

struct LegitSettings {
    bool aimbotEnabled = false;
    int aimbotFOV = 10;
    int aimbotSmooth = 5;
    int aimbotKey = VK_XBUTTON2;
    bool aimbotTeamCheck = false;
    bool aimbotDrawFOV = false;
    bool isSettingKey = false;
    
    int aimbotHitbox = HITBOX_HEAD;
    int aimbotPriority = PRIORITY_CROSSHAIR;
    
    bool triggerbotEnabled = false;
    int triggerbotDelay = 300;
    int triggerbotKey = VK_XBUTTON1;
    bool isSettingTriggerbotKey = false;
    int triggerbotHitbox = HITBOX_HEAD;
    bool triggerbotTeamCheck = false;
};

class Aimbot;
class Triggerbot;

class Legit {
private:
    static LegitSettings settings;

public:
    static void Initialize();
    static void Update();
    static void Render();
    
    static LegitSettings& GetSettings() { return settings; }
    
    static bool IsKeyPressed(int key);
    static int DetectPressedKey();
};