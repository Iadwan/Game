# Game — Learning SDL3 with C++

A hands-on learning log for **SDL3** and modern C++ game programming, built one commit at a time.
This repo is intentionally kept simple and readable — the goal isn't a polished game, it's a
reference other beginners can read top-to-bottom to see how SDL3's pieces fit together.

If you're new to SDL3 and learning by reading real (if messy) code, this repo is for you.

## What's in here

Everything lives in a single file, [main.cpp](main.cpp), on purpose — no engine abstraction to get
lost in. It's a sandbox that grew feature by feature, and each feature demonstrates one SDL3 idea:

| Concept | Where to look | What it teaches |
|---|---|---|
| App lifecycle callbacks | `SDL_AppInit` / `SDL_AppEvent` / `SDL_AppIterate` / `SDL_AppQuit` | SDL3's newer callback-driven main loop (`SDL_MAIN_USE_CALLBACKS`) instead of a hand-rolled `while` loop |
| Window & renderer setup | `SDL_AppInit` | Creating a resizable window and a hardware-accelerated renderer |
| Loading textures | `Sprite::Load`, `AssetPath` | Loading a PNG into an `SDL_Surface` (via SDL3_image), uploading it to an `SDL_Texture`, and freeing the surface |
| Cross-platform asset paths | `AssetPath()` | Using `SDL_GetBasePath()` so assets load correctly whether you run from Visual Studio or double-click the `.exe` |
| Sprite-sheet animation | `Sprite::Render`, `spriteFrame` switch | Slicing a sprite sheet into frames with a source `SDL_FRect` and stepping through them on a timer |
| Keyboard input | `SDL_AppEvent` | Reading discrete key-down events (`SDL_EVENT_KEY_DOWN`) vs. the live keyboard-state array (`SDL_GetKeyboardState`) for chorded keys |
| Mouse input | `SDL_AppEvent` | Handling `SDL_EVENT_MOUSE_MOTION` |
| Immediate-mode shapes | `SDL_RenderFillRect`, `DrawCircle`, `SDL_RenderGeometry` | Filled rectangles, a hand-rolled midpoint-ish filled circle, and raw triangle geometry (two different ways to fill an `SDL_Vertex[]`) |
| Movement & screen wrapping | `squareX/Y`, `circleX/Y`, `ballX/Y`, `tri2Y` | Basic per-frame translation and wrap-around-at-the-edge logic |
| AABB collision detection | `SDL_HasRectIntersectionFloat` | Rectangle-vs-rectangle collision with edge-triggered "hit" counting (`wasColliding`) so a single overlap counts once |
| Audio playback | `SDL_LoadWAV`, `SDL_OpenAudioDeviceStream`, `SDL_PutAudioStreamData` | Loading a WAV and streaming it to the default playback device |
| Debug text overlay | `SDL_RenderDebugText`, `SDL_RenderDebugTextFormat` | SDL3's built-in debug text renderer — no font loading required |
| Frame limiting | end of `SDL_AppIterate` | A simple sleep-based frame limiter (`SDL_GetTicks` + `SDL_Delay`) so movement speed stays consistent across machines |

## Project structure

```
Game/
├── main.cpp          # Everything: app lifecycle, rendering, input, audio, collision
├── Game.vcxproj       # Visual Studio project (Win32 + x64, Debug + Release)
├── Game.sln           # Visual Studio solution
├── SDL3.dll           # Runtime DLL (copied next to the .exe on build)
└── assets/
    ├── 508.png         # Player sprite sheet
    ├── walk.png         # Alternate walk sprite sheet
    ├── explode.png       # Explosion sprite sheet
    ├── spare.png         # Simple moving texture
    └── Goblins_Dance.wav # Background audio clip
```

## Prerequisites

- **Windows** with **Visual Studio 2022** (or newer), "Desktop development with C++" workload
- **[SDL3](https://github.com/libsdl-org/SDL)** and **[SDL3_image](https://github.com/libsdl-org/SDL_image)** — headers and `.lib` files

> The `.vcxproj` currently points at local paths (`C:\Library\SDL3-3.4.14` and
> `C:\Library\SDL3_image-3.4.4`). If you clone this repo, update
> **Project Properties → VC++ Directories** (or edit `Game.vcxproj` directly) to point at wherever
> you install SDL3 and SDL3_image on your machine.

## Getting started

1. Install SDL3 and SDL3_image (grab prebuilt dev libraries from their GitHub Releases pages, or
   build from source).
2. Clone this repo and open [Game.sln](Game.sln) in Visual Studio.
3. Point the project's include/library directories at your SDL3 / SDL3_image install (see note
   above).
4. Build (x64 | Debug or Release). The post-build step copies `assets/` and `SDL3.dll` next to the
   compiled `.exe` automatically.
5. Run — either with the debugger (F5) or by launching the `.exe` from the output folder directly;
   `AssetPath()` resolves assets relative to the executable either way.

## Controls

| Input | Effect |
|---|---|
| Arrow keys | Move the green ball 10px per press |
| <kbd>A</kbd> + <kbd>S</kbd> together | Logs a chorded key-press example |
| Mouse move | Logs cursor position to the console |
| Close window | Quits |

Watch the console output — most input handling here logs via `SDL_Log` so you can see events as
they happen while you read the corresponding code.

## Why the callback-based main loop?

The classic SDL2-style pattern is a hand-written loop:

```cpp
while (running) {
    while (SDL_PollEvent(&event)) { /* handle */ }
    /* update */
    /* render */
}
```

SDL3 offers an alternative: define `SDL_MAIN_USE_CALLBACKS` and implement four functions —
`SDL_AppInit`, `SDL_AppEvent`, `SDL_AppIterate`, `SDL_AppQuit` — and SDL owns the loop itself. This
matters more than it looks: it's the same shape SDL uses to support platforms where you don't get
to own `main()` at all (Emscripten/web, iOS, some consoles). This repo uses that pattern throughout,
so it doubles as a working example of the callback style if you're used to the classic loop.

## Ideas for extending this (good next exercises)

If you're learning alongside this repo, these are natural next steps that build on what's already
here:

- Replace the hardcoded sprite-sheet frame `switch` with a data-driven animation (frame count +
  row, looped via modulo).
- Extract the movement/collision code for the ball and the `spareRect` player into a reusable
  `Entity` struct instead of loose variables on `SDLApplication`.
- Delta-time-based movement instead of fixed per-frame pixel steps, so speed doesn't depend on
  `TARGET_FPS`.
- Swap the AABB collision for circle-vs-circle now that there's already a hand-drawn circle to
  compare against.
- Add a simple state machine (menu → playing → game over) using the `SDL_AppIterate` return value.

## Resources

- [SDL3 Wiki](https://wiki.libsdl.org/SDL3/FrontPage) — the primary reference for every `SDL_*`
  call in this file
- [SDL2 → SDL3 migration guide](https://github.com/libsdl-org/SDL/blob/main/docs/README-migration.md)
  — useful if you're coming from SDL2 tutorials, which are still the most common ones online
- [SDL3_image](https://github.com/libsdl-org/SDL_image) — used here for PNG loading

## License

No license has been chosen yet, so all rights are reserved by default. If you'd like to reuse this
code, open an issue and ask.

---

This is a personal learning project, shared publicly in case it helps someone else picking up
SDL3. Issues and suggestions are welcome — especially "here's a cleaner way to do X" pointers.
