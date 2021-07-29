//
// Created by mkr on 7/27/21.
//

#ifndef CS8_ASM_TREE_EMITTER_HXX
#define CS8_ASM_TREE_EMITTER_HXX
#include "AsmTree.h"
#include <ostream>
#include <cstdint>
#include <vector>

class AsmTreeEmitter {
    std::ostream& output_stream;

    struct section {
        size_t addr;
        std::vector<uint8_t> data;
    };

    void emit_elf_file(size_t entrypoint, std::map<std::string, section> const &sections);
public:
    explicit AsmTreeEmitter(std::ostream& output_stream);

    void emit_binary(AsmTree::AsmTree const&);

};


#endif //CS8_ASM_TREE_EMITTER_HXX
