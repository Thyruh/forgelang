#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "./nob.h"
#define TARGET  "./forge"
#define OBJ_DIR "./obj"
static const char *src[] = {
    "src/main.c",
    "src/tokenizer.c",
    "src/generator.c",
    "src/parser.c",
    "src/typechecker.c",
};
static const char *obj_path(const char *c_path)
{
    const char *slash = strrchr(c_path, '/');
    const char *basename = slash ? slash + 1 : c_path;
    String_Builder sb = {0};
    sb_append_cstr(&sb, OBJ_DIR "/");
    sb_append_cstr(&sb, basename);
    sb.count -= 2;
    sb_append_cstr(&sb, ".c.o");
    sb_append_null(&sb);
    return sb.items;
}
static bool compile_object(const char *c_path, const char *o_path, bool debug, bool gdb)
{
    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    if (gdb) {
        cmd_append(&cmd,
            "-Wall", "-Wextra", "-std=c2x",
            "-ggdb3", "-O1",
            "-fno-omit-frame-pointer",
            "-fstack-protector-strong",
            "-fstack-clash-protection",
            "-Wshadow", "-Wconversion", "-Wsign-conversion",
            "-Wnull-dereference", "-Wformat=2",
            "-DDEBUG"
        );
    } else if (debug) {
        cmd_append(&cmd,
            "-Wall", "-Wextra", "-std=c2x",
            "-ggdb3", "-O1",
            "-fno-omit-frame-pointer",
            "-fstack-protector-strong",
            "-fstack-clash-protection",
            "-fsanitize=address,undefined",
            "-fno-sanitize-recover=all",
            "-Wshadow", "-Wconversion", "-Wsign-conversion",
            "-Wnull-dereference", "-Wformat=2",
            "-DDEBUG"
        );
    } else {
        cmd_append(&cmd,
            "-Wall", "-Wextra", "-std=c2x", "-pedantic", "-O3",
            "-D_FORTIFY_SOURCE=2",
            "-fstack-protector-strong",
            "-fstack-clash-protection",
            "-Wshadow", "-Wnull-dereference", "-Wformat=2"
        );
    }
    cmd_append(&cmd, "-Iinclude", "-c", c_path, "-o", o_path);
    return cmd_run_sync(cmd);
}
static bool link_target(bool debug, bool gdb)
{
    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    if (gdb) {
        cmd_append(&cmd,
            "-ggdb3"
        );
    } else if (debug) {
        cmd_append(&cmd,
            "-ggdb3",
            "-fsanitize=address,undefined",
            "-fno-sanitize-recover=all"
        );
    } else {
        cmd_append(&cmd,
            "-O3",
            "-Wl,-z,relro,-z,now"
        );
    }
    for (size_t i = 0; i < ARRAY_LEN(src); ++i)
        cmd_append(&cmd, obj_path(src[i]));
    cmd_append(&cmd, "-o", TARGET, "-lm");
    return cmd_run_sync(cmd);
}
static bool delete_file_entry(Walk_Entry entry)
{
    return delete_file(entry.path);
}
static bool cold_clean(void)
{
    nob_log(NOB_INFO, "Cold build: removing %s and %s", OBJ_DIR, TARGET);
    if (file_exists(OBJ_DIR)) {
        if (!walk_dir(OBJ_DIR, delete_file_entry, .post_order = true)) return false;
    }
    if (file_exists(TARGET)) remove(TARGET);
    return true;
}
static bool build(bool debug, bool gdb)
{
    if (!mkdir_if_not_exists(OBJ_DIR)) return false;
    bool any_rebuilt = false;
    for (size_t i = 0; i < ARRAY_LEN(src); ++i) {
        const char *c = src[i];
        const char *o = obj_path(c);
        int rebuild = needs_rebuild1(o, c);
        if (rebuild < 0) return false;
        if (rebuild) {
            nob_log(NOB_INFO, "Compiling %s", c);
            if (!compile_object(c, o, debug, gdb)) return false;
            any_rebuilt = true;
        } else {
            nob_log(NOB_INFO, "Up to date: %s", o);
        }
    }
    int target_missing = needs_rebuild1(TARGET, obj_path(src[0]));
    if (target_missing < 0) return false;
    if (any_rebuilt || target_missing) {
        nob_log(NOB_INFO, "Linking %s", TARGET);
        return link_target(debug, gdb);
    }
    nob_log(NOB_INFO, "%s is up to date", TARGET);
    return true;
}
int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);
    bool debug = false;
    bool cold  = false;
    bool gdb   = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "debug") == 0) debug = true;
        if (strcmp(argv[i], "cold")  == 0) cold  = true;
        if (strcmp(argv[i], "gdb")   == 0) gdb   = true;
    }
    if (cold && !cold_clean()) return 1;
    if (!build(debug, gdb)) return 1;
    return 0;
}
