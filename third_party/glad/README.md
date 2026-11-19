# glad — OpenGL 3.3 core loader

Generated code, vendored so that building the engine needs no Python and no
network access. Do not edit these files by hand; regenerate them instead.

| Field | Value |
| --- | --- |
| Generator | glad 2.0.8 (`v2.0.8`) |
| Source | <https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.tar.gz> |
| Archive SHA-256 | `44f06f9195427c7017f5028d0894f57eb216b0a8f7c4eda7ce883732aeb2d0fc` |
| API | `gl:core=3.3`, no extensions |
| Languages | C |
| License | MIT for glad itself; the Khronos headers keep their own notice (see `LICENSE`) |

## Files

```
include/glad/gl.h        loader declarations and GL 3.3 core entry points
include/KHR/khrplatform.h  Khronos platform types used by glad
src/gl.c                 the loader implementation
```

## How this was generated

```bash
# From an extracted glad-2.0.8 tarball, with Python 3 and Jinja2 available:
python -m glad --out-path third_party/glad \
    --api gl:core=3.3 --extensions "" --reproducible c
```

`--extensions ""` keeps the output to core 3.3 only, which is why the files are
small; adding extensions later means regenerating, not patching. `--reproducible`
uses the OpenGL registry snapshot shipped in the tarball instead of downloading
the latest specification, so the output depends only on the pinned version.

## Usage

`eng::gl_loader` is a static library with `glad/gl.h` on the include path. It is
not linked by anything yet: the renderer arrives in M2 (ADR-0003). When it does,
it calls `gladLoadGL(SDL_GL_GetProcAddress)` once after the context is current,
then uses the generated functions directly.

## Updating

Replace the whole directory by running the command above with a new tag, update
the version and hash here and in `third_party/README.md`, and build on both
platforms. Never hand-edit the generated files.
