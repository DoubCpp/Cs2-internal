#pragma once
#include "../../core/math.h"
#include "../../core/memory.h"
#include "../../game/entities.h"
#include "../../../dependencies/imgui/imgui.h"

struct VisualSettings {
    bool boxESP = false;
    bool cornerBoxESP = false;
    bool nameESP = false;
    bool healthESP = false;
    bool distanceESP = false;
    bool snaplineESP = false;
    bool skeletonESP = false;
    
    float enemyBoxColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float enemyNameColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float enemyDistanceColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float enemySnaplineColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float enemySkeletonColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    
    float teamBoxColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float teamNameColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float teamDistanceColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float teamSnaplineColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float teamSkeletonColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    
    float healthColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    
    int lineThickness = 2;
    float maxDistance = 5000.0f;
    int espRange = 100;
    bool teamESP = false;
    
    bool grenadeESP = false;
    
    bool nightMode = false;
    bool fullBright = false;
    bool removeFog = false;
    bool removeSkybox = false;
};

class ESP;

class Visuals {
private:
    static VisualSettings settings;
    static ViewMatrix viewMatrix;
    static int screenWidth;
    static int screenHeight;

public:
    static void Initialize();
    static void UpdateViewMatrix();
    static void UpdateScreenSize(int width, int height);
    static void Render();
    
    static VisualSettings& GetSettings() { return settings; }
    static ViewMatrix& GetViewMatrix() { return viewMatrix; }
    static int GetScreenWidth() { return screenWidth; }
    static int GetScreenHeight() { return screenHeight; }
    
    static bool IsValidEntity(C_BaseEntity* entity);
    static bool IsEnemy(C_BaseEntity* localPlayer, C_BaseEntity* target);
    static ImU32 GetTeamColor(C_BaseEntity* localPlayer, C_BaseEntity* target);
};