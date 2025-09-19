#include "menu.h"
#include "../features/legit/legit.h"
#include "../features/visuals/visuals.h"
#include "../core/memory.h"
#include "../game/entities.h"
#include "../../dependencies/imgui/imgui.h"
#include <string>
#include <iostream>
#include <Windows.h>

namespace gui {
    static int current_tab = TAB_LEGIT;

    void Setup() { 
    }

    void SetupMenu(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain) noexcept {
    }

    void Destroy() noexcept {
    }

    std::string VirtualKeyToString(int vkCode) {
        if (vkCode == 0) return "None";
        
        switch (vkCode) {
            case VK_LBUTTON: return "Mouse1";
            case VK_RBUTTON: return "Mouse2";
            case VK_MBUTTON: return "Mouse3";
            case VK_XBUTTON1: return "Mouse4";
            case VK_XBUTTON2: return "Mouse5";
            case VK_BACK: return "Backspace";
            case VK_TAB: return "Tab";
            case VK_CLEAR: return "Clear";
            case VK_RETURN: return "Enter";
            case VK_SHIFT: return "Shift";
            case VK_CONTROL: return "Ctrl";
            case VK_MENU: return "Alt";
            case VK_PAUSE: return "Pause";
            case VK_CAPITAL: return "CapsLock";
            case VK_ESCAPE: return "Escape";
            case VK_SPACE: return "Space";
            case VK_PRIOR: return "PageUp";
            case VK_NEXT: return "PageDown";
            case VK_END: return "End";
            case VK_HOME: return "Home";
            case VK_LEFT: return "Left";
            case VK_UP: return "Up";
            case VK_RIGHT: return "Right";
            case VK_DOWN: return "Down";
            case VK_SELECT: return "Select";
            case VK_PRINT: return "Print";
            case VK_EXECUTE: return "Execute";
            case VK_SNAPSHOT: return "PrintScreen";
            case VK_INSERT: return "Insert";
            case VK_DELETE: return "Delete";
            case VK_HELP: return "Help";
            case VK_LWIN: return "LWin";
            case VK_RWIN: return "RWin";
            case VK_APPS: return "Apps";
            case VK_SLEEP: return "Sleep";
            case VK_NUMPAD0: return "Num0";
            case VK_NUMPAD1: return "Num1";
            case VK_NUMPAD2: return "Num2";
            case VK_NUMPAD3: return "Num3";
            case VK_NUMPAD4: return "Num4";
            case VK_NUMPAD5: return "Num5";
            case VK_NUMPAD6: return "Num6";
            case VK_NUMPAD7: return "Num7";
            case VK_NUMPAD8: return "Num8";
            case VK_NUMPAD9: return "Num9";
            case VK_MULTIPLY: return "Num*";
            case VK_ADD: return "Num+";
            case VK_SEPARATOR: return "NumSep";
            case VK_SUBTRACT: return "Num-";
            case VK_DECIMAL: return "Num.";
            case VK_DIVIDE: return "Num/";
            case VK_F1: return "F1";
            case VK_F2: return "F2";
            case VK_F3: return "F3";
            case VK_F4: return "F4";
            case VK_F5: return "F5";
            case VK_F6: return "F6";
            case VK_F7: return "F7";
            case VK_F8: return "F8";
            case VK_F9: return "F9";
            case VK_F10: return "F10";
            case VK_F11: return "F11";
            case VK_F12: return "F12";
            case VK_F13: return "F13";
            case VK_F14: return "F14";
            case VK_F15: return "F15";
            case VK_F16: return "F16";
            case VK_F17: return "F17";
            case VK_F18: return "F18";
            case VK_F19: return "F19";
            case VK_F20: return "F20";
            case VK_F21: return "F21";
            case VK_F22: return "F22";
            case VK_F23: return "F23";
            case VK_F24: return "F24";
            case VK_NUMLOCK: return "NumLock";
            case VK_SCROLL: return "ScrollLock";
            case VK_LSHIFT: return "LShift";
            case VK_RSHIFT: return "RShift";
            case VK_LCONTROL: return "LCtrl";
            case VK_RCONTROL: return "RCtrl";
            case VK_LMENU: return "LAlt";
            case VK_RMENU: return "RAlt";
            case VK_OEM_1: return ";";
            case VK_OEM_PLUS: return "+";
            case VK_OEM_COMMA: return ",";
            case VK_OEM_MINUS: return "-";
            case VK_OEM_PERIOD: return ".";
            case VK_OEM_2: return "/";
            case VK_OEM_3: return "`";
            case VK_OEM_4: return "[";
            case VK_OEM_5: return "\\";
            case VK_OEM_6: return "]";
            case VK_OEM_7: return "'";
            case 0x30: return "0";
            case 0x31: return "1";
            case 0x32: return "2";
            case 0x33: return "3";
            case 0x34: return "4";
            case 0x35: return "5";
            case 0x36: return "6";
            case 0x37: return "7";
            case 0x38: return "8";
            case 0x39: return "9";
            default:
                if (vkCode >= 'A' && vkCode <= 'Z') {
                    return std::string(1, (char)vkCode);
                }
                return "Key" + std::to_string(vkCode);
        }
    }

    void RenderLegitTab() {
        auto& legitSettings = Legit::GetSettings();
        
        if (ImGui::BeginTabBar("LegitTabs")) {
            if (ImGui::BeginTabItem("Aimbot")) {
                ImGui::Checkbox("Enable Aimbot", &legitSettings.aimbotEnabled);
                ImGui::Checkbox("Team Check", &legitSettings.aimbotTeamCheck);
                ImGui::Checkbox("Draw FOV", &legitSettings.aimbotDrawFOV);
                
                ImGui::SliderInt("FOV", &legitSettings.aimbotFOV, 1, 180);
                ImGui::SliderInt("Smooth", &legitSettings.aimbotSmooth, 1, 20);
                
                const char* hitboxes[] = { "Head", "Neck", "Chest", "Stomach" };
                ImGui::Combo("Hitbox", &legitSettings.aimbotHitbox, hitboxes, IM_ARRAYSIZE(hitboxes));
                
                static bool setting_aimbot_key = false;
                std::string keyText = "Aimbot Key: " + VirtualKeyToString(legitSettings.aimbotKey);
                if (ImGui::Button(keyText.c_str())) {
                    setting_aimbot_key = !setting_aimbot_key;
                }
                if (setting_aimbot_key) {
                    ImGui::SameLine();
                    ImGui::Text("Press any key...");
                    for (int i = 1; i < 256; i++) {
                        if (GetAsyncKeyState(i) & 0x8000) {
                            legitSettings.aimbotKey = i;
                            setting_aimbot_key = false;
                            break;
                        }
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Triggerbot")) {
                ImGui::Checkbox("Enable Triggerbot", &legitSettings.triggerbotEnabled);
                ImGui::Checkbox("Team Check", &legitSettings.triggerbotTeamCheck);
                
                ImGui::SliderInt("Delay (ms)", &legitSettings.triggerbotDelay, 0, 500);
                
                const char* triggerHitboxes[] = { "Head", "Neck", "Chest", "Stomach" };
                ImGui::Combo("Hitbox", &legitSettings.triggerbotHitbox, triggerHitboxes, IM_ARRAYSIZE(triggerHitboxes));
                
                static bool setting_trigger_key = false;
                std::string triggerKeyText = "Triggerbot Key: " + VirtualKeyToString(legitSettings.triggerbotKey);
                if (ImGui::Button(triggerKeyText.c_str())) {
                    setting_trigger_key = !setting_trigger_key;
                }
                if (setting_trigger_key) {
                    ImGui::SameLine();
                    ImGui::Text("Press any key...");
                    for (int i = 1; i < 256; i++) {
                        if (GetAsyncKeyState(i) & 0x8000) {
                            legitSettings.triggerbotKey = i;
                            setting_trigger_key = false;
                            break;
                        }
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }

    void RenderVisualsTab() {
        auto& visualSettings = Visuals::GetSettings();
        
        if (ImGui::BeginTabBar("VisualTabs")) {
            if (ImGui::BeginTabItem("Player ESP")) {
                if (ImGui::BeginChild("PlayerESP", ImVec2(0, 400), true)) {
                    ImGui::Text("Player ESP Settings");
                    ImGui::Separator();
                    
                    if (ImGui::Checkbox("Box ESP", &visualSettings.boxESP)) {
                        if (visualSettings.boxESP) {
                            visualSettings.cornerBoxESP = false;
                        }
                    }
                    if (ImGui::Checkbox("Corner Box ESP", &visualSettings.cornerBoxESP)) {
                        if (visualSettings.cornerBoxESP) {
                            visualSettings.boxESP = false;
                        }
                    }
                    
                    ImGui::Checkbox("Name ESP", &visualSettings.nameESP);
                    ImGui::Checkbox("Health ESP", &visualSettings.healthESP);
                    ImGui::Checkbox("Distance ESP", &visualSettings.distanceESP);
                    ImGui::Checkbox("Snapline ESP", &visualSettings.snaplineESP);
                    ImGui::Checkbox("Skeleton ESP", &visualSettings.skeletonESP);
                    ImGui::Checkbox("Team ESP", &visualSettings.teamESP);
                    
                    ImGui::Separator();
                    ImGui::Text("ESP Settings");
                    ImGui::SliderInt("Line Thickness", &visualSettings.lineThickness, 1, 5);
                    ImGui::SliderInt("ESP Range (m)", &visualSettings.espRange, 10, 500);
                    visualSettings.maxDistance = (float)(visualSettings.espRange * 50);
                    
                    ImGui::Separator();
                    ImGui::Text("Enemy Colors");
                    ImGui::ColorEdit4("Enemy Box", visualSettings.enemyBoxColor);
                    ImGui::ColorEdit4("Enemy Name", visualSettings.enemyNameColor);
                    ImGui::ColorEdit4("Enemy Distance", visualSettings.enemyDistanceColor);
                    ImGui::ColorEdit4("Enemy Snapline", visualSettings.enemySnaplineColor);
                    ImGui::ColorEdit4("Enemy Skeleton", visualSettings.enemySkeletonColor);
                    
                    ImGui::Separator();
                    ImGui::Text("Team Colors");
                    ImGui::ColorEdit4("Team Box", visualSettings.teamBoxColor);
                    ImGui::ColorEdit4("Team Name", visualSettings.teamNameColor);
                    ImGui::ColorEdit4("Team Distance", visualSettings.teamDistanceColor);
                    ImGui::ColorEdit4("Team Snapline", visualSettings.teamSnaplineColor);
                    ImGui::ColorEdit4("Team Skeleton", visualSettings.teamSkeletonColor);
                    
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }

    void Render() noexcept {
        if (!open) return;

        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
        
        if (ImGui::Begin("CS2 Internal Cheat", &open)) {
            if (ImGui::BeginTabBar("MainTabs")) {
                if (ImGui::BeginTabItem("Legit")) {
                    current_tab = TAB_LEGIT;
                    RenderLegitTab();
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Visuals")) {
                    current_tab = TAB_VISUALS;
                    RenderVisualsTab();
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Debug")) {
                    ImGui::Text("Debug Information");
                    ImGui::Separator();
                    ImGui::Text("Cheat Info");
                    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                    ImGui::Text("Build: Release");
                    ImGui::Text("ImGui Version: %s", ImGui::GetVersion());
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    void Show() noexcept {
        open = true;
    }
    
    void Hide() noexcept {
        open = false;
    }
    
    void Toggle() noexcept {
        open = !open;
    }
    
    bool IsOpen() noexcept {
        return open;
    }
}