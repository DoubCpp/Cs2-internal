#pragma once
#include "../game/entities.h"

class Math {
public:
    static bool WorldToScreen(const Vector3& worldPos, Vector2& screenPos, const ViewMatrix& viewMatrix, int screenWidth, int screenHeight);
    
    static float Distance3D(const Vector3& pos1, const Vector3& pos2);
    
    static Vector3 CalcAngle(const Vector3& src, const Vector3& dst);
    
    static void AngleVectors(const Vector3& angles, Vector3* forward, Vector3* right = nullptr, Vector3* up = nullptr);
    
    static bool GetBoundingBox(const Vector3& playerPos, const ViewMatrix& viewMatrix, int screenWidth, int screenHeight, 
                              Vector2& topLeft, Vector2& bottomRight);
    
    static float Distance2D(const Vector2& pos1, const Vector2& pos2);
    
    static float CalculateFOV(const Vector3& viewAngles, const Vector3& aimAngles);
    
    static Vector3 NormalizeAngles(const Vector3& angles);
    
    static Vector3 ClampAngles(const Vector3& angles);
    
    static Vector3 LerpAngles(const Vector3& current, const Vector3& target, float factor);
    
    static bool IsOnScreen(const Vector2& screenPos, int screenWidth, int screenHeight);
    
    static Vector3 GetHeadPosition(const Vector3& playerPos, float height = 72.0f);
};
