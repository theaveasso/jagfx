#include <stdlib.h>
#include <jagfx/jagfx.h>
#include <jautils/base.h>
#include <jawindow/window.h>

#include "cube_shared.h"

static constexpr s32 TEXTURE_WIDTH  = 256;
static constexpr s32 TEXTURE_HEIGHT = 256;

struct app
{
    ja_window  Window;
    gpu_heap   Heap;
    frame_ring Ring;
    sampler    Sampler;
    texture    Texture;
    pipeline   Pipeline;
};

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

static b32 InitApp(app *App);
static void RunApp(app *App);
static void QuitApp(app *App);

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
    if(!OpenWindow(640, 480, "Cube", &App->Window))
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
    if(!CreateFrameRing(&App->Heap, MB(16), &App->Ring))
    {
        return 0;
    }
    sampler_desc SamplerDesc = {.Filter = SAMPLER_FILTER_NEAREST, .Wrap = SAMPLER_WRAP_REPEAT, .MaxAnisotropy = 8.0f};
    if(!CreateSampler(&SamplerDesc, &App->Sampler))
    {
        return 0;
    }
    if(!LoadTexture2D(BASE_DIR "lunarg_logo_256x256.rgba8", TEXTURE_WIDTH, TEXTURE_HEIGHT, &App->Sampler, &App->Texture))
    {
        return 0;
    }
    graphics_pipeline_desc PipelineDesc = {
        .VertexPath   = BASE_DIR "cube.vert.glsl",
        .FragmentPath = BASE_DIR "cube.frag.glsl",
        .DepthTest    = 1,
        .DepthWrite   = 1,
        .Cull         = CULL_BACK,
    };
    if(!CreateGraphicsPipeline(&PipelineDesc, &App->Pipeline))
    {
        return 0;
    }
    return 1;
}

static void
RunApp(app *App)
{
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

    f32      Angle = 0;
    ja_input Input = {};
    while(!WindowShouldClose(&App->Window))
    {
        PumpEvents(&App->Window, &Input);
        if(WasPressed(Input.Keys[JA_KEY_ESCAPE]))
        {
            break;
        }

        int Width  = 0;
        int Height = 0;
        GetWindowFramebufferSize(&App->Window, &Width, &Height);
        if(Width == 0 || Height == 0)
        {
            continue;
        }

        frame_slot *Frame = BeginFrame(&App->Ring);
        (void)Frame;

        gpu_cpu_range<draw_args> Args = PushGpu<draw_args>(&App->Heap, &Frame->Arena, 1);

        Angle += 0.02;
        f32  Aspect     = static_cast<f32>(Width) / static_cast<f32>(Height);
        mat4 Model      = RotationY(Angle);
        mat4 View       = LookAt({0, 3, 5}, {0, 0, 0}, {0, 1, 0});
        mat4 Projection = Perspective(45.0 * 3.14159265 / 180.0, Aspect, 0.1, 100.0);

        *Args.Cpu = {
            .ViewProj    = Projection * View,
            .Model       = Model,
            .VerticesPtr = GpuPtr(&App->Heap, Vertices.Cpu),
            .Texture     = App->Texture.Bindless,
        };

        BeginRendering(Width, Height, {.X = 0.2, .Y = 0.2, .Z = 0.2, .W = 1.0});

        DrawIndexed(&App->Pipeline, Args.Gpu, Indices.Gpu, CUBE_INDEX_COUNT);

        EndRendering();
        EndFrame(&App->Ring);
        PresentWindow(&App->Window);
    }
}

static void
QuitApp(app *App)
{
    DestroyPipeline(&App->Pipeline);
    DestroyTexture(&App->Texture);
    DestroySampler(&App->Sampler);
    DestroyFrameRing(&App->Ring);
    DestroyGpuHeap(&App->Heap);
    JagfxShutdown();
    CloseWindow(&App->Window);
}
