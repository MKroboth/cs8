//
// Created by mkr on 7/27/21.
//

#include "asm_tree_emitter.hxx"

#include <sstream>
#include <elfio/elfio.hpp>
#include <filesystem>
#include <cassert>

void AsmTreeEmitter::emit_binary(const AsmTree::AsmTree &asm_tree) {
    std::map<std::string, section> sections;
    size_t entry = 0;
    std::string current_section = "flat";



    for (auto const &node : asm_tree.nodes) {
        switch (node->get_type()) {

            case AsmTree::AsmTreeType::Label: {
                auto const &label = dynamic_cast<AsmTree::AsmTreeLabel const &>(*node);
            }
                break;
            case AsmTree::AsmTreeType::Instruction: {
                auto const &instruction = dynamic_cast<AsmTree::Instruction::AsmTreeInstructionNode const &>(*node);

                auto &current_section_data = sections.at(current_section).data;

                if (auto const *b = dynamic_cast<AsmTree::Instruction::AsmTreeInstruction1BNode const *>(&instruction)) {
                    auto binary = b->emit();
                    current_section_data.push_back(binary[0]);
                } else if (auto const *b = dynamic_cast<AsmTree::Instruction::AsmTreeInstruction2BNode const *>(&instruction)) {
                    auto binary = b->emit();
                    current_section_data.push_back(binary[0]);
                    current_section_data.push_back(binary[1]);
                } else if (auto const *b = dynamic_cast<AsmTree::Instruction::AsmTreeInstruction3BNode const *>(&instruction)) {
                    auto binary = b->emit();
                    current_section_data.push_back(binary[0]);
                    current_section_data.push_back(binary[1]);
                    current_section_data.push_back(binary[2]);

                }
            }
                break;
            case AsmTree::AsmTreeType::Directive: {
                auto const &directive = dynamic_cast<AsmTree::AsmTreeDirective const &>(*node);

                if (directive.name == "entrypoint") {
                    entry = std::stoull(directive.args.at(0));
                } else if (directive.name == "section") {
                    current_section = directive.args.at(0);
                    if (!sections.contains(current_section)) {
                        sections[current_section] = section{current_section, std::stoull(directive.args.at(1)), std::vector<uint8_t>()};
                    }
                } else if (directive.name == "global") {
                    auto symbol_name = directive.args.at(0);
                    auto address = asm_tree.label_map.at(symbol_name);

                    if(exported_symbols.contains(symbol_name)) {
                        exported_symbols.at(symbol_name).type = symbol_type::Global;
                    } else {
                        exported_symbols.insert(
                                {symbol_name, symbol{symbol_name, sections.at(address.section), symbol_type::Global, std::make_optional(address.address)}});
                    }
                } else if (directive.name == "weak") {
                    auto symbol_name = directive.args.at(0);
                    auto address = asm_tree.label_map.at(symbol_name);

                    if(exported_symbols.contains(symbol_name)) {
                        exported_symbols.at(symbol_name).type = symbol_type::Global;
                    } else {
                        exported_symbols.insert(
                                {symbol_name, symbol{symbol_name, sections.at(address.section), symbol_type::Weak, std::make_optional(address.address)}});
                    }
                }  else if (directive.name == "extern") {
                    auto symbol_name = directive.args.at(0);

                    if(exported_symbols.contains(symbol_name)) {
                        exported_symbols.at(symbol_name).type = symbol_type::Extern;
                    } else {
                        exported_symbols.insert(
                                {symbol_name, symbol{symbol_name, sections.at(current_section), symbol_type::Extern, std::nullopt }});
                    }
                } else if (directive.name == "secinfo") {
                    if (!sections.contains(directive.args.at(0))) {
                        sections[directive.args.at(0)] = section{};
                    }

                    auto &edited_section = sections[directive.args.at(0)];
                    edited_section.flags.clear();

                    for (auto iter = ++std::begin(directive.args); iter != std::end(directive.args); ++iter) {
                        auto value = *iter;
                        if (value == "execute") {
                            edited_section.flags.insert(section_flags::X);
                        } else if (value == "read") {
                            edited_section.flags.insert(section_flags::R);
                        } else if (value == "write") {
                            edited_section.flags.insert(section_flags::W);
                        }
                    }
                } else if (directive.name == "byte") {
                    auto &current_section_data = sections.at(current_section).data;
                    current_section_data.push_back(std::stoi(directive.args.at(0)));
                } else if (directive.name == "word") {
                    auto &current_section_data = sections.at(current_section).data;
                    int16_t num = std::stoi(directive.args.at(0));
                    current_section_data.push_back(num >> 8);
                    current_section_data.push_back(num);
                } else if (directive.name == "bytes") {
                    auto &current_section_data = sections.at(current_section).data;
                    for (auto const &b : directive.args) {
                        current_section_data.push_back(std::stoi(b));
                    }
                }
            }
                break;
        }
    }

    std::transform(asm_tree.label_map.begin(), asm_tree.label_map.end(),
                   std::inserter(exported_symbols, exported_symbols.end()),
                   [&](std::pair<std::string, AsmTree::AsmTree::label> sym) {
                       return std::pair{sym.first,
                                        symbol{sym.first, sections.at(sym.second.section), symbol_type::Static, std::make_optional(sym.second.address)}};
                   });


    emit_elf_file(entry, sections);
}

void AsmTreeEmitter::emit_elf_file(size_t entrypoint, std::map<std::string, section> const &sections) {
    ELFIO::elfio emitter;
    emitter.create(ELFCLASS64, ELFDATA2MSB);
    emitter.set_os_abi(ELFOSABI_NONE);
    emitter.set_type(ET_EXEC);
    emitter.set_machine(EM_NONE);

    for (auto &section : sections) {
        auto section_name = section.first;
        auto const &section_data = section.second;

        ELFIO::section *current_section = emitter.sections.add(section_name);
        current_section->set_type(SHT_PROGBITS);
        ELFIO::Elf_Word flags = SHF_ALLOC;
        for (auto flag: section_data.flags) {
            switch (flag) {
                case section_flags::W:
                    flags |= SHF_WRITE;
                    break;
                case section_flags::X:
                    flags |= SHF_EXECINSTR;
                    break;
                default:
                    break;
            }
        }

        current_section->set_flags(flags);
        current_section->set_data(std::bit_cast<const char *>(section_data.data.data()),
                                  static_cast<ELFIO::Elf_Word>(section_data.data.size()));

        ELFIO::segment *current_segment = emitter.segments.add();
        current_segment->set_type(PT_LOAD);
        current_segment->set_virtual_address(section_data.addr);
        current_segment->set_physical_address(section_data.addr);


        flags = 0;
        for (auto flag: section_data.flags) {
            switch (flag) {
                case section_flags::R:
                    flags |= PF_R;
                    break;
                case section_flags::W:
                    flags |= PF_W;
                    break;
                case section_flags::X:
                    flags |= PF_X;
                    break;
            }
        }

        current_segment->set_flags(flags);
        current_segment->add_section(current_section, current_section->get_addr_align());
    }

    create_elf_symtab(emitter, exported_symbols);

    emitter.set_entry(entrypoint);

    std::cout << std::filesystem::current_path() << '\n';
    auto error = emitter.validate();
    if (!error.empty()) {
        throw std::runtime_error(error);
    }

    if (!output_stream) throw std::runtime_error("Invalid output stream");
    if (!emitter.save(output_stream)) throw std::runtime_error("Unknown Error");
}

AsmTreeEmitter::AsmTreeEmitter(std::ostream &output_stream) : output_stream{output_stream} {

}

void AsmTreeEmitter::create_elf_symtab(ELFIO::elfio & elfio, const std::map<std::string, symbol> &symbols) {
    ELFIO::section* symtab = elfio.sections.add(".symtab");
    ELFIO::section* strtab = elfio.sections.add(".strtab");
    strtab->set_type(SHT_STRTAB);
    strtab->set_size(0);
    symtab->set_type(SHT_SYMTAB);
    symtab->set_size(0);
    symtab->set_link(strtab->get_index());


    ELFIO::string_section_accessor string_accessor( strtab );
    ELFIO::symbol_section_accessor symbol_accessor( elfio, symtab );


    for(auto const& symp : symbols) {
        auto const& sym = symp.second;

        size_t section_index = SHN_UNDEF;
        for (int i = 0; i < elfio.sections.size(); ++i) {
            if(elfio.sections[i]->get_name() == sym.symbol_section.name) {
                section_index = elfio.sections[i]->get_index();
                break;
            }
        }

        ELFIO::Elf64_Addr addr = sym.address.value_or(0);

        ELFIO::Elf_Word bind;
        switch (sym.type) {
            case symbol_type::Global:
                bind = STB_GLOBAL;
                break;
            case symbol_type::Extern:
                bind = STB_GLOBAL;
                break;
            case symbol_type::Weak:
                bind = STB_WEAK;
                break;
            case symbol_type::Static:
                bind = STB_GLOBAL;
                break;
        }

        ELFIO::Elf_Word type = STT_FUNC;
        if(!sym.symbol_section.flags.contains(section_flags::X)) {
            type = STT_OBJECT;
        }

        ELFIO::Elf_Word visibility = STV_DEFAULT;
        switch (sym.type) {
            case symbol_type::Static:
                visibility = STV_HIDDEN;
                break;
            default:
                break;
        }


        auto name = string_accessor.add_string(sym.name);

        symbol_accessor.add_symbol(name,
                                   addr,
                                   0,
                                   bind,
                                   type,
                                   visibility,
                                   section_index
                                   );

    }
}
