#include "jagfx/jagfx.h"
#include "jautils/arena.h"
#include "jautils/base.h"
#include "jautils/file.h"

#include <GL/glcorearb.h>
#include <cstring>
#include <stdio.h>
#include <math.h>
#include <string.h>

static_assert(sizeof(GLuint) == sizeof(u32));
static_assert(sizeof(GLuint64) == sizeof(u64));

static constexpr u32 MAX_DISPATCH_GROUPS = 65535;

#define JAGFX_GL_FUNCTIONS(X)                                                      \
    X(PFNGLGETSTRINGPROC, glGetString)                                             \
    X(PFNGLGETSTRINGIPROC, glGetStringi)                                           \
    X(PFNGLCLEARCOLORPROC, glClearColor)                                           \
    X(PFNGLCLEARPROC, glClear)                                                     \
    X(PFNGLENABLEPROC, glEnable)                                                   \
    X(PFNGLDISABLEPROC, glDisable)                                                 \
    X(PFNGLDEPTHMASKPROC, glDepthMask)                                             \
    X(PFNGLCULLFACEPROC, glCullFace)                                               \
    X(PFNGLBLENDFUNCPROC, glBlendFunc)                                             \
    X(PFNGLGETERRORPROC, glGetError)                                               \
    X(PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback)                       \
    X(PFNGLCREATETEXTURESPROC, glCreateTextures)                                   \
    X(PFNGLDELETETEXTURESPROC, glDeleteTextures)                                   \
    X(PFNGLTEXTURESTORAGE2DPROC, glTextureStorage2D)                               \
    X(PFNGLGETTEXTUREIMAGEPROC, glGetTextureImage)                                 \
    X(PFNGLGETTEXTURESUBIMAGEPROC, glGetTextureSubImage)                           \
    X(PFNGLCREATEFRAMEBUFFERSPROC, glCreateFramebuffers)                           \
    X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers)                           \
    X(PFNGLNAMEDFRAMEBUFFERTEXTUREPROC, glNamedFramebufferTexture)                 \
    X(PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC, glCheckNamedFramebufferStatus)         \
    X(PFNGLCLEARNAMEDFRAMEBUFFERFVPROC, glClearNamedFramebufferfv)                 \
    X(PFNGLCREATEBUFFERSPROC, glCreateBuffers)                                     \
    X(PFNGLNAMEDBUFFERSTORAGEPROC, glNamedBufferStorage)                           \
    X(PFNGLMAPNAMEDBUFFERRANGEPROC, glMapNamedBufferRange)                         \
    X(PFNGLGETNAMEDBUFFERSUBDATAPROC, glGetNamedBufferSubData)                     \
    X(PFNGLUNMAPNAMEDBUFFERPROC, glUnmapNamedBuffer)                               \
    X(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)                                     \
    X(PFNGLCREATESHADERPROC, glCreateShader)                                       \
    X(PFNGLSHADERSOURCEPROC, glShaderSource)                                       \
    X(PFNGLCOMPILESHADERPROC, glCompileShader)                                     \
    X(PFNGLGETSHADERIVPROC, glGetShaderiv)                                         \
    X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)                               \
    X(PFNGLDELETESHADERPROC, glDeleteShader)                                       \
    X(PFNGLCREATEPROGRAMPROC, glCreateProgram)                                     \
    X(PFNGLATTACHSHADERPROC, glAttachShader)                                       \
    X(PFNGLLINKPROGRAMPROC, glLinkProgram)                                         \
    X(PFNGLGETPROGRAMIVPROC, glGetProgramiv)                                       \
    X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)                             \
    X(PFNGLUSEPROGRAMPROC, glUseProgram)                                           \
    X(PFNGLDELETEPROGRAMPROC, glDeleteProgram)                                     \
    X(PFNGLCREATEVERTEXARRAYSPROC, glCreateVertexArrays)                           \
    X(PFNGLVERTEXARRAYELEMENTBUFFERPROC, glVertexArrayElementBuffer)               \
    X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)                                 \
    X(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)                           \
    X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer)                                 \
    X(PFNGLVIEWPORTPROC, glViewport)                                               \
    X(PFNGLDRAWARRAYSPROC, glDrawArrays)                                           \
    X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced)                         \
    X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced)                     \
    X(PFNGLBINDBUFFERBASEPROC, glBindBufferBase)                                   \
    X(PFNGLPROGRAMUNIFORM1UIPROC, glProgramUniform1ui)                             \
    X(PFNGLFENCESYNCPROC, glFenceSync)                                             \
    X(PFNGLCLIENTWAITSYNCPROC, glClientWaitSync)                                   \
    X(PFNGLDELETESYNCPROC, glDeleteSync)                                           \
    X(PFNGLDISPATCHCOMPUTEPROC, glDispatchCompute)                                 \
    X(PFNGLMEMORYBARRIERPROC, glMemoryBarrier)                                     \
    X(PFNGLTEXTURESUBIMAGE2DPROC, glTextureSubImage2D)                             \
    X(PFNGLGENERATETEXTUREMIPMAPPROC, glGenerateTextureMipmap)                     \
    X(PFNGLTEXTUREPARAMETERIPROC, glTextureParameteri)                             \
    X(PFNGLBINDTEXTUREUNITPROC, glBindTextureUnit)                                 \
    X(PFNGLGETINTEGERVPROC, glGetIntegerv)                                         \
    X(PFNGLGETTEXTUREHANDLEARBPROC, glGetTextureHandleARB)                         \
    X(PFNGLMAKETEXTUREHANDLERESIDENTARBPROC, glMakeTextureHandleResidentARB)       \
    X(PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC, glMakeTextureHandleNonResidentARB) \
    X(PFNGLCREATESAMPLERSPROC, glCreateSamplers)                                   \
    X(PFNGLDELETESAMPLERSPROC, glDeleteSamplers)                                   \
    X(PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri)                             \
    X(PFNGLSAMPLERPARAMETERFPROC, glSamplerParameterf)                             \
    X(PFNGLGETTEXTURESAMPLERHANDLEARBPROC, glGetTextureSamplerHandleARB)

constexpr u32 JAGFX_HEAP_BINDING  = 0;
constexpr u32 JAGFX_ROOT_LOCATION = 0;

constexpr u32 FRAME_SLOT_ALIGNMENT = 256;
constexpr u64 FENCE_TIMEOUT_NS     = 1'000'000'000;

constexpr u32 MAX_INCLUDE_DEPTH = 8;
constexpr u32 MAX_SHADER_FILES  = 16;

struct shader_stage
{
    GLenum      Type;
    const char *Path;
};

struct shader_source
{
    mem_arena  *Out;
    mem_arena  *Files;
    const char *FileNames[MAX_SHADER_FILES];
    u32         FileCount;
};

constexpr u32 MAX_SHADER_STAGES = 2;

#define X(Type, Name) static Type Name;
JAGFX_GL_FUNCTIONS(X)
#undef X

static b32
LoadGL(gl_get_proc GetProc)
{
#define JAGFX_LOAD_OR_FAIL(Type, Name)                        \
    Name = reinterpret_cast<Type>(GetProc(#Name));            \
    if(!(Name))                                               \
    {                                                         \
        fprintf(stderr, "jagfx: failed to load %s\n", #Name); \
        return 0;                                             \
    }
    JAGFX_GL_FUNCTIONS(JAGFX_LOAD_OR_FAIL)
#undef JAGFX_LOAD_OR_FAIL
    return 1;
}

static void
UnloadGL()
{
#define X(Type, Name) Name = nullptr;
    JAGFX_GL_FUNCTIONS(X)
#undef X
}

static void APIENTRY
DebugCallback(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, const GLchar *Message, const void *UserParam)
{
    (void)Type;
    (void)Id;
    (void)Length;
    (void)UserParam;
    if(Severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        return;
    }
    fprintf(stderr, "[GL]: %s\n", Message);
    if(Source == GL_DEBUG_SOURCE_API && Severity == GL_DEBUG_SEVERITY_HIGH)
    {
        JA_DEBUGBREAK();
    }
}

static b32    GInitialized = 0;
static GLuint GEmptyVao    = 0;

static b32
HasExtension(const char *Name)
{
    GLint Count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &Count);
    for(GLint Index = 0; Index < Count; ++Index)
    {
        const GLubyte *Extension = glGetStringi(GL_EXTENSIONS, (GLuint)Index);
        if(strcmp((const char *)Extension, Name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

b32
JagfxInit(gl_get_proc GetProc)
{
    JA_ASSERT(!GInitialized);
    JA_ASSERT(GetProc);

    if(!LoadGL(GetProc))
    {
        fprintf(stderr, "jagfx: LoadGL failed\n");
        UnloadGL();
        return 0;
    }

    if(!HasExtension("GL_ARB_bindless_texture"))
    {
        fprintf(stderr, "jagfx: GL_ARB_bindless_texture not supported by this driver\n");
        UnloadGL();
        return 0;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);

    glCreateVertexArrays(1, &GEmptyVao);
    glBindVertexArray(GEmptyVao);

    GInitialized = 1;
    return 1;
}

void
JagfxShutdown()
{
    if(!GInitialized)
    {
        return;
    }

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &GEmptyVao);
    GEmptyVao = 0;

    glDebugMessageCallback(nullptr, nullptr);
    glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDisable(GL_DEBUG_OUTPUT);

    UnloadGL();
    GInitialized = 0;
}

b32
CreateGpuHeap(size_t Size, gpu_heap *Heap)
{
    *Heap = {};

    GLuint     Buffer = 0;
    GLbitfield Flags  = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    glCreateBuffers(1, &Buffer);
    glNamedBufferStorage(Buffer, static_cast<GLsizeiptr>(Size), nullptr, Flags);
    void *Mapped = glMapNamedBufferRange(Buffer, 0, static_cast<GLsizeiptr>(Size), Flags);
    if(!Mapped)
    {
        fprintf(stderr, "jagfx: mapping the GPU heap failed\n");
        glDeleteBuffers(1, &Buffer);
        return 0;
    }

    JA_ASSERT(reinterpret_cast<umm>(Mapped) % FRAME_SLOT_ALIGNMENT == 0);
    InitArena(Mapped, Size, &Heap->Arena);
    Heap->Buffer = Buffer;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, JAGFX_HEAP_BINDING, Heap->Buffer);
    glVertexArrayElementBuffer(GEmptyVao, Buffer);
    return 1;
}

void
DestroyGpuHeap(gpu_heap *Heap)
{
    if(!Heap->Buffer)
    {
        return;
    }
    glUnmapNamedBuffer(Heap->Buffer);
    glDeleteBuffers(1, &Heap->Buffer);
    *Heap = {};
}

static void
WaitAndDeleteFence(frame_slot *Slot)
{
    GLsync Fence = static_cast<GLsync>(Slot->Fence);
    if(!Fence)
    {
        return;
    }

    GLenum Result = glClientWaitSync(Fence, GL_SYNC_FLUSH_COMMANDS_BIT, FENCE_TIMEOUT_NS);
    switch(Result)
    {
        case GL_CONDITION_SATISFIED:
        case GL_ALREADY_SIGNALED:
            break;
        case GL_TIMEOUT_EXPIRED:
            fprintf(stderr, "glClientWaitSync TIMEOUT\n");
            break;
        case GL_WAIT_FAILED:
        default:
            fprintf(stderr, "fence WAIT_FAILED (0x%X)\n", Result);
            break;
    }
    glDeleteSync(Fence);
    Slot->Fence = nullptr;
}

static b32
CreateFrameSlot(gpu_heap *Heap, size_t Size, frame_slot *Slot)
{
    *Slot    = {};
    u8 *Base = static_cast<u8 *>(PushSize(&Heap->Arena, Size));
    if(!Base)
    {
        JA_ASSERT(!"PushSize failed");
        return 0;
    }
    InitArena(Base, Size, &Slot->Arena);
    return 1;
}

static void
DestroyFrameSlot(frame_slot *Slot)
{
    if(Slot->Fence)
    {
        glDeleteSync(static_cast<GLsync>(Slot->Fence));
    }
    *Slot = {};
}

b32
CreateFrameRing(gpu_heap *Heap, size_t Size, frame_ring *Ring)
{
    *Ring = {};
    for(u32 Index = 0; Index < FRAMES_IN_FLIGHT; ++Index)
    {
        if(!CreateFrameSlot(Heap, Size, &Ring->Slots[Index]))
        {
            fprintf(stderr, "jagfx: CreateFrameSlot failed\n");
            return 0;
        }
    }
    return 1;
}

void
DestroyFrameRing(frame_ring *Ring)
{
    for(u32 Index = 0; Index < FRAMES_IN_FLIGHT; ++Index)
    {
        DestroyFrameSlot(&Ring->Slots[Index]);
    }
    *Ring = {};
}

frame_slot *
BeginFrame(frame_ring *Ring)
{
    JA_ASSERT(!Ring->Current);
    frame_slot *Slot = &Ring->Slots[Ring->FrameIndex % FRAMES_IN_FLIGHT];
    WaitAndDeleteFence(Slot);
    ResetArena(&Slot->Arena);
    Ring->Current = Slot;
    return Slot;
}

void
EndFrame(frame_ring *Ring)
{
    JA_ASSERT(Ring->Current);
    Ring->Current->Fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    Ring->Current        = nullptr;
    ++Ring->FrameIndex;
}

static b32 GRendering = 0;

void
BeginRendering(s32 Width, s32 Height, vec4 ClearColor)
{
    JA_ASSERT(!GRendering && "BeginRendering called twice");
    glViewport(0, 0, Width, Height);
    glClearColor(ClearColor.X, ClearColor.Y, ClearColor.Z, ClearColor.W);
    // glClear obeys the depth write mask: a pipeline with DepthWrite = 0 would otherwise
    // leave next frame's depth buffer uncleared.
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GRendering = 1;
}

void
EndRendering()
{
    JA_ASSERT(GRendering && "EndRendering without BeginRendering");
    GRendering = 0;
}

static void
SetEnabled(GLenum Cap, b32 Enabled)
{
    if(Enabled)
    {
        glEnable(Cap);
    }
    else
    {
        glDisable(Cap);
    }
}

// Sets every piece of raster state the pipeline owns, so the previous draw's state can't leak in.
static void
ApplyPipelineState(const pipeline *Pipeline)
{
    SetEnabled(GL_DEPTH_TEST, Pipeline->DepthTest);
    glDepthMask(Pipeline->DepthWrite ? GL_TRUE : GL_FALSE);

    SetEnabled(GL_CULL_FACE, Pipeline->Cull != CULL_NONE);
    if(Pipeline->Cull != CULL_NONE)
    {
        glCullFace(Pipeline->Cull == CULL_FRONT ? GL_FRONT : GL_BACK);
    }

    switch(Pipeline->Blend)
    {
        case BLEND_NONE:
            glDisable(GL_BLEND);
            break;
        case BLEND_ALPHA:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BLEND_ADDITIVE:
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            break;
        default:
            JA_ASSERT(!"bad blend_mode");
            break;
    }
}

void
Barrier(u32 Flags)
{
    JA_ASSERT(Flags != 0);
    GLbitfield Bits = 0;
    if(Flags & BARRIER_STORAGE)
    {
        Bits |= GL_SHADER_STORAGE_BARRIER_BIT;
    }
    if(Flags & BARRIER_INDIRECT)
    {
        Bits |= GL_COMMAND_BARRIER_BIT;
    }
    JA_ASSERT(Bits != 0);
    glMemoryBarrier(Bits);
}

void
DrawArraysInstancedUntyped(pipeline *Pipeline, u32 RootOffset, u32 VertexCount, u32 InstanceCount)
{
    JA_ASSERT(GRendering && "draw outside BeginRendering/EndRendering");
    ApplyPipelineState(Pipeline);
    glUseProgram(Pipeline->Program);
    glProgramUniform1ui(Pipeline->Program, JAGFX_ROOT_LOCATION, RootOffset);
    glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(VertexCount), static_cast<GLsizei>(InstanceCount));
}

void
DrawElementsInstancedUntyped(pipeline *Pipeline, u32 RootOffset, u32 IndexOffset, u32 IndexCount, u32 InstanceCount)
{
    JA_ASSERT(GRendering && "draw outside BeginRendering/EndRendering");
    ApplyPipelineState(Pipeline);
    glUseProgram(Pipeline->Program);
    glProgramUniform1ui(Pipeline->Program, JAGFX_ROOT_LOCATION, RootOffset);
    glDrawElementsInstanced(GL_TRIANGLES, IndexCount, GL_UNSIGNED_SHORT, (void *)(umm)IndexOffset, InstanceCount);
}

void
DispatchGroupsUntyped(pipeline *Pipeline, u32 RootOffset, u32 GroupsX, u32 GroupsY, u32 GroupsZ)
{
    JA_ASSERT(!GRendering && "dispatch inside BeginRendering/EndRendering");
    JA_ASSERT(GroupsX >= 1 && GroupsX <= MAX_DISPATCH_GROUPS);
    JA_ASSERT(GroupsY >= 1 && GroupsY <= MAX_DISPATCH_GROUPS);
    JA_ASSERT(GroupsZ >= 1 && GroupsZ <= MAX_DISPATCH_GROUPS);
    glUseProgram(Pipeline->Program);
    glProgramUniform1ui(Pipeline->Program, JAGFX_ROOT_LOCATION, RootOffset);
    glDispatchCompute(GroupsX, GroupsY, GroupsZ);
}

static GLuint
CompileShader(GLenum Type, const char *Source, const char *Path)
{
    GLuint Result = glCreateShader(Type);
    glShaderSource(Result, 1, &Source, nullptr);
    glCompileShader(Result);

    GLint Okay = 0;
    glGetShaderiv(Result, GL_COMPILE_STATUS, &Okay);
    if(!Okay)
    {
        char Log[2048];
        glGetShaderInfoLog(Result, sizeof(Log), nullptr, Log);
        fprintf(stderr, "jagfx: %s: compile failed:\n%s\n", Path, Log);
        glDeleteShader(Result);
        return 0;
    }

    return Result;
}

static b32
Emit(shader_source *Source, const char *Text, size_t Size)
{
    char *Dest = (char *)PushSize(Source->Out, Size, 1);
    if(!Dest)
    {
        fprintf(stderr, "jagfx: shader source too large\n");
        return 0;
    }
    memcpy(Dest, Text, Size);
    return 1;
}

static b32
EmitLine(shader_source *Source, u32 Line, u32 FileIndex)
{
    char Buffer[32];
    int  Length = snprintf(Buffer, sizeof(Buffer), "#line %u %u\n", Line, FileIndex);
    return Emit(Source, Buffer, (size_t)Length);
}

static const char *
CopyString(mem_arena *Arena, const char *String)
{
    size_t Size = strlen(String) + 1;
    char  *Copy = (char *)PushSize(Arena, Size, 1);
    if(Copy)
    {
        memcpy(Copy, String, Size);
    }
    return Copy;
}

static b32
ExpandFile(shader_source *Source, const char *Path, u32 Depth)
{
    if(Depth > MAX_INCLUDE_DEPTH)
    {
        fprintf(stderr, "jagfx: %s includes nested deeper than %u (cycle?)\n", Path, MAX_INCLUDE_DEPTH);
        return 0;
    }
    if(Source->FileCount == MAX_SHADER_FILES)
    {
        fprintf(stderr, "jagfx: %s more than %u shader files\n", Path, MAX_SHADER_FILES);
        return 0;
    }

    file File = ReadEntireFile(Path, Source->Files);
    if(!File.Data)
    {
        fprintf(stderr, "jagfx: failed to read %s\n", Path);
        return 0;
    }

    u32 FileIndex                = Source->FileCount++;
    Source->FileNames[FileIndex] = CopyString(Source->Files, Path);

    // #version must be the first line the driver sees
    if(Depth > 0 && !EmitLine(Source, 1, FileIndex))
    {
        return 0;
    }

    const char *At   = (const char *)File.Data;
    u32         Line = 1;
    while(*At)
    {
        const char *End = At;
        while(*End && *End != '\n')
        {
            ++End;
        }
        const char *Next = *End ? End + 1 : End;

        if(strncmp(At, "#include", 8) == 0)
        {
            const char *P = At + 8;
            while(*P == ' ' || *P == '\t')
            {
                ++P;
            }
            if(*P == '<')
            {
                if(!Emit(Source, "\n", 1))
                {
                    return 0;
                }
                At = Next;
                ++Line;
                continue;
            }
            if(*P != '"')
            {
                fprintf(stderr, "jagfx: %s(%u): expected \"file\" or <file> after #include\n", Path, Line);
                return 0;
            }
            ++P;

            const char *NameStart = P;
            while(P < End && *P != '"')
            {
                ++P;
            }
            size_t NameLength = (size_t)(P - NameStart);
            if(P == End || NameLength == 0 || NameLength >= 256)
            {
                fprintf(stderr, "jagfx: %s(%u) malformed #include\n", Path, Line);
                return 0;
            }
            char Name[256];
            memcpy(Name, NameStart, NameLength);
            Name[NameLength] = '\0';

            const char *Slash = nullptr;
            for(const char *C = Path; *C; ++C)
            {
                if(*C == '/' || *C == '\\')
                {
                    Slash = C;
                }
            }
            int FolderLength = Slash ? static_cast<int>(Slash - Path + 1) : 0;

            char Candidate[512];
            int  Length = snprintf(Candidate, sizeof(Candidate), "%.*s%s", FolderLength, Path, Name);
            if(!FileExists(Candidate))
            {
                Length = snprintf(Candidate, sizeof(Candidate), "%s%s", JAGFX_SHADER_INCLUDE_DIR, Name);
            }
            if(Length < 0 || static_cast<size_t>(Length) >= sizeof(Candidate))
            {
                fprintf(stderr, "jagfx: %s(%u): include path too long\n", Path, Line);
                return 0;
            }

            if(!ExpandFile(Source, Candidate, Depth + 1))
            {
                return 0;
            }

            if(!EmitLine(Source, Line + 1, FileIndex))
            {
                return 0;
            }
        }
        else if(!Emit(Source, At, size_t(Next - At)))
        {
            return 0;
        }
        At = Next;
        ++Line;
    }

    return Emit(Source, "\n", 1);
}

static GLuint
LinkProgram(const GLuint *Shaders, u32 Count)
{
    GLuint Result = glCreateProgram();
    for(u32 Index = 0; Index < Count; ++Index)
    {
        glAttachShader(Result, Shaders[Index]);
    }
    glLinkProgram(Result);

    GLint Okay = 0;
    glGetProgramiv(Result, GL_LINK_STATUS, &Okay);
    if(!Okay)
    {
        char Log[2048];
        glGetProgramInfoLog(Result, sizeof(Log), nullptr, Log);
        fprintf(stderr, "jagfx: link failed:\n%s\n", Log);
        glDeleteProgram(Result);
        return 0;
    }

    return Result;
}

template <size_t N>
static b32
LoadProgram(const shader_stage (&Stages)[N], GLuint *Program)
{
    static_assert(N <= MAX_SHADER_STAGES, "raise MAX_SHADER_STAGES");
    *Program = 0;

    mem_temp Scratch                    = GetScratch();
    GLuint   Shaders[MAX_SHADER_STAGES] = {};
    u32      Compiled                   = 0;

    for(; Compiled < ArrayCount(Stages); ++Compiled)
    {
        const shader_stage *Stage = &Stages[Compiled];

        mem_temp      Files  = GetScratch(&Scratch.Arena, 1);
        shader_source Source = {.Out = Scratch.Arena, .Files = Files.Arena};

        const char *Text     = (const char *)Scratch.Arena->Base + Scratch.Arena->Used;
        b32         Expanded = ExpandFile(&Source, Stage->Path, 0) && Emit(&Source, "", 1);
        GLuint      Shader   = Expanded ? CompileShader(Stage->Type, Text, Stage->Path) : 0;
        if(!Expanded && !Shader)
        {
            for(u32 Index = 0; Index < Source.FileCount; ++Index)
            {
                fprintf(stderr, "jagfx:     file %u =%s\n", Index, Source.FileNames[Index]);
            }
        }
        EndTempMemory(Files);
        if(!Shader)
        {
            break;
        }
        Shaders[Compiled] = Shader;
    }

    if(Compiled == ArrayCount(Stages))
    {
        *Program = LinkProgram(Shaders, Compiled);
    }

    for(u32 Index = 0; Index < Compiled; ++Index)
    {
        glDeleteShader(Shaders[Index]);
    }

    EndTempMemory(Scratch);
    return *Program ? 1 : 0;
}

b32
CreateGraphicsPipeline(const graphics_pipeline_desc *Desc, pipeline *Pipeline)
{
    *Pipeline = {};
    JA_ASSERT(Desc && Desc->VertexPath && Desc->FragmentPath);

    shader_stage Stages[] = {
        {.Type = GL_VERTEX_SHADER, .Path = Desc->VertexPath},
        {.Type = GL_FRAGMENT_SHADER, .Path = Desc->FragmentPath},
    };
    if(!LoadProgram(Stages, &Pipeline->Program))
    {
        return 0;
    }

    Pipeline->DepthTest  = Desc->DepthTest;
    Pipeline->DepthWrite = Desc->DepthWrite;
    Pipeline->Cull       = Desc->Cull;
    Pipeline->Blend      = Desc->Blend;
    return 1;
}

b32
CreateComputeProgram(const compute_pipeline_desc *Desc, pipeline *Pipeline)
{
    *Pipeline = {};

    shader_stage Stages[] = {{.Type = GL_COMPUTE_SHADER, .Path = Desc->ComputePath}};
    if(!LoadProgram(Stages, &Pipeline->Program))
    {
        return 0;
    }
    glGetProgramiv(Pipeline->Program, GL_COMPUTE_WORK_GROUP_SIZE, (GLint *)Pipeline->GroupSize);
    return 1;
}

void
DestroyPipeline(pipeline *Pipeline)
{
    if(Pipeline->Program)
    {
        glDeleteProgram(Pipeline->Program);
    }
    *Pipeline = {};
}

static GLenum
ToGL(sampler_filter Filter, b32 Mipmapped)
{
    switch(Filter)
    {
        case SAMPLER_FILTER_LINEAR:
            return Mipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        case SAMPLER_FILTER_NEAREST:
            return Mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    }
    JA_ASSERT(!"bad sampler_filter");
    return GL_LINEAR;
}

static GLenum
ToGL(sampler_wrap Wrap)
{
    switch(Wrap)
    {
        case SAMPLER_WRAP_REPEAT:
            return GL_REPEAT;
        case SAMPLER_WRAP_CLAMP:
            return GL_CLAMP_TO_EDGE;
    }
    return GL_REPEAT;
}

b32
CreateSampler(const sampler_desc *Desc, sampler *Sampler)
{
    *Sampler = {};
    JA_ASSERT(Desc);

    glCreateSamplers(1, &Sampler->Handle);
    if(!Sampler->Handle)
    {
        fprintf(stderr, "jagfx: glCreateSampler: failed\n");
        return 0;
    }
    glSamplerParameteri(Sampler->Handle, GL_TEXTURE_MIN_FILTER, ToGL(Desc->Filter, 1));
    glSamplerParameteri(Sampler->Handle, GL_TEXTURE_MAG_FILTER, ToGL(Desc->Filter, 0));
    glSamplerParameteri(Sampler->Handle, GL_TEXTURE_WRAP_S, ToGL(Desc->Wrap));
    glSamplerParameteri(Sampler->Handle, GL_TEXTURE_WRAP_T, ToGL(Desc->Wrap));
    glSamplerParameteri(Sampler->Handle, GL_TEXTURE_WRAP_R, ToGL(Desc->Wrap));
    if(Desc->MaxAnisotropy > 1)
    {
        glSamplerParameterf(Sampler->Handle, GL_TEXTURE_MAX_ANISOTROPY, Desc->MaxAnisotropy);
    }
    return 1;
}

void
DestroySampler(sampler *Sampler)
{
    if(Sampler->Handle)
    {
        glDeleteSamplers(1, &Sampler->Handle);
    }
    *Sampler = {};
}

b32
LoadTexture2D(const char *Path, s32 Width, s32 Height, const sampler *Sampler, texture *Texture)
{
    *Texture = {};
    JA_ASSERT(Width > 0 && Height > 0);
    mem_temp Scratch = GetScratch();

    size_t Expected = static_cast<size_t>(Width) * static_cast<size_t>(Height) * 4;
    file   Source   = ReadEntireFile(Path, Scratch.Arena);
    if(!Source.Data)
    {
        fprintf(stderr, "jagfx: failed to read %s\n", Path);
        EndTempMemory(Scratch);
        return 0;
    }
    if(Source.Size != Expected)
    {
        fprintf(stderr, "jagfx: %s: size %zu, expected %zu (%d x %d x 4)\n", Path, Source.Size, Expected, Width, Height);
        EndTempMemory(Scratch);
        return 0;
    }

    b32 Result = CreateTexture2D(Width, Height, Source.Data, Sampler, Texture);
    EndTempMemory(Scratch);
    return Result;
}

b32
CreateTexture2D(s32 Width, s32 Height, const void *RGBA8Pixels, const sampler *Sampler, texture *Texture)
{
    *Texture = {};
    JA_ASSERT(Sampler);
    JA_ASSERT(Width > 0 && Height > 0);

    s32 Largest = Width > Height ? Width : Height;
    s32 Levels  = 1;
    while((Largest >> Levels) > 0)
    {
        ++Levels;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &Texture->Handle);
    Texture->Width  = Width;
    Texture->Height = Height;
    glTextureStorage2D(Texture->Handle, Levels, GL_SRGB8_ALPHA8, Width, Height);
    glTextureSubImage2D(Texture->Handle, 0, 0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, RGBA8Pixels);
    glGenerateTextureMipmap(Texture->Handle);
    Texture->Bindless = glGetTextureSamplerHandleARB(Texture->Handle, Sampler->Handle);
    if(!Texture->Bindless)
    {
        fprintf(stderr, "jagfx: glGetTextureHandleARB failed\n");
        DestroyTexture(Texture);
        *Texture = {};
        return 0;
    }
    glMakeTextureHandleResidentARB(Texture->Bindless);
    return 1;
}

void
DestroyTexture(texture *Texture)
{
    if(Texture->Handle)
    {
        if(Texture->Bindless)
        {
            glMakeTextureHandleNonResidentARB(Texture->Bindless);
        }
        glDeleteTextures(1, &Texture->Handle);
    }
    *Texture = {};
}
