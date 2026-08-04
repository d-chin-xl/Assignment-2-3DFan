# Assignment 2 — 3D Fan

A 3D table fan (base, pole, hub, four spinning blades) built from a
shared cube drawn seven times through a hierarchical `Transform` push/pop
matrix stack. One C++ core (`Scene/`) runs identically on Desktop, Android,
and WebGL.

## Prerequisites

- **Desktop**: CMake ≥ 3.16, Visual Studio 2022 (Desktop development with C++ workload)
- **Web**: [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) (emsdk), Python 3 (for local serving), Git
- **Android**: Android Studio (latest stable) with the NDK and CMake components installed via SDK Manager

GLM (header-only math library) not committed to this repo — every
build target fetches it automatically the first time it's needed (via CMake
`FetchContent` on Desktop, or a `git clone` step in the Web/Android build
scripts and CI workflow). No manual setup required.

## Build & Run — Desktop (Windows)

```bat
script_build_and_run.bat
```

First run downloads GLFW, GLEW, and GLM automatically via CMake
`FetchContent` (allow a few minutes). The fan renders in GLFW window; ESC quits. Output binary:
`build_desktop\Debug\TouchEvents.exe`.

## Build & Run — Web (Emscripten)

```bat
:: from a shell with emsdk activated (run emsdk_env.bat first)
build_web.bat
```

This clones GLM into `external/glm` (first run only) and compiles with
`emcc` directly (GLFW/GLES3 are provided by Emscripten itself — no library
build needed for this target). Output: `web/index.html` (+ `.js` / `.wasm`).

Serve and open in a browser:

```bat
cd web
python -m http.server 8080
:: open http://localhost:8080
```

## Build & Run — Android

1. Open the `android/` folder in Android Studio and let Gradle sync.
2. Run on a device or emulator via the Run ▶ button, **or** from the command line:
   ```bat
   cd android
   gradlew.bat assembleDebug
   ```
3. View gesture/toggle logs with:
   ```
   adb logcat -s Fan3D
   ```

## Controls (all platforms)

| Gesture              | Effect                                                    |
|-----------------------|------------------------------------------------------------|
| Tap / click           | Toggle fan ON/OFF, logs `Fan ON` / `Fan OFF`               |
| Swipe / click-drag    | Spins faster proportional to swipe speed; decays gradually after release (fling) |

## CI/CD

`.github/workflows/build-deploy.yml` builds Desktop, Android, and Web on
every push to `main`, and deploys Web build to GitHub Pages. Desktop and
Android build via their native toolchains (MSVC/CMake and Gradle/NDK); Web
build mirrors `build_web.bat`'s exact `emcc` invocation, fetching GLM the
same way.

## Project layout

```
Assignment-2-3DFan/
├── .github/workflows/ <- CI/CD (build all 3 targets + deploy web to Pages)
├── CMakeLists.txt <- Desktop entry point (GLFW + GLEW + GLM via FetchContent)
├── script_build_and_run.bat <- Desktop convenience wrapper
├── build_web.bat / .sh <- Web entry point (Emscripten)
├── cmake/ImportDependencies.cmake
├── Scene/ <- shared by all three platforms
│   ├── Fan.h / Fan.cpp <- this assignment's implementation
│   ├── Transform.h / .cpp     <- provided, unmodified
│   ├── Renderer.h / .cpp, Model.h, Platform.h, ShaderHelper.h
│   └── main.cpp <- provided, unmodified
├── android/ <- Android Studio / Gradle project
│   └── app/src/main/assets/shader/  <- FanVertex.glsl / FanFragment.glsl (shared shader source)
└── README.md
```

## Notes

- `external/`, `emsdk/`, `build_desktop/`, `build-web/`, `web/`,
  `android/app/build/`, `android/app/.cxx/` are generated/fetched and are
  not committed — see `.gitignore`.
- GitHub Pages requires GitHub Pro on this private repository (provided by
  the GitHub Student Developer Pack); the `deploy-pages` job in CI will not
  succeed until that's active on the account.
