# Third-party dependencies

Pinned dependencies only. Changing a version means changing the URL and hash
here, recording it in this file and in `CHANGELOG.md`, and running the test and
benchmark suites on both platforms.

| Dependency | Version | License | Used by | Pin |
| --- | --- | --- | --- | --- |
| SDL | **3.4.20** | Zlib | `engine/platform` | URL + SHA-256 in `third_party/CMakeLists.txt` |

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

## Offline builds

FetchContent downloads on the first configure. To build offline, populate
`build/<preset>/_deps/sdl3-src` beforehand (or use a local archive URL in a
`CMakeUserPresets.json` override). CI has network access; local agents should
not need it after the first configure.

## Linux build prerequisites

SDL needs development headers for the windowing/audio backends it enables.
On Debian/Ubuntu:

```
sudo apt-get install build-essential cmake ninja-build \
    libx11-dev libxext-dev libxrandr-dev libxi-dev libxcursor-dev \
    libxfixes-dev libxss-dev libasound2-dev libpulse-dev \
    libwayland-dev libxkbcommon-dev
```

Missing optional backends are disabled by SDL rather than failing the build; the
engine still targets X11 and Wayland for real desktops. Exact package
requirements are verified when Linux CI first runs.

## CI note

`ubuntu-24.04` runners do not ship all of the above packages. The Linux CI job
installs the windowing/audio development headers before configuring; if a
package is missing, SDL disables that backend and the build still succeeds.
