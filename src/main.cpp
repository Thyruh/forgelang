#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

enum class tokenType {
  _exit,
  semicolon,
  int_lit,
  plus,
  close_paren,
  open_paren
};

struct Token{
  tokenType type;
  std::string value;
};

std::vector<Token> tokenize(const std::string& str) {
  std::vector<Token> tokens;

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

      if (buffer == "exit") {
        tokens.push_back({.type = tokenType::_exit});
        buffer.clear();
        continue;
      }
      else {
        buffer.clear();
        fprintf(stderr, "No exit token found!\n");
        exit(EXIT_FAILURE);
      }
    }


    else if (std::isdigit(c)) {
      buffer.push_back(c);
      i++;
      while (std::isdigit(str.at(i))) {
        buffer.push_back(str.at(i));
        i++;
      }
      i--;
      tokens.push_back({.type = tokenType::int_lit, .value = buffer});
      buffer.clear();
    }
    else if (c == ';') {
      tokens.push_back({.type = tokenType::semicolon});
    }

    else if (c == '+') {
      tokens.push_back({.type = tokenType::plus});
    }

    else if (std::isspace(c)) continue;

    else {
      fprintf(stderr, "No %c token found!\n", c);
      exit(EXIT_FAILURE);
    }

  }
  return tokens;
}


std::string readContents(const char* filepath) {
  std::fstream strm;
  strm.open(filepath, std::ios_base::in);
  if (!strm.is_open()) {
    fprintf(stderr, "Cannot find %s: No such file or directory\n", filepath);
  }

  std::stringstream contents;

  contents << strm.rdbuf();
  return contents.str();
}

std::string tokensToASM(const std::vector<Token>& tokens) {
  std::stringstream output;
  output << "global _start\n_start:\n";
  for (int i = 0; i < tokens.size(); i++) {
    const Token& token = tokens.at(i);
    if (token.type == tokenType::_exit) {
      if (i + 1 < tokens.size() && tokens.at(i + 1).type == tokenType::int_lit) {
        if (i + 2 < tokens.size() && tokens.at(i + 2).type == tokenType::semicolon) {
          output << "    mov rax, 60\n";
          output << "    mov rdi, " << tokens.at(i + 1).value << "\n";
          output << "    syscall\n";
        }
      }
    }
  }
  return output.str();
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("\n");
    fprintf(stderr, "Error: One argument required: a file to compile.\n");
    printf("       Correct syntax: ./forge <name>.fgl\n");
    return EXIT_FAILURE;
  }

  std::string contents = readContents(argv[1]);

  printf("Read file: %s\n", argv[1]);

  const std::vector<Token>& tokens = tokenize(contents);

  {
    std::fstream file("examples/build/out.asm", std::ios::out);
    file << tokensToASM(tokens);
  }

  system("nasm -felf64 examples/build/out.asm");
  system("ld -o examples/build/out examples/build/out.o");
  //  system("rm out.o out.asm");

  return EXIT_SUCCESS;
}
