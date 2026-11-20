# Third-party dependencies

Pinned dependencies only. Changing a version means changing the URL and hash
here, recording it in this file and in `CHANGELOG.md`, and running the test and
benchmark suites on both platforms.

| Dependency | Version | License | Used by | Pin |
| --- | --- | --- | --- | --- |
| SDL | **3.4.20** | Zlib | `engine/platform` | URL + SHA-256 in `third_party/CMakeLists.txt` |
| glad (OpenGL loader) | **2.0.8** | MIT (Khronos headers keep their notice) | M2 renderer, via `peri::gl_loader` | Generated output vendored in `third_party/glad/`; generator archive pinned by SHA-256 in its README |
| Manrope variable font | **4.505** | SIL OFL 1.1 | Brand assets only; not linked into engine | Google Fonts commit `b31870aff700ab7a1d74fa0c6887d95beb9e0037`, SHA-256 `3ae11c49db0455a3cc33e37d380f20fdb8c7f8b41dc07625c177e3d87a9d6ae6` |

## Manrope 4.505

- Source: [Google Fonts, pinned commit](https://github.com/google/fonts/tree/b31870aff700ab7a1d74fa0c6887d95beb9e0037/ofl/manrope).
- Vendored font and license: `brand/fonts/Manrope-Variable.ttf` and
  `brand/fonts/OFL.txt`. Used for brand work, not engine builds. Upstream is
  maintained through Google Fonts; the pin updates only in a reviewed change.
- Why: modern, commercially usable typography under the SIL OFL 1.1. See
  ADR-0011 and `brand/README.md` for license and logo-use details.

## SDL 3.4.20

- Source: <https://github.com/libsdl-org/SDL/releases/tag/release-3.4.20>
- Archive: `SDL3-3.4.20.tar.gz`
- SHA-256: `f3bb06f55bed9557227a700af2ea62e49497ebdf58037732a731a5fb3ef8581f`
- Why: windowing, input/gamepads, timers, audio devices and threads on Windows
  and Linux, without making SDL part of the engine's API. See ADR-0002.
- Build: static, tests/examples disabled, fetched by CMake's FetchContent on the
  first configure into `build/<preset>/_deps/`.
- Update policy: check upstream release notes, bump the pin, run the full test
  suite and `bench_blank` on both platforms, and note behavioral changes.

## glad 2.0.8

- Generator source: <https://github.com/Dav1dde/glad/releases/tag/v2.0.8>
- Archive: `glad-2.0.8.tar.gz`, SHA-256
  `44f06f9195427c7017f5028d0894f57eb216b0a8f7c4eda7ce883732aeb2d0fc`
- Output: GL 3.3 core, no extensions, C.
- Why vendored: glad generates with Python and Jinja2. Requiring a specific
  Python for every build is fragile; the output never changes between
  dependency updates, so it is generated once and committed. See ADR-0010.
- Details, exact command and regeneration steps: `third_party/glad/README.md`.
- Nothing links the loader yet; the renderer arrives in M2.

## Offline builds

FetchContent downloads on the first configure. To build offline, populate
`build/<preset>/_deps/sdl3-src` beforehand (or use a local archive URL in a
`CMakeUserPresets.json` override). CI has network access; local agents should
not need it after the first configure.

## Linux build prerequisites

SDL needs development headers for the windowing/audio backends it enables.
On Debian/Ubuntu:

```
sudo apt-get install build-essential cmake ninja-build pkg-config \
    libx11-dev libxext-dev libxrandr-dev libxi-dev libxcursor-dev \
    libxfixes-dev libxss-dev libxtst-dev libasound2-dev libpulse-dev \
    libwayland-dev libxkbcommon-dev libxkbcommon-x11-dev libdecor-0-dev \
    libdrm-dev libgbm-dev libudev-dev
```

SDL 3.4.20 treats XTest as required when X11 is enabled, so `libxtst-dev` is
not optional: without it the configure step fails with
`Couldn't find dependency package for XTEST`.

Most optional backends are disabled by SDL when their headers are absent, but
some X11 dependencies are hard requirements; see the XTest note above. The
engine targets X11 and Wayland for real desktops.

## CI note

`ubuntu-24.04` runners do not ship all of the above packages. The Linux CI job
installs the windowing/audio development headers before configuring, and
`tools/dev/ci_configure.sh` prints configure errors as annotations so failures
are readable without admin access to the job logs.
