#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

enum class tokenType{
  _return,
  semicolon,
  int_lit,
  open_paren,
  close_paren
};

struct Token{
  tokenType type;
  std::string value;
};

std::vector<Token> tokenize(std::string& str) {
  std::vector<Token> tokens = {};

  std::string buffer;

  for (int i = 0; i < str.size(); i++) {
    char c = str.at(i);
    if (std::isalpha(c)) {
      buffer.push_back(c);
      i++;
      while (std::isalnum(str.at(i))) {
        buffer.push_back(str.at(i));
        i++;
      }
      i--;

      if (buffer == "return") {
        tokens.push_back({.type = tokenType::_return});
        buffer.clear();
        continue;
      }
    }

    else if (std::isdigit(c)) {
      buffer.push_back(c);
      i++;
      while (std::isdigit(c)) {
        buffer.push_back(c);
        i++;
      }
      i--;
      tokens.push_back({.type = tokenType::int_lit, .value = buffer});
      buffer.clear();
    }
    else if (c == ';') {
      tokens.push_back({.type = tokenType::semicolon});
    }

    else if (std::isspace(c)) {
      continue;
    }

    else {
      fprintf(stderr, "No token found!\n");
      exit(EXIT_FAILURE);
    }
  }
  return tokens;
}


std::string readContents(int in_num, char* filepath) {
  std::fstream strm;

  strm.open(filepath, std::ios_base::in);
  if (!strm.is_open()) {
    fprintf(stderr, "Cannot find %s: No such file or directory\n", filepath);
  }

  std::stringstream contents_string;

  contents_string << strm.rdbuf();
  return contents_string.str();
}


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("\n");
    fprintf(stderr, "Error: One argument required: a file to compile.\n");
    printf("       Correct syntax: ./forge <name>.forge\n");
    return EXIT_FAILURE;
  }


  std::string contents = readContents(argc, argv[1]);

  printf("Read file: %s\n", argv[1]);
  tokenize(contents);
  return EXIT_SUCCESS;
}
