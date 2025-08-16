#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include "../include/tokenizer.h"

std::string readContents(int in_num, char* filepath) {
  std::fstream strm;

  strm.open(filepath, std::ios_base::in);
  if (!strm.is_open()) {
    fprintf(stderr, "Cannot find %s: No such file or directory\n", filepath);
  }

  std::stringstream contents_string;

  contents_string << strm.rdbuf();
  std::string contents = contents_string.str();
  return contents;
}


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("\n");
    fprintf(stderr, "Error: One argument required: a file to compile.\n");
    printf("       Correct syntax: ./forge <name>.forge\n");
    return EXIT_FAILURE;
  }


  std::string contents = readContents(argc, argv[1]);

  printf("Read file: %s.\n", argv[1]);
  printf("Contents:\n\n\"%s \"\n", contents.c_str()); // Lets pretend I never wrote that
}
