#include "math.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool Math::WorldToScreen(const Vector3& worldPos, Vector2& screenPos, const ViewMatrix& viewMatrix, int screenWidth, int screenHeight) {
    Vector3 transform;
    transform.x = viewMatrix.matrix[0][0] * worldPos.x + viewMatrix.matrix[0][1] * worldPos.y + viewMatrix.matrix[0][2] * worldPos.z + viewMatrix.matrix[0][3];
    transform.y = viewMatrix.matrix[1][0] * worldPos.x + viewMatrix.matrix[1][1] * worldPos.y + viewMatrix.matrix[1][2] * worldPos.z + viewMatrix.matrix[1][3];
    transform.z = viewMatrix.matrix[3][0] * worldPos.x + viewMatrix.matrix[3][1] * worldPos.y + viewMatrix.matrix[3][2] * worldPos.z + viewMatrix.matrix[3][3];

    if (transform.z < 0.001f) {
        return false; // Behind camera
    }

    screenPos.x = (screenWidth / 2.0f) + (transform.x / transform.z) * (screenWidth / 2.0f);
    screenPos.y = (screenHeight / 2.0f) - (transform.y / transform.z) * (screenHeight / 2.0f);

    return (screenPos.x >= 0 && screenPos.x <= screenWidth && screenPos.y >= 0 && screenPos.y <= screenHeight);
}

float Math::Distance3D(const Vector3& pos1, const Vector3& pos2) {
    return pos1.Distance(pos2);
}

Vector3 Math::CalcAngle(const Vector3& src, const Vector3& dst) {
    Vector3 diff = dst - src;
    float distance = sqrt(diff.x * diff.x + diff.y * diff.y);
    
    Vector3 angle;
    angle.x = (float)(atan2(-diff.z, distance) * 180.0 / M_PI); // Pitch
    angle.y = (float)(atan2(diff.y, diff.x) * 180.0 / M_PI);    // Yaw
    angle.z = 0.0f; // Roll
    
    return angle;
}

bool Math::GetBoundingBox(const Vector3& playerPos, const ViewMatrix& viewMatrix, int screenWidth, int screenHeight, 
                         Vector2& topLeft, Vector2& bottomRight) {
    Vector3 origin = playerPos;
    Vector3 head = origin;
    head.z += 72.0f; // Approximate player height
    
    Vector2 originScreen, headScreen;
    
    if (!WorldToScreen(origin, originScreen, viewMatrix, screenWidth, screenHeight) ||
        !WorldToScreen(head, headScreen, viewMatrix, screenWidth, screenHeight)) {
        return false;
    }
    
    float height = abs(originScreen.y - headScreen.y);
    float width = height * 0.65f; // Approximate width ratio
    
    topLeft.x = headScreen.x - width / 2.0f;
    topLeft.y = headScreen.y;
    bottomRight.x = headScreen.x + width / 2.0f;
    bottomRight.y = originScreen.y;
    
    return true;
}

// ============================================================================
// Implementation of new Math functions
// ============================================================================

float Math::Distance2D(const Vector2& pos1, const Vector2& pos2) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return sqrt(dx * dx + dy * dy);
}

float Math::CalculateFOV(const Vector3& viewAngles, const Vector3& aimAngles) {
    Vector3 delta = viewAngles - aimAngles;
    delta = NormalizeAngles(delta);
    return sqrt(delta.x * delta.x + delta.y * delta.y);
}

Vector3 Math::NormalizeAngles(const Vector3& angles) {
    Vector3 normalized = angles;
    
    // Normalize pitch [-89, 89]
    if (normalized.x > 89.0f) normalized.x = 89.0f;
    if (normalized.x < -89.0f) normalized.x = -89.0f;
    
    // Normalize yaw [-180, 180]
    while (normalized.y > 180.0f) normalized.y -= 360.0f;
    while (normalized.y < -180.0f) normalized.y += 360.0f;
    
    // Roll should be 0
    normalized.z = 0.0f;
    
    return normalized;
}

Vector3 Math::ClampAngles(const Vector3& angles) {
    return NormalizeAngles(angles);
}

Vector3 Math::LerpAngles(const Vector3& current, const Vector3& target, float factor) {
    if (factor <= 0.0f) return current;
    if (factor >= 1.0f) return target;
    
    Vector3 diff = target - current;
    diff = NormalizeAngles(diff);
    
    return current + (diff * factor);
}

bool Math::IsOnScreen(const Vector2& screenPos, int screenWidth, int screenHeight) {
    return (screenPos.x >= 0 && screenPos.x <= screenWidth && 
            screenPos.y >= 0 && screenPos.y <= screenHeight);
}

Vector3 Math::GetHeadPosition(const Vector3& playerPos, float height) {
    Vector3 headPos = playerPos;
    headPos.z += height;
    return headPos;
}

void Math::AngleVectors(const Vector3& angles, Vector3* forward, Vector3* right, Vector3* up) {
    float sr, sp, sy, cr, cp, cy;
    
    const float deg2rad = static_cast<float>(M_PI) / 180.0f;
    
    sy = sinf(angles.y * deg2rad);
    cy = cosf(angles.y * deg2rad);
    sp = sinf(angles.x * deg2rad);
    cp = cosf(angles.x * deg2rad);
    sr = sinf(angles.z * deg2rad);
    cr = cosf(angles.z * deg2rad);
    
    if (forward) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }
    
    if (right) {
        right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right->y = (-1 * sr * sp * sy + -1 * cr * cy);
        right->z = -1 * sr * cp;
    }
    
    if (up) {
        up->x = (cr * sp * cy + -sr * -sy);
        up->y = (cr * sp * sy + -sr * cy);
        up->z = cr * cp;
    }
}
