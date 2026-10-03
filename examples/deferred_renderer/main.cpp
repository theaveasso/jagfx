#include "jawindow/input.h"
#include <stdlib.h>

#include <jautils/base.h>
#include <jawindow/window.h>
#include <jagfx/jagfx.h>

#include "deferred_shared.h"

constexpr u32 OBJECT_COUNT = 100000;

struct app
{
    ja_window  Window;
    gpu_heap   Heap;
    frame_ring Ring;
    pipeline   SimulatePipeline;
    pipeline   CullPipeline;
    pipeline   DrawPipeline;
};

static b32 InitApp(app *App);
static void RunApp(app *App);
static void QuitApp(app *App);

static constexpr vertex CUBE_VERTICES[] = {
    // -X face
    {.Position = {-1, -1, -1, 1}, .UV = {0, 1}},
    {.Position = {-1, -1, 1, 1}, .UV = {1, 1}},
    {.Position = {-1, 1, 1, 1}, .UV = {1, 0}},
    {.Position = {-1, 1, -1, 1}, .UV = {0, 0}},
    // -Z face
    {.Position = {-1, -1, -1, 1}, .UV = {1, 1}},
    {.Position = {-1, 1, -1, 1}, .UV = {1, 0}},
    {.Position = {1, 1, -1, 1}, .UV = {0, 0}},
    {.Position = {1, -1, -1, 1}, .UV = {0, 1}},
    // -Y face
    {.Position = {-1, -1, -1, 1}, .UV = {1, 0}},
    {.Position = {1, -1, -1, 1}, .UV = {1, 1}},
    {.Position = {1, -1, 1, 1}, .UV = {0, 1}},
    {.Position = {-1, -1, 1, 1}, .UV = {0, 0}},
    // +Y face
    {.Position = {-1, 1, -1, 1}, .UV = {1, 0}},
    {.Position = {-1, 1, 1, 1}, .UV = {0, 0}},
    {.Position = {1, 1, 1, 1}, .UV = {0, 1}},
    {.Position = {1, 1, -1, 1}, .UV = {1, 1}},
    // +X face
    {.Position = {1, 1, -1, 1}, .UV = {1, 0}},
    {.Position = {1, 1, 1, 1}, .UV = {0, 0}},
    {.Position = {1, -1, 1, 1}, .UV = {0, 1}},
    {.Position = {1, -1, -1, 1}, .UV = {1, 1}},
    // +Z face
    {.Position = {-1, -1, 1, 1}, .UV = {0, 1}},
    {.Position = {1, -1, 1, 1}, .UV = {1, 1}},
    {.Position = {1, 1, 1, 1}, .UV = {1, 0}},
    {.Position = {-1, 1, 1, 1}, .UV = {0, 0}},
};
static_assert(ArrayCount(CUBE_VERTICES) == 24);

constexpr u32 CUBE_FACE_COUNT  = 6;
constexpr u32 CUBE_INDEX_COUNT = CUBE_FACE_COUNT * 6;

int
main()
{
    app App  = {};
    b32 Okay = InitApp(&App);
    if(Okay)
    {
        RunApp(&App);
    }
    QuitApp(&App);
    return Okay ? EXIT_SUCCESS : EXIT_FAILURE;
}

static b32
InitApp(app *App)
{
    if(!OpenWindow(1280, 720, "Deferred", &App->Window))
    {
        return 0;
    }
    if(!JagfxInit(GetWindowProcLoader()))
    {
        return 0;
    }
    if(!CreateGpuHeap(MB(64), &App->Heap))
    {
        return 0;
    }
    if(!CreateFrameRing(&App->Heap, MB(10), &App->Ring))
    {
        return 0;
    }
    const compute_pipeline_desc SimDesc = {.ComputePath = BASE_DIR "simulate.comp.glsl"};
    if(!CreateComputeProgram(&SimDesc, &App->SimulatePipeline))
    {
        return 0;
    }
    const compute_pipeline_desc CullDesc = {.ComputePath = BASE_DIR "cull.comp.glsl"};
    if(!CreateComputeProgram(&CullDesc, &App->CullPipeline))
    {
        return 0;
    }
    const graphics_pipeline_desc DrawDesc = {
        .VertexPath   = BASE_DIR "deferred.vert.glsl",
        .FragmentPath = BASE_DIR "deferred.frag.glsl",
        .DepthTest    = 1,
        .DepthWrite   = 1,
        .Cull         = CULL_BACK,
    };
    if(!CreateGraphicsPipeline(&DrawDesc, &App->DrawPipeline))
    {
        return 0;
    }

    return 1;
}

static void
RunApp(app *App)
{
    gpu_cpu_range<mat4> Objects = PushGpu<mat4>(&App->Heap, &App->Heap.Arena, OBJECT_COUNT);
    JA_ASSERT(Objects.Cpu);

    gpu_cpu_range<vertex> Vertices = PushGpu<vertex>(&App->Heap, &App->Heap.Arena, ArrayCount(CUBE_VERTICES));
    JA_ASSERT(Vertices.Cpu);
    for(u32 Index = 0; Index < ArrayCount(CUBE_VERTICES); ++Index)
    {
        Vertices.Cpu[Index] = CUBE_VERTICES[Index];
    }

    constexpr u16      QUAD_PATTERN[] = {0, 1, 2, 2, 3, 0};
    gpu_cpu_range<u16> Indices        = PushGpu<u16>(&App->Heap, &App->Heap.Arena, CUBE_INDEX_COUNT);
    JA_ASSERT(Indices.Cpu);
    for(u32 Face = 0; Face < CUBE_FACE_COUNT; ++Face)
    {
        for(u32 Corner = 0; Corner < 6; ++Corner)
        {
            Indices.Cpu[Face * 6 + Corner] = static_cast<u16>(Face * 4 + QUAD_PATTERN[Corner]);
        }
    }

    gpu_cpu_range<u32> VisibleIds = PushGpu<u32>(&App->Heap, &App->Heap.Arena, OBJECT_COUNT);

    ja_input Input     = {};
    f64      StartTime = GetTimeSeconds();
    while(!WindowShouldClose(&App->Window))
    {
        PumpEvents(&App->Window, &Input);
        if(WasPressed(Input.Keys[JA_KEY_ESCAPE]))
        {
            break;
        }
        int Width, Height = 0;
        GetWindowFramebufferSize(&App->Window, &Width, &Height);
        if(Width == 0 || Height == 0)
        {
            continue;
        }

        frame_slot *Frame = BeginFrame(&App->Ring);

        gpu_cpu_range<sim_args> SimArgs = PushGpu<sim_args>(&App->Heap, &Frame->Arena, 1);
        *SimArgs.Cpu                    = {
            .Count      = OBJECT_COUNT,
            .Time       = (f32)(GetTimeSeconds() - StartTime),
            .ObjectsPtr = Objects.Gpu,
        };

        gpu_cpu_range<draw_indexed_command> Command = PushGpu<draw_indexed_command>(&App->Heap, &Frame->Arena, 1);
        *Command.Cpu                                = {
            .IndexCount    = CUBE_INDEX_COUNT,
            .InstanceCount = 0,
            .FirstIndex    = u32(Indices.Gpu.Offset / sizeof(u16)),
        };

        gpu_cpu_range<cull_args> CullArgs = PushGpu<cull_args>(&App->Heap, &Frame->Arena, 1);
        *CullArgs.Cpu                     = {
            .Count         = OBJECT_COUNT,
            .ObjectsPtr    = Objects.Gpu,
            .VisibleIdsPtr = VisibleIds.Gpu,
            .CommandPtr    = Command.Gpu,
        };

        f32  Aspect     = (f32)Width / (f32)Height;
        mat4 View       = LookAt({0, 25, 40}, {0, 0, 0}, {0, 1, 0});
        mat4 Projection = Perspective(45.0 * 3.14159265 / 180.0, Aspect, 0.1, 100.0);

        gpu_cpu_range<draw_args> DrawArgs = PushGpu<draw_args>(&App->Heap, &Frame->Arena, 1);
        *DrawArgs.Cpu                     = {
            .ViewProj      = Projection * View,
            .VerticesPtr   = Vertices.Gpu,
            .ObjectsPtr    = Objects.Gpu,
            .VisibleIdsPtr = VisibleIds.Gpu,
        };

        Dispatch(&App->SimulatePipeline, SimArgs.Gpu, OBJECT_COUNT);
        Barrier(barrier_flags::BARRIER_STORAGE);
        Dispatch(&App->CullPipeline, CullArgs.Gpu, OBJECT_COUNT);
        Barrier(barrier_flags::BARRIER_STORAGE | barrier_flags::BARRIER_INDIRECT);

        BeginRendering(Width, Height, {0.0, 1.0, 0.0, 1.0});
        DrawIndexedIndirect(&App->DrawPipeline, DrawArgs.Gpu, Command.Gpu);
        EndRendering();
        EndFrame(&App->Ring);
        PresentWindow(&App->Window);
    }
}

static void
QuitApp(app *App)
{
    DestroyPipeline(&App->DrawPipeline);
    DestroyPipeline(&App->CullPipeline);
    DestroyPipeline(&App->SimulatePipeline);
    DestroyGpuHeap(&App->Heap);
    JagfxShutdown();
    CloseWindow(&App->Window);
}
