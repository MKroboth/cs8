//
// Created by mkr on 7/28/21.
//

#include <optional>
#include <fstream>
#include "CS8_Assembler.hxx"
#include "Ast.h"
#include "cs8_parser.h"
#include "MacroExpander.h"
#include "AsmTreeTransformer.h"
#include "asm_tree_emitter.hxx"

std::optional<AstRootNode> invoke_parse(std::filesystem::path const& filename) {
    FILE* file = nullptr;

    auto current = std::filesystem::current_path();

    std::filesystem::current_path(filename.parent_path().c_str());

    try {
        file = fopen(filename.c_str(), "r");
        auto ast = parse(filename.filename(), file);
        if(file) {
            fclose(file);
            file = nullptr;
        }

        std::filesystem::current_path(current);
        return std::make_optional(std::move(ast));
    } catch(std::exception const& ex) {
        std::cerr << ex.what() << '\n';
        if(file) {
            fclose(file);
        }

        std::filesystem::current_path(current);
        return std::nullopt;
    }

}
void cs8_assembler::assemble(const std::filesystem::path &output,
                             const std::filesystem::path &input) {
    if(auto ast = invoke_parse(input); ast.has_value()) {
        MacroExpander macro_expander;
        macro_expander.expand_macros(*ast);

        AsmTreeTransformer trans;
        auto asm_tree = trans.transform(*ast);
        for(auto const& entry : asm_tree.label_map) {
            std::cout << entry.first << std::hex << ": " << entry.second.address << std::dec << '\n';
        }

        for(auto const& node : asm_tree.nodes) {
            node->to_ostream(std::cout);
        }

        std::ofstream output_stream(output, std::ios::out | std::ios::binary);

        if(!output_stream) throw std::runtime_error("Bad output stream");
        AsmTreeEmitter emitter(output_stream);
        emitter.emit_binary(asm_tree);
    }
}



void cs8_assembler::assemble(std::ostream &output, std::istream &input) {
    // TODO
}
