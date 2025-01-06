#define NOB_IMPLEMENTATION
#include "./build/nob.h"

#if defined(_WIN32) || defined(_WIN64)
#define BAKED_FILE ".\\baked_shaders.h"
#define EXE_NAME "winner.exe"
#define COMPILER "gcc"
#define RUN_CMD (const char*[]){ ".\\"EXE_NAME }
#define LINKS (const char*[]){ "-lUser32", "-lKernel32", "-lGdi32", "-lOpengl32" }
#define NEWLINE "\r\n"
#elif defined(__linux)
#define BAKED_FILE "./baked_shaders.h"
#define EXE_NAME "./linux/winner.exe"
#define COMPILER "x86_64-w64-mingw32-gcc"
#define RUN_CMD (const char*[]) { "wine", EXE_NAME }
#define LINKS (const char*[]){ "-luser32", "-lkernel32", "-lgdi32", "-lopengl32" }
#define NEWLINE "\n"
#endif // defined(_WIN32) || defined(_WIN64)

#define cmd_append_links(cmdptr, linkmacro) nob_da_append_many((cmdptr), (linkmacro), sizeof(linkmacro)/sizeof(*linkmacro))
#define cmd_append_run(cmdptr, runmacro) nob_da_append_many((cmdptr), (runmacro), sizeof(runmacro)/sizeof(*runmacro))

typedef struct {
    bool echo, no_bake, dry;
    const char* opt;
    Nob_Cmd run;
} Todo;

bool get_todo(int argc, char** argv, Todo* todo);
bool bake_shaders(const Todo todo);

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    Todo todo;
    if (!get_todo(argc, argv, &todo))
        return 1;

    int result = 0;
    if (!bake_shaders(todo))
        nob_return_defer(1);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, COMPILER, "main.c", todo.opt);
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-Werror", "-static");
    if (strcmp(todo.opt, "-Os") == 0) {
        nob_cmd_append(&cmd, "-fdata-sections", "-ffunction-sections");
        nob_cmd_append(&cmd, "-s");
    }
    if (strcmp(todo.opt, "-O0") == 0) {
        nob_cmd_append(&cmd, "-DDEBUG");
    }
    cmd_append_links(&cmd, LINKS);
    nob_cmd_append(&cmd, "-Wl,--subsystem=windows");
    nob_cmd_append(&cmd, "-o", EXE_NAME);

    if (todo.echo) {
        Nob_String_Builder sb = {0};
        nob_cmd_render(cmd, &sb);
        nob_sb_append_null(&sb);
        printf("cmd: %s"NEWLINE, sb.items);
        nob_sb_free(sb);
        if (todo.run.count <= 0) nob_return_defer(0);
        memset(&sb, 0, sizeof sb);
        nob_cmd_render(todo.run, &sb);
        nob_sb_append_null(&sb);
        printf("cmd: %s"NEWLINE, sb.items);
        nob_sb_free(sb);
        nob_return_defer(0);
    }

    if (todo.dry)
        nob_return_defer(0);

    if (!nob_cmd_run_sync(cmd)) {
        nob_log(NOB_ERROR, "failure to compile %s", EXE_NAME);
        nob_return_defer(1);
    } else nob_log(NOB_INFO, "successfully compiled %s", EXE_NAME);
    if (todo.run.count <= 0) nob_return_defer(0);

    if (!nob_cmd_run_sync(todo.run)) {
        nob_log(NOB_ERROR, "failure to run %s", EXE_NAME);
        nob_return_defer(1);
    } else nob_log(NOB_INFO, "successfully ran %s", EXE_NAME);

defer:
    if (todo.run.items) nob_cmd_free(todo.run);
    if (cmd.items) nob_cmd_free(cmd);
    return result;
}

bool get_todo(int argc, char** argv, Todo* todo) {
    memset(todo, 0, sizeof *todo);
    bool result = true;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "echo") == 0) todo->echo = true;
        else if (strcmp(argv[i], "O0") == 0) todo->opt = "-O0";
        else if (strcmp(argv[i], "O1") == 0) todo->opt = "-O1";
        else if (strcmp(argv[i], "O2") == 0) todo->opt = "-O2";
        else if (strcmp(argv[i], "O3") == 0) todo->opt = "-O3";
        else if (strcmp(argv[i], "Os") == 0) todo->opt = "-Os";
        else if (strcmp(argv[i], "no-bake") == 0) todo->no_bake = true;
        else if (strcmp(argv[i], "dry") == 0) todo->dry = true;
        else if (strcmp(argv[i], "run") == 0) cmd_append_run(&todo->run, RUN_CMD);
        else if (strcmp(argv[i], "release") == 0) {
            todo->no_bake = false;
            todo->dry = false;
            todo->opt = "-Os";
        } else {
            nob_log(NOB_ERROR, "unknown argument '%s'", argv[i]);
            nob_return_defer(false);
        }
    }
defer:
    if (!result && todo->run.items) nob_cmd_free(todo->run);
    if (todo->opt == NULL) todo->opt = "-O0";
    return result;
}

bool bake_shaders(const Todo todo) {
    if (todo.echo || todo.no_bake) return true;
    nob_log(NOB_INFO, "baking shaders into '%s'", BAKED_FILE);

    bool result = true;
    Nob_File_Paths shaders_dir = {0};
    Nob_String_Builder file_data = {0};
    FILE* baked_file = fopen(BAKED_FILE, "w");
    assert(bake_shaders != NULL);

    fprintf(baked_file, "#ifndef _BAKED_SHADERS_H\n");
    fprintf(baked_file, "#define _BAKED_SHADERS_H\n\n");
    if (!nob_read_entire_dir(".\\shaders\\", &shaders_dir))
        nob_return_defer(false);

    void* const end = shaders_dir.items + shaders_dir.count;
    for (char* const * cur = (void*)shaders_dir.items; cur != end; ++cur) {
        if (strcmp(*cur, ".") == 0) continue;
        else if (strcmp(*cur, "..") == 0) continue;
        char buffer[512] = {0};
        
        char* head = buffer;
        const char* tail = *cur + strlen(*cur);
        for (int i = strlen(*cur); i >= 0; --i) {
            if ((*cur)[i] == '.') break;
            else tail--;
        }
        if (strcmp(tail, ".glsl") != 0) continue;
        for (int i = 0; i < strlen(*cur); ++i) {
            assert(i < (sizeof(buffer) / 2));
            if ((*cur)[i] == '.') break;
            else head[i] = (*cur)[i];
        }

        fprintf(baked_file, "const char* %s = \"", head);
        nob_log(NOB_INFO, "baking '.\\shaders\\%s' as 'const char* %s'", *cur, head);

        char* cur_path = buffer + (sizeof(buffer) / 2);
        assert(snprintf(cur_path, (sizeof(buffer) / 2), ".\\shaders\\%s", *cur) >= 0);
        if (!nob_read_entire_file(cur_path, &file_data))
            nob_return_defer(false);

        for (int i = 0; i < file_data.count; ++i) {
            switch (file_data.items[i]) {
                case '\r': break;
                case '\n': {
                    fprintf(baked_file, "\\n\"\n    \"");
                } break;
                default: { 
                    fprintf(baked_file, "%c", file_data.items[i]);
                } break;
            }
        }

        fprintf(baked_file, "\";\n\n");
        memset(file_data.items, 0, file_data.capacity * sizeof(file_data.items[0]));
        file_data.count = 0;
    }

defer:
    fprintf(baked_file, "\n#endif // _BAKED_SHADERS_H\n");
    fclose(baked_file);
    if (shaders_dir.items) nob_da_free(shaders_dir);
    if (file_data.items) nob_sb_free(file_data);
    return result;
}
