//
// Created by mkr on 7/27/21.
//

#ifndef CS8_ASM_TREE_EMITTER_HXX
#define CS8_ASM_TREE_EMITTER_HXX
#include "AsmTree.h"
#include "../../cs8_emulator/dependencies/ELFIO/elfio/elfio.hpp"
#include <ostream>
#include <cstdint>
#include <vector>
#include <set>
#include <vector>

class AsmTreeEmitter {
    std::ostream& output_stream;

    enum class section_flags {
        R, W, X
    };
    struct section {
        std::string name;
        size_t addr;
        std::vector<uint8_t> data;
        std::set<section_flags> flags { section_flags::R, section_flags::W, section_flags::X };

    };

    enum class symbol_type {
        Global, Extern, Weak, Static
    };

    struct symbol {
        std::string name;
        section symbol_section;
        symbol_type type;

        std::optional<size_t> address { std::nullopt };
    };

    std::map<std::string, symbol> exported_symbols;

    void emit_elf_file(size_t entrypoint, std::map<std::string, section> const &sections);
    static void create_elf_symtab(ELFIO::elfio &elfio, std::map<std::string, symbol> const &symbols);

public:
    explicit AsmTreeEmitter(std::ostream& output_stream);

    void emit_binary(AsmTree::AsmTree const&);

};


#endif //CS8_ASM_TREE_EMITTER_HXX
