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

  if (cmd.exist("tokens")) {
    yslang::Lexer lexer(cmd.rest()[0]);
    lexer.print_all();
  }

  yslang::Parser parser(cmd.rest()[0]);
  yslang::File *file = parser.parse();

  if (cmd.exist("ast")) {
    parser.print();
  }

  yslang::CodeGen codegen;
  codegen.generate(file);
  auto module = codegen.getModule();

  std::error_code error_info;
  llvm::raw_fd_ostream raw_stream("out.ll", error_info,
                                  llvm::sys::fs::OpenFlags::F_None);
  module->print(raw_stream, nullptr);

  return 0;
}
