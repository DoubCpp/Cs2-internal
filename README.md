# CS2 Internal Cheat

Basic Esp and Aimbot internal CS2 cheat with ImGui interface.

## Features
- **Aimbot** - Configurable aim assistance
- **Triggerbot** - Automatic firing
- **ESP** - Player/box/health/distance visuals

## Build
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## Usage
1. Launch CS2
2. Inject `Internal-Menu-Cs2.dll` 
3. Press `INSERT` to open menu
4. Press `END` to unload

## Controls
- `INSERT` - Toggle menu
- `Mouse4` - Aimbot
- `Mouse5` - Triggerbot
- `END` - Unload

## 🛠️ Installation

### Build from source

1. **Clone the repository**
   ```bash
   git clone https://github.com/doubcpp/cs2-internal.git
   cd cs2-internal
   ```

2. **Generate build files**
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```

3. **Compile the project**
   ```bash
   cmake --build . --config Release
   ```

4. **Locate compiled files**
   - DLL: `bin/Release/Internal-Menu-Cs2.dll`
   - Injector: `bin/Release/Internal-Menu-Injector.exe`

## 🏗️ Architecture

```
src/
├── core/                   # Core functions
│   ├── math.cpp/h         # Mathematical calculations
│   └── memory.cpp/h       # Memory management
├── features/              # Main features
│   ├── legit/            # Aimbot and triggerbot
│   └── visuals/          # ESP and visuals
├── game/                 # Game interface
│   ├── entities.h        # Entity structures
│   └── pattern.cpp/h     # Pattern scanner
├── gui/                  # User interface
│   └── menu.cpp/h        # ImGui menu
└── dllmain.cpp          # Entry point
```

## ⚠️ Disclaimer

This project is for educational purposes only. Using cheats in online games may result in:
- Permanent account ban
- VAC restrictions
- Terms of service violations

**Use at your own risk.**

---

