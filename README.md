# jagfx

An OpenGL 4.6 + `ARB_bindless_texture` port of the ideas in Sebastian Aaltonen's (sebbbi)
[**NoGraphicsAPI**](https://github.com/sebbbi/NoGraphicsAPI): a graphics API built around
GPU memory and pointers instead of bind slots. Not core GL alone: bindless textures are an
extension (see [Trade-offs](#trade-offs)).

<!-- TODO: drop a GIF of the spinning cube here, e.g. ![cube](docs/cube.gif) -->

```cpp
// The whole per-frame CPU side of examples/cube: write one struct into the GPU heap, draw.
gpu_cpu_range<draw_args> Args = PushGpu<draw_args>(&App->Heap, &Frame->Arena, 1);
*Args.Cpu = {
    .ViewProj    = Projection * View,
    .Model       = Model,
    .VerticesPtr = GpuPtr(&App->Heap, Vertices.Cpu),
    .Texture     = App->Texture.Bindless,
};
DrawIndexed(&App->Pipeline, Args.Gpu, Indices.Gpu, CUBE_INDEX_COUNT);
```

No VAO layouts, no vertex/uniform buffer binds, no texture units. The draw passes **one `uint`**,
and the shaders reach everything else through it.

The article's root pointer relies on Vulkan/Metal features, such as real 64-bit GPU pointers,
that GL doesn't have, so on GL it's an ordinary uniform update. The reason to keep the shape
anyway is GPU-driven drawing: with
multi-draw indirect, uniforms can't change between draws, so every draw has to find its data
through memory (planned: `gl_DrawID` → root table → struct). For plain CPU-driven draws, regular
uniforms can be faster since they skip one memory read.

## The model

- **One GPU heap.** A single persistent, coherent-mapped SSBO bound once at `binding = 0`.
  Allocating is an arena bump; uploading is a `memcpy`.
- **Pointers are byte offsets.** `gpu_ptr<T>` is a typed `u32` offset into the heap (GL's stand-in
  for a 64-bit GPU pointer). Structs can hold pointers to other heap data.
- **One root per draw.** Every draw/dispatch takes exactly one root pointer, passed as
  `uniform uint Root`. Vertex and fragment shaders both follow it.
- **Vertex pulling.** Vertex shaders fetch their own vertices by `gl_VertexID`; there are no
  vertex attributes.
- **Bindless textures.** `ARB_bindless_texture` handles (texture + sampler pair) live in the heap
  as plain 64-bit fields.
- **Shared layouts.** Each struct is declared once in a `*_shared.h` that compiles as both C++ and
  GLSL. Offsets are `#define`d once and `static_assert`ed against `offsetof` on the C++ side.
- **Pipelines own their state.** Depth, cull and blend state are baked in at creation and applied
  by every draw, Vulkan-style, so GL state can't leak between draws.
- **Per-frame ring.** Per-frame data lives in fenced ring slots, recycled once the GPU is done.

The shader side of the cube:

```glsl
#version 460 core
#include "cube_shared.h"   // expanded by jagfx's loader; shares structs + offsets with C++

out vec2 UV;
out vec3 WorldPos;

void main()
{
    draw_args Args   = LoadDrawArgs(Root);
    vertex    Vertex = LoadVertex(Args.VerticesPtr + gl_VertexID * VERTEX_SIZE);

    gl_Position = Args.ViewProj * Args.Model * Vertex.Position;
    UV          = Vertex.UV;
    WorldPos    = (Args.Model * Vertex.Position).xyz;
}
```

## Trade-offs

| Cost | Why | Mitigation |
|---|---|---|
| No GPU-side type safety | A wrong offset reads garbage; no validation layer knows a `u32` is a pointer | Shared headers + `static_assert`; zeroed handles render black instead of crashing |
| Hand-written loaders | Each struct needs a `LoadX(BytePtr)` in GLSL | Small; generated later if it hurts |
| Heap is CPU-visible memory | Persistent mapping may put large static data behind PCIe | Planned: GPU-only heap + copy uploads |
| Extension dependency | Needs `GL_ARB_bindless_texture` (NVIDIA, AMD; Intel spotty; no macOS) | Checked at startup with a clear error |
| 4 GB heap | Pointers are `u32` offsets | Fine for this scope |

What GL can't express from NoGraphicsAPI, and so isn't ported: command buffers and multiple
queues (GL has one implicit queue), placed resources / texture heaps (the driver owns texture
memory), and separate texture/sampler descriptors in shaders (GL bindless handles are always a
texture+sampler pair).

## Status

| Milestone | |
|---|---|
| **M0** Triangle: GL loader, GPU heap, frame ring, root-pointer draw, vertex pulling | done |
| **M1** Shared C++/GLSL layouts, `static_assert`ed offsets, shader `#include` expander | done |
| **M2** Cube: indexed draws, bindless textures, sampler objects, `dFdx`/`dFdy` lighting, pipeline state | done |
| **M3** Deferred renderer: indirect draws, GPU-chosen roots, G-buffer, compute simulation | next |

## Building

Requirements: Windows, Visual Studio (MSVC) with C++20, CMake 3.28+, Ninja, and a GPU/driver with
OpenGL 4.6 and `GL_ARB_bindless_texture`.

```bat
cmake --preset msvc-debug
cmake --build --preset msvc-debug
build\msvc-debug\bin\jagfx_cube.exe
```

Run these from a Developer Command Prompt, or use `build-msvc-debug.bat`, which sets up the
MSVC environment first. GLFW is vendored under `vendor/`.

## Layout

```
include/jagfx/        public API (jagfx.h) and the C++/GLSL shared header (jagfx_shared.h)
src/jagfx.cpp         the whole GL backend; GL headers stay private to this file
utility/              jautils: base types, arenas, file IO, math
window/               jawindow: GLFW window + input
examples/triangle     M0
examples/cube         M1 + M2
```

Code style is Handmade Hero-like: plain structs and free functions, arenas instead of the STL,
no exceptions or RTTI.

## How this was made

This is a learning project. I built it with [Claude Code](https://claude.com/claude-code) as a
tutor and reviewer: it explained concepts and reviewed my code. Parts of the code and most of
the documentation (including this README) were written with it. Reviews and corrections are very
welcome.

## Credits

- [NoGraphicsAPI](https://github.com/sebbbi/NoGraphicsAPI) by Sebastian Aaltonen: the design this
  project ports and learns from.
- [GLFW](https://www.glfw.org/) for windowing.
- The Khronos Group for the OpenGL headers.

## License

jagfx is released under the zlib license. See [LICENSE](LICENSE).
Credit is appreciated but not required.

Third-party code under `vendor/` keeps its own license:

- `vendor/glfw`: GLFW, zlib license (see `vendor/glfw/LICENSE.md`)
- `vendor/khronos`: Khronos OpenGL headers, MIT license (see the notice at the top of each file)
