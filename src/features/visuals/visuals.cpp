#include "visuals.h"
#include "esp.h"
#include <iostream>

VisualSettings Visuals::settings;
ViewMatrix Visuals::viewMatrix;
int Visuals::screenWidth = 1920;
int Visuals::screenHeight = 1080;

void Visuals::Initialize() {
    std::cout << "[Visuals] Visual system initialized\n";
    
    // Initialize sub-modules
    ESP::Initialize();
}

void Visuals::UpdateViewMatrix() {
    viewMatrix = Memory::GetViewMatrix();
}

void Visuals::UpdateScreenSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void Visuals::Render() {
    // Update view matrix
    __try {
        UpdateViewMatrix();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cout << "[Visuals] ViewMatrix read failed" << std::endl;
        return;
    }
    
    // Delegate to ESP module for rendering
    ESP::Render();
}

bool Visuals::IsValidEntity(C_BaseEntity* entity) {
    if (!entity) return false;
    
    __try {
        // Try to read a basic property to validate the entity
        int health = entity->GetHealth();
        return health >= 0 && health <= 100;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool Visuals::IsEnemy(C_BaseEntity* localPlayer, C_BaseEntity* target) {
    if (!localPlayer || !target) return false;
    
    int localTeam = localPlayer->GetTeamNum();
    int targetTeam = target->GetTeamNum();
    
    return localTeam != targetTeam;
}

ImU32 Visuals::GetTeamColor(C_BaseEntity* localPlayer, C_BaseEntity* target) {
    // This function is deprecated - colors are now handled individually
    // Return white as fallback
    return IM_COL32(255, 255, 255, 255);
}