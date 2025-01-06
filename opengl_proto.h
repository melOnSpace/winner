#ifndef _OPENGL_PROTOTYPES_H
#define _OPENGL_PROTOTYPES_H

typedef union {
    struct { float x, y; };
    float raw[2];
} Vec2_t;
typedef union {
    struct { float x, y, z; };
    Vec2_t v2;
    float raw[3];
} Vec3_t;
typedef union {
    struct { float x, y, z, w; };
    Vec3_t v3;
    Vec2_t v2;
    float raw[4];
} Vec4_t;

static BOOL load_all_opengl_procs(void);
extern inline void* get_gl_address(HMODULE module, const char* name, BOOL* success);

// See: https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
#define WGL_NUMBER_PIXEL_FORMATS_ARB    0x2000
#define WGL_DRAW_TO_WINDOW_ARB          0x2001
#define WGL_DRAW_TO_BITMAP_ARB          0x2002
#define WGL_ACCELERATION_ARB            0x2003
#define WGL_NEED_PALETTE_ARB            0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB     0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB      0x2006
#define WGL_SWAP_METHOD_ARB             0x2007
#define WGL_NUMBER_OVERLAYS_ARB         0x2008
#define WGL_NUMBER_UNDERLAYS_ARB        0x2009
#define WGL_TRANSPARENT_ARB             0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB   0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB  0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB             0x200C
#define WGL_SHARE_STENCIL_ARB           0x200D
#define WGL_SHARE_ACCUM_ARB             0x200E
#define WGL_SUPPORT_GDI_ARB             0x200F
#define WGL_SUPPORT_OPENGL_ARB          0x2010
#define WGL_DOUBLE_BUFFER_ARB           0x2011
#define WGL_STEREO_ARB                  0x2012
#define WGL_PIXEL_TYPE_ARB              0x2013
#define WGL_COLOR_BITS_ARB              0x2014
#define WGL_RED_BITS_ARB                0x2015
#define WGL_RED_SHIFT_ARB               0x2016
#define WGL_GREEN_BITS_ARB              0x2017
#define WGL_GREEN_SHIFT_ARB             0x2018
#define WGL_BLUE_BITS_ARB               0x2019
#define WGL_BLUE_SHIFT_ARB              0x201A
#define WGL_ALPHA_BITS_ARB              0x201B
#define WGL_ALPHA_SHIFT_ARB             0x201C
#define WGL_ACCUM_BITS_ARB              0x201D
#define WGL_ACCUM_RED_BITS_ARB          0x201E
#define WGL_ACCUM_GREEN_BITS_ARB        0x201F
#define WGL_ACCUM_BLUE_BITS_ARB         0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB        0x2021
#define WGL_DEPTH_BITS_ARB              0x2022
#define WGL_STENCIL_BITS_ARB            0x2023
#define WGL_AUX_BUFFERS_ARB             0x2024

#define WGL_NO_ACCELERATION_ARB         0x2025
#define WGL_GENERIC_ACCELERATION_ARB    0x2026
#define WGL_FULL_ACCELERATION_ARB       0x2027
#define WGL_SWAP_EXCHANGE_ARB           0x2028
#define WGL_SWAP_COPY_ARB               0x2029
#define WGL_SWAP_UNDEFINED_ARB          0x202A
#define WGL_TYPE_RGBA_ARB               0x202B
#define WGL_TYPE_COLORINDEX_ARB         0x202C

// See: https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

// Values for WGL_CONTEXT_FLAGS
#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002

// Values for WGL_CONTEXT_PROFILE_MASK_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x0001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x0002

typedef HGLRC (WINAPI* ProtoCreateContextAttribsABR)(HDC hDc, HGLRC hGlContext, const int* lpAttribs);
static ProtoCreateContextAttribsABR wglCreateContextAttribsABR = NULL;
#define LIBSTRWGLCREATECONTEXTATTRIBSABR "wglCreateContextAttribsARB"

typedef HRESULT (APIENTRY* ProtoChoosePixelFormatABR)(HDC hDc, const int* lpAttribsI, const float* lpAttribsF, UINT nMaxFormats, int* lpFormats, UINT* nNumFormats);
static ProtoChoosePixelFormatABR wglChoosePixelFormatABR = NULL;
#define LIBSTRWGLCHOOSEPIXELFORMATABR "wglChoosePixelFormatARB"

typedef BOOL (APIENTRY* ProtoSwapIntervalEXT)(int Interval);
static ProtoSwapIntervalEXT wglSwapIntervalEXT = NULL;
#define LIBSTRWGLSWAPINTERVALEXT "wglSwapIntervalEXT"

typedef void (APIENTRY* ProtoGenBuffers)(GLsizei Num, GLuint* Buffers);
static ProtoGenBuffers glGenBuffers = NULL;
#define LIBSTRGLGENBUFFERS "glGenBuffers"

typedef void (APIENTRY* ProtoBindBuffer)(GLenum Target, GLuint const Buffer);
static ProtoBindBuffer glBindBuffer = NULL;
#define LIBSTRGLBINDBUFFER "glBindBuffer"

typedef void (APIENTRY* ProtoDeleteBuffers)(GLsizei Num, GLuint* Buffers);
static ProtoDeleteBuffers glDeleteBuffers = NULL;
#define LIBSTRGLDELETEBUFFERS "glDeleteBuffers"

typedef void (APIENTRY* ProtoBufferData)(GLenum Target, GLsizeiptr Size, const void* Data, GLenum Usage);
static ProtoBufferData glBufferData = NULL;
#define LIBSTRGLBUFFERDATA "glBufferData"

typedef void (APIENTRY* ProtoBindVertexArray)(GLuint Array);
static ProtoBindVertexArray glBindVertexArray = NULL;
#define LIBSTRGLBINDVERTEXARRAY "glBindVertexArray"

typedef void (APIENTRY* ProtoGenVertexArrays)(GLsizei Num, GLuint* Arrays);
static ProtoGenVertexArrays glGenVertexArrays = NULL;
#define LIBSTRGLGENVERTEXARRAYS "glGenVertexArrays"

typedef void (APIENTRY* ProtoVertexAttribPointer)(GLuint Index, GLint Size, GLenum Type, GLboolean Normalized, GLsizei Stride, const void* Pointer);
static ProtoVertexAttribPointer glVertexAttribPointer = NULL;
#define LIBSTRGLVERTEXATTRIBPOINTER "glVertexAttribPointer"

typedef void (APIENTRY* ProtoEnableVertexAttribArray)(GLuint Index);
static ProtoEnableVertexAttribArray glEnableVertexAttribArray = NULL;
#define LIBSTRGLENABLEVERTEXATTRIBARRAY "glEnableVertexAttribArray"

typedef void (APIENTRY* ProtoDeleteVertexArrays)(GLsizei Num, GLuint* Arrays);
static ProtoDeleteVertexArrays glDeleteVertexArrays = NULL;
#define LIBSTRGLDELETEVERTEXARRAYS "glDeleteVertexArrays"

typedef GLuint (APIENTRY* ProtoCreateShader)(GLenum ShaderType);
static ProtoCreateShader glCreateShader = NULL;
#define LIBSTRGLCREATESHADER "glCreateShader"

typedef void (APIENTRY* ProtoShaderSource)(GLuint Shader, GLsizei Count, const GLchar** String, const GLint* Length);
static ProtoShaderSource glShaderSource = NULL;
#define LIBSTRGLSHADERSOURCE "glShaderSource"

typedef void (APIENTRY* ProtoCompileShader)(GLuint Shader);
static ProtoCompileShader glCompileShader = NULL;
#define LIBSTRGLCOMPILESHADER "glCompileShader"

typedef void (APIENTRY* ProtoGetShaderiv)(GLuint Shader, GLenum Pname, GLint* Params);
static ProtoGetShaderiv glGetShaderiv = NULL;
#define LIBSTRGLGETSHADERIV "glGetShaderiv"

typedef void (APIENTRY* ProtoGetShaderInfoLog)(GLuint Shader, GLsizei MaxLength, GLsizei* Length, GLchar* InfoLog);
static ProtoGetShaderInfoLog glGetShaderInfoLog = NULL;
#define LIBSTRGLGETSHADERINFOLOG "glGetShaderInfoLog"

typedef GLuint (APIENTRY* ProtoCreateProgram)(void);
static ProtoCreateProgram glCreateProgram = NULL;
#define LIBSTRGLCREATEPROGRAM "glCreateProgram"

typedef void (APIENTRY* ProtoAttachShader)(GLuint Program, GLuint Shader);
static ProtoAttachShader glAttachShader = NULL;
#define LIBSTRGLATTACHSHADER "glAttachShader"

typedef void (APIENTRY* ProtoLinkProgram)(GLuint Program);
static ProtoLinkProgram glLinkProgram = NULL;
#define LIBSTRGLLINKPROGRAM "glLinkProgram"

typedef void (APIENTRY* ProtoDeleteShader)(GLuint Shader);
static ProtoDeleteShader glDeleteShader = NULL;
#define LIBSTRGLDELETESHADER "glDeleteShader"

typedef void (APIENTRY* ProtoDeleteProgram)(GLuint Program);
static ProtoDeleteProgram glDeleteProgram = NULL;
#define LIBSTRGLDELETEPROGRAM "glDeleteProgram"

typedef void (APIENTRY* ProtoGetProgramiv)(GLuint Program, GLenum Pname, GLint *Params);
static ProtoGetProgramiv glGetProgramiv = NULL;
#define LIBSTRGLGETPROGRAMIV "glGetProgramiv"

typedef void (APIENTRY* ProtoGetProgramInfoLog)(GLuint Program, GLsizei MaxLength, GLsizei* Length,  GLchar* InfoLog);
static ProtoGetProgramInfoLog glGetProgramInfoLog = NULL;
#define LIBSTRGLGETPROGRAMINFOLOG "glGetProgramInfoLog"

typedef void (APIENTRY* ProtoUseProgram)(GLuint Program);
static ProtoUseProgram glUseProgram = NULL;
#define LIBSTRGLUSEPROGRAM "glUseProgram"

typedef GLint (APIENTRY* ProtoGetUniformLocation)(GLuint Program, const GLchar* Name);
static ProtoGetUniformLocation glGetUniformLocation = NULL;
#define LIBSTRGLGETUNIFORMLOCATION "glGetUniformLocation"

typedef void (APIENTRY* ProtoUniform3fv)(GLint Location, GLsizei Count, const Vec3_t* Value);
static ProtoUniform3fv glUniform3fv = NULL;
#define LIBSTRGLUNIFORM3FV "glUniform3fv"

typedef void (APIENTRY* ProtoUniform1i)(GLint Location, GLint Value);
static ProtoUniform1i glUniform1i = NULL;
#define LIBSTRGLUNIFORM1I "glUniform1i"

typedef void (APIENTRY* ProtoUniform1f)(GLint Location, GLfloat Value);
static ProtoUniform1f glUniform1f = NULL;
#define LIBSTRGLUNIFORM1F "glUniform1f"

static BOOL load_all_opengl_procs(void) {
    BOOL result = TRUE;
    HMODULE opengl_module = LoadLibraryA("opengl32.dll");
    assert(opengl_module != NULL);

    wglCreateContextAttribsABR = get_gl_address(opengl_module, LIBSTRWGLCREATECONTEXTATTRIBSABR, &result); if (!result) goto defer;
    wglChoosePixelFormatABR = get_gl_address(opengl_module, LIBSTRWGLCHOOSEPIXELFORMATABR, &result); if (!result) goto defer;
    wglSwapIntervalEXT = get_gl_address(opengl_module, LIBSTRWGLSWAPINTERVALEXT, &result); if (!result) goto defer;
    glGenBuffers = get_gl_address(opengl_module, LIBSTRGLGENBUFFERS, &result); if (!result) goto defer;
    glBindBuffer = get_gl_address(opengl_module, LIBSTRGLBINDBUFFER, &result); if (!result) goto defer;
    glDeleteBuffers = get_gl_address(opengl_module, LIBSTRGLDELETEBUFFERS, &result); if (!result) goto defer;
    glBufferData = get_gl_address(opengl_module, LIBSTRGLBUFFERDATA, &result); if (!result) goto defer;
    glBindVertexArray = get_gl_address(opengl_module, LIBSTRGLBINDVERTEXARRAY, &result); if (!result) goto defer;
    glGenVertexArrays = get_gl_address(opengl_module, LIBSTRGLGENVERTEXARRAYS, &result); if (!result) goto defer;
    glVertexAttribPointer = get_gl_address(opengl_module, LIBSTRGLVERTEXATTRIBPOINTER, &result); if (!result) goto defer;
    glEnableVertexAttribArray = get_gl_address(opengl_module, LIBSTRGLENABLEVERTEXATTRIBARRAY, &result); if (!result) goto defer;
    glDeleteVertexArrays = get_gl_address(opengl_module, LIBSTRGLDELETEVERTEXARRAYS, &result); if (!result) goto defer;
    glCreateShader = get_gl_address(opengl_module, LIBSTRGLCREATESHADER, &result); if (!result) goto defer;
    glShaderSource = get_gl_address(opengl_module, LIBSTRGLSHADERSOURCE, &result); if (!result) goto defer;
    glCompileShader = get_gl_address(opengl_module, LIBSTRGLCOMPILESHADER, &result); if (!result) goto defer;
    glGetShaderiv = get_gl_address(opengl_module, LIBSTRGLGETSHADERIV, &result); if (!result) goto defer;
    glGetShaderInfoLog = get_gl_address(opengl_module, LIBSTRGLGETSHADERINFOLOG, &result); if (!result) goto defer;
    glCreateProgram = get_gl_address(opengl_module, LIBSTRGLCREATEPROGRAM, &result); if (!result) goto defer;
    glAttachShader = get_gl_address(opengl_module, LIBSTRGLATTACHSHADER, &result); if (!result) goto defer;
    glLinkProgram = get_gl_address(opengl_module, LIBSTRGLLINKPROGRAM, &result); if (!result) goto defer;
    glDeleteShader = get_gl_address(opengl_module, LIBSTRGLDELETESHADER, &result); if (!result) goto defer;
    glDeleteProgram = get_gl_address(opengl_module, LIBSTRGLDELETEPROGRAM, &result); if (!result) goto defer;
    glGetProgramiv = get_gl_address(opengl_module, LIBSTRGLGETPROGRAMIV, &result); if (!result) goto defer;
    glGetProgramInfoLog = get_gl_address(opengl_module, LIBSTRGLGETPROGRAMINFOLOG, &result); if (!result) goto defer;
    glUseProgram = get_gl_address(opengl_module, LIBSTRGLUSEPROGRAM, &result); if (!result) goto defer;
    glGetUniformLocation = get_gl_address(opengl_module, LIBSTRGLGETUNIFORMLOCATION, &result); if (!result) goto defer;
    glUniform3fv = get_gl_address(opengl_module, LIBSTRGLUNIFORM3FV, &result); if (!result) goto defer;
    glUniform1i = get_gl_address(opengl_module, LIBSTRGLUNIFORM1I, &result); if (!result) goto defer;
    glUniform1f = get_gl_address(opengl_module, LIBSTRGLUNIFORM1F, &result); if (!result) goto defer;

defer:
    FreeLibrary(opengl_module);
    return result;
}

extern inline void* get_gl_address(HMODULE module, const char* name, BOOL* success) {
    void* proc = (void*)wglGetProcAddress(name);
    if ((proc) == (void*)(0x00) ||
        (proc) == (void*)(0x01) ||
        (proc) == (void*)(0x02) ||
        (proc) == (void*)(0x03) ||
        (proc) == (void*)(-0x1)) {
        (proc) = (void*)GetProcAddress(module, name);
    }
    if (proc == NULL) {
        char caption_buffer[256] = {0};
        char message_buffer[256] = {0};
        snprintf(caption_buffer, sizeof(caption_buffer), "Failure to Load OpenGL function '%s'", name);
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, message_buffer, sizeof(message_buffer), NULL);
        const int msg_result = MessageBoxA(NULL, message_buffer, caption_buffer, MB_OKCANCEL);
        assert(msg_result == IDOK);
        assert(msg_result != FALSE);
        *success = (*success && FALSE);
    }
    *success = (*success && TRUE);
    return proc;
}

#endif // _OPENGL_PROTOTYPES_H
