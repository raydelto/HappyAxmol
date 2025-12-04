# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

HappyAxmol is a 2D mobile game built with the Axmol game engine (version 3.0.0-b86f4d3). The game is a physics-based arcade game where players control a character at the bottom of the screen, avoiding falling bombs while trying to explode them by tapping. The project demonstrates a typical Axmol game structure with multi-platform support.

**Package**: com.raycasters.happybunny

## Prerequisites

Before working with this project, ensure the `AX_ROOT` environment variable is set to point to the Axmol engine installation. The build system expects this to locate the engine. If not embedded, the project uses the system's Axmol installation via this environment variable.

## Build Commands

### Windows (Primary Platform)
```batch
# Build for Win32 (creates build/ directory)
axmol build -p win32

# Alternative: Use provided script
build.bat
```

### Build Output
- Build artifacts are placed in `build/bin/HappyAxmol/{BuildConfig}/`
- Build configurations: Debug, Release, MinSizeRel, RelWithDebInfo
- Content folder is copied automatically to output directory

### Running the Application
```batch
# Run with specific build config (defaults to Debug if not specified)
run.bat [BuildConfig]

# Example: run Debug build
run.bat Debug

# The script caches your last build config choice in run.bat.txt
```

### Multi-Platform Support
The project includes platform-specific project folders:
- `proj.win32/` - Windows desktop
- `proj.android/` - Android (Gradle-based)
- `proj.ios_mac/` - iOS and macOS
- `proj.linux/` - Linux
- `proj.wasm/` - WebAssembly
- `proj.winrt/` - Windows Runtime (UWP)

For Android builds, use Gradle in the `proj.android/` directory.

## Project Architecture

### Core Application Flow
The application follows the Axmol pattern:
1. **AppDelegate** (`AppDelegate.h/cpp`) - Application lifecycle manager
   - Initializes rendering context and Director
   - Sets up multi-resolution support with three asset tiers (low/mid/high) based on screen height
   - Design resolution: 768x1280 (portrait orientation)
   - Manages audio engine lifecycle
   - Creates and launches the main scene

2. **MainScene** (`MainScene.h/cpp`) - Primary gameplay scene
   - Inherits from `ax::Scene` and uses physics integration via `initWithPhysics()`
   - Manages game state through `GameState` enum (init, update, pause, end, menu1, menu2)
   - Implements touch, mouse, keyboard, and accelerometer input handlers
   - Uses Axmol's physics system (Chipmunk2D wrapper) for collision detection

### Physics System
The game uses Axmol's built-in physics engine with these patterns:
- **Physics World**: Zero gravity (set in `MainScene::init()`)
- **Physics Bodies**: Created via `PhysicsBody::createCircle()` for circular collision shapes
- **Collision Detection**: Contact listeners with `onContactBegin` callbacks
- **Body Properties**: All sprites use `setContactTestBitmask(true)` and `setDynamic(true)`
- Bombs fall with velocity set via `setVelocity(ax::Vec2(0, -100))` and variants

### Scene Management
- **MainScene** - Main gameplay
- **GameOverScene** (`GameOverScene.h/cpp`) - Displayed on collision
- **PauseScene** (`PauseScene.h/cpp`) - Pause overlay
- Scene transitions use `TransitionFlipX::create(1.0, scene)`

### Asset Management
Assets are organized in the `Content/` directory with resolution-based search paths:
- `images/high/` - For screens > 800px height (scale factor: 1280.0/1280)
- `images/mid/` - For screens > 600px height (scale factor: 800.0/1280)
- `images/low/` - For screens ≤ 600px height (scale factor: 320.0/1280)
- `sounds/` - Audio files (music.mp3, bomb.mp3, uh.mp3)
- `particles/` - Particle effect definitions (.plist files)
- `fonts/` - Font resources

The engine automatically selects the appropriate asset tier at startup based on frame buffer size.

### Audio System
Uses Axmol's `AudioEngine` (not the deprecated CocosDenshion):
- Background music with looping support
- Sound effects for collisions and explosions
- Volume control and mute functionality
- Audio preloading for frequently used sounds

### Input Handling
Multiple input methods are supported simultaneously:
- **Touch**: Tap to explode bombs, drag to move player
- **Accelerometer**: Tilt device to move player (mobile platforms)
- **Keyboard**: Back button handling for Android
- **Mouse**: Desktop platform support

## CMake Build System

The project uses a modular CMake system with these modules (in `cmake/modules/`):
1. **AXGameEngineOptions.cmake** - Feature flags and extension configuration
2. **AXGameEngineSetup.cmake** - Locates Axmol engine (via AX_ROOT or embedded)
3. **AXGameSourceSetup.cmake** - Configures source files
4. **AXGameTargetSetup.cmake** - Creates build targets
5. **AXGamePlatformSetup.cmake** - Platform-specific configurations
6. **AXGameFinalSetup.cmake** - Final build configuration

### CMake Configuration Notes
- Minimum CMake version: 3.22
- Source files are auto-discovered via `file(GLOB_RECURSE)` in `Source/` directory
- The build system supports both embedded engine (local `axmol/` folder) and system-installed engine
- To change engine features (physics, 3D, extensions), modify `AXGameEngineOptions.cmake` and delete `CMakeCache.txt`

## Code Patterns

### Memory Management
Axmol uses reference counting with automatic memory management:
- Objects created with `create()` are autoreleased
- Manual retain/release rarely needed
- Prefer `ax::Vector<T>` over `std::vector` for Axmol objects

### Scheduling and Updates
- Use `scheduleUpdate()` for per-frame updates via `update(float delta)`
- Use `schedule(AX_SCHEDULE_SELECTOR(Class::method), interval)` for timed callbacks
- Example: Score updates every 3 seconds, bomb spawning every 8 seconds

### Scene Lifecycle
- `init()` - Initialize scene, return true on success
- `update(float delta)` - Per-frame updates (requires `scheduleUpdate()`)
- Input callbacks registered via `EventDispatcher` listeners

## Key Dependencies

The project uses these Axmol features:
- Physics engine (enabled by default)
- Audio engine
- Sprite animations
- Particle systems
- Touch/mouse/keyboard/accelerometer input
- Multi-resolution support
- Scene transitions

## Debugging

- FPS display is enabled by default: `director->setStatsDisplay(true)` in AppDelegate
- Physics debug draw can be enabled via `getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL)`
- Use `AXLOGD()` macro for debug logging (visible in debug builds)
