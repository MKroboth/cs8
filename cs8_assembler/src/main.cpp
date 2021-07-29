//
// Created by mkr on 25.07.21.
//

#include "cs8_parser.h"
#include "AsmTreeTransformer.h"
#include "asm_tree_emitter.hxx"
#include "CS8_Assembler.hxx"
#include <optional>
#include <filesystem>



int main(int argc, const char* argv[]) {
   if(argc < 2) return -1;
   auto infile = argv[1];
   std::filesystem::path outfile = "out.elf";

   cs8_assembler assembler;
   assembler.assemble(outfile, infile);
}