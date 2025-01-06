#ifdef UNICODE // All my homies hate utf-16
#undef UNICODE // it is really annoying
#endif // UNICODE

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "opengl_proto.h"
#include "baked_shaders.h"

#define EPSILON 0.00001
#define MIN_SCALE 0.65
#define FRICTION 0.8
#define MOUSE_SENSITIVITY 0.06
#define WINDOW_CLASS "WinnerWndClass"
#define WINDOW_NAME "Winner"
#define WINDOW_STYLE WS_POPUP | WS_BORDER
#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 3
#define OPENGL_VERSION_STR "v3.3 Core"
#define TARGET_FPS 60.0
#define TARGET_SPF (1.0 / TARGET_FPS)
#define DEFAULT_CAMERA ((Camera){       \
    .pos = (Vec2_t){.x=0.0f,.y=0.0f},   \
    .curr_scl = 1.0f,                   \
    .prev_scl = 1.0f,                   \
    .svel = 0.0f,                       \
    .pvel = (Vec2_t){.x=0.0f,.y=0.0f},  \
    .cursor = (Vec2_t){.x=0.0f, .y=0.0f}\
})
#define DEFAULT_FLASHLIGHT ((Flashlight){\
    .where = (Vec2_t){.x=0.0f,.y=0.0f},  \
    .radius = 0.25f,                     \
    .rvel = 0.0f,                        \
    .enabled = FALSE,                    \
})

typedef struct {
    Vec2_t pos;
    float curr_scl;
    float prev_scl;
    float svel;
    Vec2_t pvel;
    Vec2_t cursor;
} Camera;

typedef struct {
    Vec2_t where;
    float radius;
    float rvel;
    BOOL enabled;
} Flashlight;

typedef struct {
    UINT virtual_pos_x, virtual_pos_y;
    UINT virtual_width, virtual_height;
    HGLRC opengl_context;
    HWND main_window;
    BOOL should_run;
    HDC main_dc, screen_dc, compatible_dc;
    HBITMAP hbm_screenshot;
    double delta_time;
    GLuint screenshot_texture;
    Camera camera;
    Flashlight flashlight;
} GlobalData;
GlobalData global_data = {
    .virtual_pos_x = 0, .virtual_pos_y = 0,
    .virtual_width = 0, .virtual_height = 0,
    .opengl_context = NULL,
    .main_window = NULL,
    .should_run = TRUE,
    .main_dc = NULL, .screen_dc = NULL, .compatible_dc = NULL,
    .hbm_screenshot = NULL,
    .delta_time = 0.0,
    .screenshot_texture = 0,
    .camera = DEFAULT_CAMERA,
    .flashlight = DEFAULT_FLASHLIGHT,
};

#define UNUSED(x) (void)(x)
#define return_defer(value) do { result = (value); goto defer; } while (0)
#define PostQuitAndStop() do { PostQuitMessage(0); global_data.should_run = FALSE; } while (0)
#define about_equals(a, b) ((((a) - (b)) <= EPSILON) && (((a) - (b)) >= -EPSILON))
#define vec2_mag(v) sqrtf((v).x * (v).x + (v).y * (v).y)
void _error_message_box(const char* restrict caption, const int error_num);
_Noreturn void _fatal_message_box(const char* restrict caption, const int error_num);
#define error_message_box(msg) _error_message_box((msg), GetLastError())
#define fatal_message_box(msg) _fatal_message_box((msg), GetLastError())
extern float sqrtf(float);
extern float powf(float, float);
extern inline int64_t get_performance_frequency(void);
extern inline int64_t get_performance_counter(void);
extern inline Vec2_t get_camscl_delta(void);
GLuint get_program(const char* restrict vert, const char* restrict frag, const char* restrict name);
LRESULT WndProc(HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param);

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd) {
    UNUSED(prev_instance); UNUSED(cmd_line);
    global_data.virtual_pos_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    global_data.virtual_pos_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    global_data.virtual_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    global_data.virtual_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    int result = 0;
    { // Create Screenshot
        global_data.screen_dc = GetDC(NULL);
        global_data.compatible_dc = CreateCompatibleDC(global_data.screen_dc);
        global_data.hbm_screenshot = CreateCompatibleBitmap(global_data.screen_dc, global_data.virtual_width, global_data.virtual_height);
        HGDIOBJ old_obj = SelectObject(global_data.compatible_dc, global_data.hbm_screenshot);
        BitBlt(global_data.compatible_dc, 0, 0, global_data.virtual_width, global_data.virtual_width,
               global_data.screen_dc, global_data.virtual_pos_x, global_data.virtual_pos_x, SRCCOPY);
        SelectObject(global_data.compatible_dc, old_obj);
    }

    WNDCLASSEXA wnd_class_ex = {
        .cbSize = sizeof(wnd_class_ex),
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = WndProc,
        .hInstance = instance,
        .hbrBackground = (HBRUSH)1,
        .hIcon = 0,
        .lpszClassName = WINDOW_CLASS,
    };
    if (!RegisterClassExA(&wnd_class_ex))
        fatal_message_box("failure to register window class");

    global_data.main_window = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT, // dwExStyle
        WINDOW_CLASS,                      // lpClassName
        WINDOW_NAME,                       // lpWindowName
        WINDOW_STYLE,                      // dwStyle
        global_data.virtual_pos_x,         // X
        global_data.virtual_pos_y,         // Y
        global_data.virtual_width,         // nWidth
        global_data.virtual_height,        // nHeight
        NULL,                              // hWndParent
        NULL,                              // hMenu
        instance,                          // hInstance
        NULL                               // lpParam
    );
    if (global_data.main_window == NULL)
        fatal_message_box("failure to create window");

    GLfloat screenshot_verts[] = {
    //    x      y     u     v
        -1.0f, -1.0f, 0.0f, 0.0f, //
        +1.0f, -1.0f, 1.0f, 0.0f, // triangle 0
        +1.0f, +1.0f, 1.0f, 1.0f, //
        -1.0f, -1.0f, 0.0f, 0.0f,    //
        +1.0f, +1.0f, 1.0f, 1.0f,    // triangle 1
        -1.0f, +1.0f, 0.0f, 1.0f,    //
    };

    GLuint vao = 0, vbo = 0;
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(screenshot_verts), screenshot_verts, GL_DYNAMIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    GLuint program_screenshot = get_program(screenshot_shader_vert, screenshot_shader_frag, "Screenshot Shader");
    if (program_screenshot == 0)
        PostQuitAndStop();

    GLuint program_flashlight = get_program(flashlight_shader_vert, flashlight_shader_frag, "Flashlight Shader");
    if (program_screenshot == 0)
        PostQuitAndStop();

    if (global_data.should_run)
        ShowWindow(global_data.main_window, show_cmd);

    const float aratio = global_data.virtual_width / global_data.virtual_height;

    MSG Msg = {0};
    const double performance_frequency = (double)get_performance_frequency();
    int64_t performance_count_prev = get_performance_counter();
    int64_t performance_count_curr;
    while (global_data.should_run) {
        while (PeekMessageA(&Msg, global_data.main_window, 0, 0, PM_REMOVE))
            DispatchMessageA(&Msg);

        performance_count_curr = get_performance_counter();
        global_data.delta_time = ((double)llabs(performance_count_curr - performance_count_prev)) / performance_frequency;
        if (global_data.delta_time < TARGET_SPF) continue;
        else performance_count_prev = performance_count_curr;

        Flashlight* flashlight = &global_data.flashlight;
        Camera* camera = &global_data.camera;
        if (!about_equals(camera->svel, 0.0f)) {
            camera->prev_scl = camera->curr_scl;
            camera->curr_scl += camera->svel * global_data.delta_time;
            camera->svel *= FRICTION;
            if (camera->curr_scl < MIN_SCALE) {
                camera->curr_scl = MIN_SCALE;
                camera->svel = 0.0f;
            }
            const Vec2_t delta_pos = get_camscl_delta();
            camera->pos.x -= delta_pos.x;
            camera->pos.y -= delta_pos.y;
        } else camera->svel = 0.0f;

        if (!about_equals(vec2_mag(camera->pvel), 0.0f)) {
            camera->pos.x += camera->pvel.x * global_data.delta_time;
            camera->pos.y += camera->pvel.y * global_data.delta_time;
            camera->pvel.x *= FRICTION;
            camera->pvel.y *= FRICTION;
        } else camera->pvel = (Vec2_t){.x=0.0f,.y=0.0f};

        if (!about_equals(flashlight->rvel, 0.0f)) {
            flashlight->radius += flashlight->rvel * global_data.delta_time;
            flashlight->rvel *= FRICTION;
            if (flashlight->radius < 0.01) {
                flashlight->radius = 0.01;
                flashlight->rvel = 0.0f;
            }
        } else flashlight->rvel = 0.0f;

        POINT cursor_mpos;
        GetCursorPos(&cursor_mpos);
        flashlight->where = (Vec2_t){
            .x = 2.0f * (((float)cursor_mpos.x) / ((float)(global_data.virtual_width - 1))) - 1.0f,
            .y = 2.0f * (((float)cursor_mpos.y) / ((float)(global_data.virtual_height - 1))) - 1.0f,
        };
        flashlight->where.x *= aratio;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program_screenshot);
        glBindTexture(GL_TEXTURE_2D, global_data.screenshot_texture);
        glBindVertexArray(vao);
        glUniform3fv(glGetUniformLocation(program_screenshot, "transforms"), 1, (void*)camera);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(program_flashlight);
        glBindVertexArray(vao);
        glUniform3fv(glGetUniformLocation(program_flashlight, "flashlight"), 1, (void*)flashlight);
        glUniform1i(glGetUniformLocation(program_flashlight, "flash_enabled"), flashlight->enabled ? 1 : 0);
        glUniform1f(glGetUniformLocation(program_flashlight, "aratio"), aratio);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SwapBuffers(global_data.main_dc);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    if (program_screenshot != 0) glDeleteProgram(program_screenshot);
    if (program_flashlight != 0) glDeleteProgram(program_flashlight);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    DeleteDC(global_data.compatible_dc);
    ReleaseDC(NULL, global_data.screen_dc);
    DeleteObject(global_data.hbm_screenshot);
    UnregisterClassA(WINDOW_CLASS, instance);
    return result;
}

#define keycombo(keycode, plus_ctrl, plus_shift) \
    (w_param == (keycode)) &&\
    (ctrl_depressed == (plus_ctrl)) &&\
    (shift_depressed == (plus_shift))

LRESULT WndProc(HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    static Vec2_t drag_start = {.x=0.0f,.y=0.0f};
    switch (msg) {
        case WM_DESTROY: {
            wglMakeCurrent(global_data.main_dc, NULL);
            wglDeleteContext(global_data.opengl_context);
            DeleteDC(global_data.main_dc);
            PostQuitAndStop();
        } return 0;

        case WM_PAINT: {
            PAINTSTRUCT paint_struct;
            HDC paint_dc = BeginPaint(wnd, &paint_struct);
            HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(paint_dc, &paint_struct.rcPaint, brush);
            DeleteObject(brush);
            EndPaint(wnd, &paint_struct);
        } return 0;

        case WM_LBUTTONDOWN: {
            POINT point = { (int16_t)(l_param >> 0), (int16_t)(l_param >> 16), };
            if (DragDetect(wnd, point)) {
                drag_start = (Vec2_t){
                    .x = 2.0f * (((float)point.x) / ((float)(global_data.virtual_width - 1))) - 1.0f,
                    .y = -(2.0f * (((float)point.y) / ((float)(global_data.virtual_height - 1))) - 1.0f),
                };
                drag_start.x = (drag_start.x - global_data.camera.pos.x) / global_data.camera.curr_scl;
                drag_start.y = (drag_start.y - global_data.camera.pos.y) / global_data.camera.curr_scl;
            }
        } return 0;

        case WM_MOUSEMOVE: {
            POINT point = { (int16_t)(l_param >> 0), (int16_t)(l_param >> 16), };
            if (DragDetect(wnd, point)) {
                Vec2_t where = {
                    .x = 2.0f * (((float)point.x) / ((float)(global_data.virtual_width - 1))) - 1.0f,
                    .y = -(2.0f * (((float)point.y) / ((float)(global_data.virtual_height - 1))) - 1.0f),
                };
                where.x = (where.x - global_data.camera.pos.x) / global_data.camera.curr_scl;
                where.y = (where.y - global_data.camera.pos.y) / global_data.camera.curr_scl;

                global_data.camera.pos.x += where.x - drag_start.x;
                global_data.camera.pos.y += where.y - drag_start.y;
            }
        } return 0;

        case WM_MOUSEWHEEL: {
            const float motion = (float)(int16_t)(w_param >> 16); // neg == wheel down, pos == wheel up
            if (global_data.flashlight.enabled && ((w_param & MK_SHIFT) != 0)) {
                global_data.flashlight.rvel += motion * MOUSE_SENSITIVITY * 0.1;
            } else {
                if (motion > 0.0f) global_data.camera.svel += global_data.camera.curr_scl * motion * MOUSE_SENSITIVITY;
                else global_data.camera.svel += motion * MOUSE_SENSITIVITY * powf(global_data.camera.curr_scl, 0.5f);
                global_data.camera.cursor.x = (2.0f * ((float)((int16_t)(l_param >>  0))) / (global_data.virtual_width - 1)) - 1.0f;
                global_data.camera.cursor.y = -((2.0f * ((float)((int16_t)(l_param >> 16))) / (global_data.virtual_height - 1)) - 1.0f);
            }
        } return 0;

        case WM_KEYDOWN: {
            const BOOL ctrl_depressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            const BOOL shift_depressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            UNUSED(shift_depressed);
            if (keycombo('C', TRUE, FALSE)) {
                HBITMAP screenshot = CreateCompatibleBitmap(global_data.screen_dc, global_data.virtual_width, global_data.virtual_height);
                HGDIOBJ old_obj = SelectObject(global_data.compatible_dc, screenshot);
                BitBlt(global_data.compatible_dc, 0, 0, global_data.virtual_width, global_data.virtual_width,
                       global_data.screen_dc, global_data.virtual_pos_x, global_data.virtual_pos_x, SRCCOPY);
                OpenClipboard(NULL);
                EmptyClipboard();
                SetClipboardData(CF_BITMAP, screenshot);
                CloseClipboard();
                SelectObject(global_data.compatible_dc, old_obj);
                DeleteObject(screenshot);

            } else if (keycombo('F', FALSE, FALSE)) {
                if (global_data.flashlight.enabled) {
                    global_data.flashlight = DEFAULT_FLASHLIGHT;
                    return 0;
                }
                global_data.flashlight.enabled = TRUE;

            } else if (keycombo('P', FALSE, TRUE)) {
                global_data.camera.pos.x = 0.0f;
                global_data.camera.pos.y = 0.0f;

#ifdef DEBUG
            } else if (keycombo('T', FALSE, FALSE)) {
                char buf[128] = {0};
                snprintf(buf, sizeof(buf), "scale = %f", global_data.camera.curr_scl);
                MessageBoxA(wnd, buf, "tell", MB_OK);
#endif // DEBUG

            } else if (keycombo(VK_LEFT, FALSE, FALSE)) {
                global_data.camera.pvel.x += 5.0f;
            } else if (keycombo(VK_RIGHT, FALSE, FALSE)) {
                global_data.camera.pvel.x -= 5.0f;
            } else if (keycombo(VK_UP, FALSE, FALSE)) {
                global_data.camera.pvel.y -= 5.0f;
            } else if (keycombo(VK_DOWN, FALSE, FALSE)) {
                global_data.camera.pvel.y += 5.0f;

            } else if (keycombo('R', FALSE, FALSE)) {
                global_data.camera = DEFAULT_CAMERA;
            } else if (keycombo('S', FALSE, FALSE)) {
                static BOOL is_nearest = TRUE;
                if (is_nearest) {
                    glBindTexture(GL_TEXTURE_2D, global_data.screenshot_texture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } else {
                    glBindTexture(GL_TEXTURE_2D, global_data.screenshot_texture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                is_nearest = !is_nearest;
            } else if (w_param == 'Q' || w_param == VK_ESCAPE) {
                PostQuitAndStop();
            }
        } return 0;

        case WM_CREATE: {
            global_data.main_dc = GetDC(wnd);

            int result = 0;
            WNDCLASSA dummy_class = {0};
            HWND dummy_window = NULL;
            HDC dummy_dc = NULL;
            HGLRC dummy_opengl_context = NULL;
            void* buffer = NULL;
            BITMAP screenshot_bm = {0};
            HGDIOBJ old_object = NULL;

            dummy_class = (WNDCLASSA){
                .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                .lpfnWndProc = DefWindowProcA,
                .hInstance = GetModuleHandle(0),
                .lpszClassName = "DummyWinnerClassWGL",
            };
            if (!RegisterClassA(&dummy_class)) {
                error_message_box("OpenGL Init: Failure to Register DUMMY Window Class");
                return 1;
            }
            
            dummy_window = CreateWindowExA(
                0, dummy_class.lpszClassName, "Dummy Winner Window for WGL", 0,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                0, 0, dummy_class.hInstance, 0
            );
            if (dummy_window == NULL) {
                error_message_box("OpenGL Init: Failure to Create DUMMY WindowEx");
                return_defer(1);
            }

            dummy_dc = GetDC(dummy_window);
            if (dummy_dc == NULL) {
                error_message_box("OpenGL Init: Failure to Create DUMMY Device Context");
                return_defer(1);
            }

            PIXELFORMATDESCRIPTOR dummy_pfd = {
                .nSize = sizeof(dummy_pfd),
                .nVersion = 1,
                .iPixelType = PFD_TYPE_RGBA,
                .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                .cColorBits = 32,
                .cAlphaBits = 8,
                .iLayerType = PFD_MAIN_PLANE,
                .cDepthBits = 24,
                .cStencilBits = 8,
            };
            int dummy_pixel_format = ChoosePixelFormat(dummy_dc, &dummy_pfd);
            if (dummy_pixel_format == 0) {
                error_message_box("OpenGL Init: Failure to CHOOSE a DUMMY Pixel Format");
                return_defer(1);
            }
            if (!SetPixelFormat(dummy_dc, dummy_pixel_format, &dummy_pfd)) {
                error_message_box("OpenGL Init: Failure to SET a DUMMY Pixel Format");
                return_defer(1);
            }

            dummy_opengl_context = wglCreateContext(dummy_dc);
            if (dummy_opengl_context == NULL) {
                error_message_box("OpenGL Init: Failure to Create DUMMY OpenGL Context");
                return_defer(1);
            }

            if (!wglMakeCurrent(dummy_dc, dummy_opengl_context)) {
                error_message_box("OpenGL Init: Failure to Set DUMMY OpenGL Context");
                return_defer(1);
            }

            if (!load_all_opengl_procs())
                return_defer(1);

            int real_pixel_attributes[] = {
                WGL_NUMBER_PIXEL_FORMATS_ARB, 1,
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                WGL_TRANSPARENT_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB, 32,
                WGL_DEPTH_BITS_ARB, 24,
                WGL_STENCIL_BITS_ARB, 8,
                0 // NULL-TERMINATOR
            };
            int real_pixel_format;
            UINT num_real_formats;
            if (!wglChoosePixelFormatABR(global_data.main_dc, real_pixel_attributes, NULL, 1, &real_pixel_format, &num_real_formats)) {
                _error_message_box("OpenGL Init: Failure to Choose REAL Pixel Format", glGetError());
                return_defer(1);
            } else if (num_real_formats == 0) {
                _error_message_box("OpenGL Init: Failure to find matching Real Pixel Format", glGetError());
                return_defer(1);
            }

            PIXELFORMATDESCRIPTOR real_pfd;
            DescribePixelFormat(global_data.main_dc, real_pixel_format, sizeof(real_pfd), &real_pfd);
            if (!SetPixelFormat(global_data.main_dc, real_pixel_format, &real_pfd)) {
                error_message_box("OpenGL Init: Failure to SET REAL Pixel Format");
                return_defer(1);
            }

            int opengl_attribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_VERSION_MAJOR,
                WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_VERSION_MINOR,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0 // NULL-TERMINATOR
            };
            global_data.opengl_context = wglCreateContextAttribsABR(global_data.main_dc, 0, opengl_attribs);
            if (global_data.opengl_context == NULL) {
                error_message_box("OpenGL Init: Failure to Create Real and Final Context (" OPENGL_VERSION_STR ")");
                return_defer(1);
            }
            assert(wglMakeCurrent(global_data.main_dc, global_data.opengl_context) == TRUE);
            glViewport(
                global_data.virtual_pos_x,
                global_data.virtual_pos_y,
                global_data.virtual_width,
                global_data.virtual_height
            );

            // Converting the screenshot to an OpenGL texture
            GetObject(global_data.hbm_screenshot, sizeof(screenshot_bm), &screenshot_bm);
            const size_t buffer_size = screenshot_bm.bmWidth * screenshot_bm.bmHeight * (screenshot_bm.bmBitsPixel / 8);
            buffer = malloc(buffer_size);
            assert(buffer != NULL && "Buy more ram lol");

            old_object = SelectObject(global_data.compatible_dc, global_data.hbm_screenshot);
            if (!GetBitmapBits(global_data.hbm_screenshot, (LONG)buffer_size, (LPVOID)buffer)) {
                int msg_result = MessageBoxA(wnd, "Failure to copy data to tmp buffer", "ScreenShotToTexture Error", MB_OKCANCEL);
                assert(msg_result == IDOK);
                assert(msg_result != FALSE);
                return_defer(1);
            }

            glGenTextures(1, &global_data.screenshot_texture);
            glBindTexture(GL_TEXTURE_2D, global_data.screenshot_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         screenshot_bm.bmWidth, screenshot_bm.bmHeight, 0,
                         GL_RGBA,
                         GL_UNSIGNED_INT_8_8_8_8_REV,
                         buffer);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        defer:
            if (buffer) free(buffer);
            if (old_object) SelectObject(global_data.compatible_dc, old_object);
            if (global_data.screenshot_texture != 0) glBindTexture(GL_TEXTURE_2D, 0);
            if ((result != 0) && (global_data.screenshot_texture != 0)) glDeleteTextures(1, &global_data.screenshot_texture);
            if (dummy_opengl_context) wglDeleteContext(dummy_opengl_context);
            if (dummy_dc) ReleaseDC(dummy_window, dummy_dc);
            if (dummy_window) DestroyWindow(dummy_window);
            UnregisterClassA(dummy_class.lpszClassName, dummy_class.hInstance);

            assert((result == 0) && "failure to initialize opengl");
            return result;
        } __builtin_unreachable();

        default: return DefWindowProc(wnd, msg, w_param, l_param);
    }
}
#undef keycombo

void _error_message_box(const char* restrict caption, const int error_num) {
    char msg[512] = {0};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_num, LANG_USER_DEFAULT, msg, sizeof(msg), NULL);
    const int msg_result = MessageBoxA(global_data.main_window, msg, caption, MB_OKCANCEL);
    assert(msg_result == IDOK);
    assert(msg_result != FALSE);
}

void _fatal_message_box(const char* restrict caption, const int error_num) {
    char msg[512] = {0};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_num, LANG_USER_DEFAULT, msg, sizeof(msg), NULL);
    MessageBoxA(global_data.main_window, msg, caption, MB_OKCANCEL);
    assert(0 && "Fatal Error Encountered!!!");
}

extern inline int64_t get_performance_frequency(void) {
    int64_t result;
    QueryPerformanceFrequency((void*)&result);
    return result;
}

extern inline int64_t get_performance_counter(void) {
    int64_t result;
    QueryPerformanceCounter((void*)&result);
    return result;
}

    /*CLOSE!!!*/
    /*Vec2_t result = {*/
    /*    .x = ((cur.x - pos.x) / csp) - ((cur.x - pos.x - (pos.x * csp)) / (csc * csp)),*/
    /*    .y = ((cur.y - pos.y) / csp) - ((cur.y - pos.y - (pos.y * csp)) / (csc * csp)),*/
    /*};*/

    /*COOL EVEN THO IT IS WRONG*/
    /*const Vec2_t b = {*/
    /*    .x = (cur.x - pos.x) / csp,*/
    /*    .y = (cur.y - pos.y) / csp,*/
    /*};*/
    /*const Vec2_t a = {*/
    /*    .x = (b.x - pos.x) / csc,*/
    /*    .y = (b.y - pos.y) / csc,*/
    /*};*/
    /**/
    /*Vec2_t result = {*/
    /*    .x = b.x - a.x,*/
    /*    .y = b.y - a.y,*/
    /*};*/
extern inline Vec2_t get_camscl_delta(void) {
    const float csc = global_data.camera.curr_scl;
    const float csp = global_data.camera.prev_scl;
    const Vec2_t pos = global_data.camera.pos;
    const Vec2_t cur = {
        .x = (global_data.camera.cursor.x - pos.x) / csp,
        .y = (global_data.camera.cursor.y - pos.y) / csp,
    };
    return (Vec2_t){
        .x = ((cur.x * csc) + pos.x) - ((cur.x * csp) + pos.x),
        .y = ((cur.y * csc) + pos.y) - ((cur.y * csp) + pos.y),
    };
}

GLuint get_program(const char* restrict vert, const char* restrict frag, const char* restrict name) {
    GLuint result = glCreateProgram();
    BOOL success = TRUE;
    char info_log[512] = {0};
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_shader, 1, (void*)&vert, NULL);
    glCompileShader(vert_shader);
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char title[128] = {0};
        glGetShaderInfoLog(vert_shader, sizeof(info_log), NULL, info_log);
        snprintf(title, sizeof(title), "Shader Vert Error: %s", name);
        assert(MessageBoxA(global_data.main_window, info_log, title, MB_OKCANCEL) == IDOK);
        goto defer;
    }

    glShaderSource(frag_shader, 1, (void*)&frag, NULL);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char title[128] = {0};
        glGetShaderInfoLog(frag_shader, sizeof(info_log), NULL, info_log);
        snprintf(title, sizeof(title), "Shader Frag Error: %s", name);
        assert(MessageBoxA(global_data.main_window, info_log, title, MB_OKCANCEL) == IDOK);
        goto defer;
    }


    glAttachShader(result, vert_shader);
    glAttachShader(result, frag_shader);
    glLinkProgram(result);
    glGetProgramiv(result, GL_LINK_STATUS, &success);
    if (!success) {
        char title[128] = {0};
        glGetProgramInfoLog(result, sizeof(info_log), NULL, info_log);
        snprintf(title, sizeof(title), "Shader Link Error: %s", name);
        assert(MessageBoxA(global_data.main_window, info_log, title, MB_OKCANCEL) == IDOK);
        goto defer;
    }

defer:
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    if (!success) {
        glDeleteProgram(result);
        return 0;
    } else {
        return result;
    }
}
