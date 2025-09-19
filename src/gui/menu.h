#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <string>
#include <vector>
#include "../features/visuals/visuals.h"
#include "../features/legit/legit.h"

struct ImFont;

namespace gui
{
	inline bool open = true;
	inline float menuAlpha = 1.0f;
	inline float animationProgress = 0.0f;
	
	extern ImFont* medium;
	extern ImFont* bold;
	extern ImFont* tab_icons;
	extern ImFont* logo;
	extern ImFont* tab_title;
	extern ImFont* tab_title_icon;
	extern ImFont* small_font;
	extern ImFont* large_font;

	enum MainTabs {
		TAB_LEGIT,
		TAB_VISUALS
	};

	enum LegitSubTabs {
		LEGIT_AIMBOT,
		LEGIT_TRIGGERBOT
	};

	enum VisualsSubTabs {
		VISUALS_PLAYERS,
		VISUALS_WORLD,
		VISUALS_VIEWMODEL,
		VISUALS_OTHER
	};



	enum WeaponGroup {
		WEAPON_GROUP_GENERAL,
		WEAPON_GROUP_PISTOL,
		WEAPON_GROUP_RIFLE,
		WEAPON_GROUP_SMG,
		WEAPON_GROUP_SNIPER,
		WEAPON_GROUP_SHOTGUN,
		WEAPON_GROUP_MACHINEGUN
	};

	void Setup();
	void SetupMenu(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain) noexcept;
	void Destroy() noexcept;
	void Render() noexcept;

	bool CustomTab(const char* icon, bool selected);
	bool CustomSubTab(const char* label, bool selected, float width = 100.0f);
	void BeginCustomChild(const char* name, const ImVec2& size, bool has_border = true);
	void EndCustomChild();
	bool CustomToggleButton(const char* label, bool* active, float width = 200.0f);
	bool CustomSlider(const char* label, int* value, int min_val, int max_val, float width = 200.0f);
	bool CustomSliderFloat(const char* label, float* value, float min_val, float max_val, const char* format = "%.1f", float width = 200.0f);
	bool CustomColorPicker(const char* label, float color[4], float width = 200.0f);
	bool CustomCombo(const char* label, int* current_item, const char* const items[], int items_count, float width = 200.0f);
	bool CustomMultiCombo(const char* label, std::vector<bool>& selected, const char* const items[], int items_count, float width = 200.0f);
	bool CustomKeySelector(const char* label, int* keyCode, bool* isSettingKey, float width = 200.0f);
	void CustomSeparator(const char* label = nullptr);
	void CustomTooltip(const char* text);
	
	void DrawNotification(const char* text, float duration = 3.0f);
	void DrawWatermark();
	void DrawKeybindList();
	void DrawSpectatorList();
	void DrawRadar();
	
	float GetAnimatedValue(float from, float to, float speed = 8.0f);
	void PushItemAnimation(float progress);
	void PopItemAnimation();
}