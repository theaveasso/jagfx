#include <cstddef>
#include <cstdlib>

#include <jagfx/jagfx.h>
#include <jautils/math.h>
#include <jawindow/window.h>

constexpr int         WIDTH  = 640;
constexpr int         HEIGHT = 480;
constexpr const char *TITLE  = "Triangle";

struct draw_args
{
    vec2          CameraPosition;
    vec2          CameraHalfSize;
    gpu_ptr<vec2> Vertices;
};
static_assert(offsetof(draw_args, CameraPosition) == 0);
static_assert(offsetof(draw_args, CameraHalfSize) == 8);
static_assert(offsetof(draw_args, Vertices) == 16);

struct app
{
    ja_window           Window;
    pipeline            Pipeline;
    gpu_heap            Heap;
    frame_ring          Ring;
    gpu_cpu_range<vec2> Vertices;
};

static b32
InitApp(app *App)
{
    if(!OpenWindow(WIDTH, HEIGHT, TITLE, &App->Window))
    {
        return 0;
    }
    if(!JagfxInit(GetWindowProcLoader()))
    {
        return 0;
    }
    graphics_pipeline_desc PipelineDesc = {
        .VertexPath   = SHADER_DIR "triangle.vert.glsl",
        .FragmentPath = SHADER_DIR "triangle.frag.glsl",
    };
    if(!CreateGraphicsPipeline(&PipelineDesc, &App->Pipeline))
    {
        return 0;
    }
    if(!CreateGpuHeap(MB(64), &App->Heap))
    {
        return 0;
    }
    if(!CreateFrameRing(&App->Heap, MB(1), &App->Ring))
    {
        return 0;
    }

    App->Vertices = PushGpu<vec2>(&App->Heap, &App->Heap.Arena, 3);
    if(!App->Vertices.Cpu)
    {
        return 0;
    }
    App->Vertices.Cpu[0] = {0.0, 0.5};
    App->Vertices.Cpu[1] = {-0.5, -0.5};
    App->Vertices.Cpu[2] = {0.5, -0.5};
    return 1;
}

static void
RunApp(app *App)
{
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

        frame_slot              *Frame = BeginFrame(&App->Ring);
        gpu_cpu_range<draw_args> Args  = PushGpu<draw_args>(&App->Heap, &Frame->Arena);
        JA_ASSERT(Args.Cpu);

        f32 Aspect = static_cast<f32>(Width) / static_cast<f32>(Height);
        *Args.Cpu  = {.CameraPosition = {0.0, 0.0}, .CameraHalfSize = {Aspect, 1.0}, .Vertices = App->Vertices.Gpu};

        BeginRendering(Width, Height, {0.05, 0.05, 0.08, 1.0});
        DrawArraysInstanced(&App->Pipeline, Args.Gpu, 3);
        EndRendering();

        EndFrame(&App->Ring);
        PresentWindow(&App->Window);
    }
}

static void
ShutdownApp(app *App)
{
    DestroyFrameRing(&App->Ring);
    DestroyGpuHeap(&App->Heap);
    DestroyPipeline(&App->Pipeline);
    JagfxShutdown();
    CloseWindow(&App->Window);
}

int
main()
{
    app App  = {};
    b32 Okay = InitApp(&App);
    if(Okay)
    {
        RunApp(&App);
    }
    ShutdownApp(&App);
    return Okay ? EXIT_SUCCESS : EXIT_FAILURE;
}
