#include<cstdio>
#include<cstdlib>
#include<iterator>
#include<fstream>
#include<vector>
#include<string>
#include<sstream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("\n");
    fprintf(stderr, "Error: One argument required: a file to compile.\n");
    printf("       Correct syntax: ./forge <name>.forge\n");
    return EXIT_FAILURE;
  }

  std::fstream strm;

  strm.open(argv[1], std::ios_base::in);
  printf("\n%s\n", argv[1]);
  if (!strm.is_open()) {
    fprintf(stderr, "Cannot find %s: No such file or directory", argv[1]);
    return 1;
  }

  std::stringstream contents_string;

  contents_string << strm.rdbuf();
  std::string contents = contents_string.str();

  printf("Read file: %s.\n", argv[1]);
  printf("Contents:\n\n\"%s \"\n", contents.c_str()); // Lets pretend I never wrote that
  return EXIT_SUCCESS;
}
