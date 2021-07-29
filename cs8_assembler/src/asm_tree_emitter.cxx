//
// Created by mkr on 7/27/21.
//

#include "asm_tree_emitter.hxx"

#include <sstream>
#include <elfio/elfio.hpp>
#include <filesystem>


void AsmTreeEmitter::emit_binary(const AsmTree::AsmTree & asm_tree) {
    std::map<std::string, section> sections;
    size_t entry = 0;
    std::string current_section = "flat";

    for(auto const& node : asm_tree.nodes) {
        switch (node->get_type()) {

            case AsmTree::AsmTreeType::Label:
            {
                auto const& label = dynamic_cast<AsmTree::AsmTreeLabel const&>(*node);
            }
            break;
            case AsmTree::AsmTreeType::Instruction:
            {
                auto const& instruction = dynamic_cast<AsmTree::Instruction::AsmTreeInstructionNode const&>(*node);

                auto& current_section_data = sections.at(current_section).data;

                if(auto const* b = dynamic_cast<AsmTree::Instruction::AsmTreeInstruction1BNode const*>(&instruction)) {
                    auto binary = b->emit();
                    current_section_data.push_back(binary[0]);
                } else if(auto const* b = dynamic_cast<AsmTree::Instruction::AsmTreeInstruction2BNode const*>(&instruction)) {
                    auto binary = b->emit();
                    current_section_data.push_back(binary[0]);
                    current_section_data.push_back(binary[1]);
                } else if(auto const* b = dynamic_cast<AsmTree::Instruction::AsmTreeInstruction3BNode const*>(&instruction)) {
                    auto binary = b->emit();
                    current_section_data.push_back(binary[0]);
                    current_section_data.push_back(binary[1]);
                    current_section_data.push_back(binary[2]);

                }
            }
            break;
            case AsmTree::AsmTreeType::Directive:
            {
                auto const& directive = dynamic_cast<AsmTree::AsmTreeDirective const&>(*node);

                if(directive.name == "entrypoint") {
                    entry = std::stoull(directive.args.at(0));
                } else if(directive.name == "section") {
                    current_section = directive.args.at(0);
                    if(!sections.contains(current_section)) {
                        sections[current_section] = section { std::stoull(directive.args.at(1) ), std::vector<uint8_t>() };
                    }
                } else if (directive.name == "byte") {
                    auto& current_section_data = sections.at(current_section).data;
                    current_section_data.push_back(std::stoi(directive.args.at(0)));
                } else if (directive.name == "word") {
                    auto& current_section_data = sections.at(current_section).data;
                    int16_t num = std::stoi(directive.args.at(0));
                    current_section_data.push_back(num >> 8);
                    current_section_data.push_back(num);
                } else if (directive.name == "bytes") {
                    auto& current_section_data = sections.at(current_section).data;
                    for (auto const& b : directive.args) {
                        current_section_data.push_back(std::stoi(b));
                    }
                }
            }
                break;
        }
    }

    emit_elf_file(entry, sections);
}

void AsmTreeEmitter::emit_elf_file(size_t entrypoint, std::map<std::string, section> const& sections) {
    ELFIO::elfio emitter;
    emitter.create(ELFCLASS64, ELFDATA2LSB);
    emitter.set_os_abi(ELFOSABI_NONE);
    emitter.set_type(ET_EXEC);
    emitter.set_machine(EM_NONE);

    for (auto& section : sections) {
        auto section_name = section.first;
        auto const& section_data = section.second;

        ELFIO::section* current_section = emitter.sections.add(section_name);
        current_section->set_type(SHT_PROGBITS);
        current_section->set_flags(SHF_ALLOC | SHF_WRITE | SHF_EXECINSTR);
        current_section->set_data(std::bit_cast<const char*>(section_data.data.data()), static_cast<ELFIO::Elf_Word>(section_data.data.size()));

        ELFIO::segment* current_segment = emitter.segments.add();
        current_segment->set_type(PT_LOAD);
        current_segment->set_virtual_address(section_data.addr);
        current_segment->set_physical_address(section_data.addr);
        current_segment->set_flags(PF_X | PF_R | PF_W);
        current_segment->add_section(current_section, current_section->get_addr_align());
    }

    emitter.set_entry(entrypoint);

    std::cout << std::filesystem::current_path() << '\n';
    auto error = emitter.validate();
    if(!error.empty()) {
        throw std::runtime_error(error);
    }

    if(!output_stream) throw std::runtime_error("Invalid output stream");
    if(!emitter.save(output_stream)) throw std::runtime_error("Unknown Error");
}

AsmTreeEmitter::AsmTreeEmitter(std::ostream& output_stream): output_stream{output_stream} {

}
