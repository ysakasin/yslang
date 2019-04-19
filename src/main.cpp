#include <iostream>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "../third_party/cmdline.h"
#include "./codegen.hpp"
#include "./lexer.hpp"
#include "./parser.hpp"
#include "./token.hpp"

int main(int argc, char *argv[]) {
  cmdline::parser cmd;
  cmd.add("tokens", 't', "print lexed tokens");
  cmd.add("ast", 'a', "print ast");
  cmd.footer("file");

  cmd.parse_check(argc, argv);

  if (cmd.rest().size() == 0) {
    std::cout << cmd.usage();
    return 0;
  }

  std::string path = cmd.rest()[0];

  std::ifstream ifs(path);
  if (ifs.fail()) {
    std::cerr << "Can not open " << path << std::endl;
  }

  std::istreambuf_iterator<char> it(ifs);
  std::istreambuf_iterator<char> last;
  std::string input(it, last);

  if (cmd.exist("tokens")) {
    yslang::Lexer lexer(input);
    for (yslang::Token t = lexer.next(); t.type != yslang::TokenType::TEOF;
         t = lexer.next()) {
      std::cout << t << std::endl;
    }
  }

  yslang::Parser parser(input);
  yslang::Program program = parser.parse();

  if (parser.has_error()) {
    for (const auto &msg : parser.error_messages) {
      std::cerr << msg << std::endl;
    }
    return 1;
  }

  // if (cmd.exist("ast")) {
  std::cout << program.toJson().to_string() << std::endl;
  // }

  // yslang::CodeGen codegen;
  // codegen.generate(file);
  // auto module = codegen.getModule();

  // std::error_code error_info;
  // llvm::raw_fd_ostream raw_stream("out.ll", error_info,
  //                                 llvm::sys::fs::OpenFlags::F_None);
  // module->print(raw_stream, nullptr);

  return 0;
}
