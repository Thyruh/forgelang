#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "./nob.h"

#define SRC "src/main.c", \
   "src/tokenizer.c", \
   "src/parser.c" 
#define TARGET "forgelang"

int main(int argc, char** argv) {
   GO_REBUILD_URSELF(argc, argv);
   Cmd cmd= { 0 };

   cmd_append(&cmd, "cc", "-o", TARGET, "-Wall", "-Wextra", "-pedantic", SRC);
   if (!cmd_run_sync(cmd)) return 1;
   return 0;
}
