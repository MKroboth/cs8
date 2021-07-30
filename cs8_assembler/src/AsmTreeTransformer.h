//
// Created by mkr on 25.07.21.
//

#ifndef CS8_ASMTREETRANSFORMER_H
#define CS8_ASMTREETRANSFORMER_H

#include <string>
#include <unordered_set>
#include "AsmTree.h"
#include "Ast.h"
#include <fmt/format.h>

using namespace std::literals;

class UnknownInstructionError : public std::exception {
    std::string error;
public:
    explicit UnknownInstructionError(std::string const& instruction): error{"Unknown Instruction: "s + instruction} {}

    [[nodiscard]] const char * what() const noexcept override {
        return error.c_str();
    }
};

class InvalidInstructionParameterCountError final : public std::exception {
    using expected_number = size_t;
    using given_number = size_t;

    static constexpr const char message_format[] = "Instruction '{}' expects {} parameters, but {} parameters are given.\n";

    [[nodiscard]] static inline std::string create_error_message(std::string_view instruction,
                                                                     size_t expected,
                                                                     size_t given) {
        return fmt::format(message_format, instruction, expected, given);
    }

    std::string const error;
public:
    [[nodiscard]]
    InvalidInstructionParameterCountError(std::string_view instruction, size_t expected, size_t given)
    : error{create_error_message(instruction, expected, given)}
    {}

    [[nodiscard]] const char * what() const noexcept override {
        return error.c_str();
    }
};

class AsmTreeTransformer {
public:
    using section_name = std::string;
    using label_name = std::string;
    using address = size_t;

    using ast_line_node = std::unique_ptr<AstLineNode>;
    using ast_line_nodes = std::list<ast_line_node>;

    using asmtree_node = std::unique_ptr<AsmTree::AsmTreeNode>;

    using asmtree_instruction_node = AsmTree::Instruction::AsmTreeInstructionNode;
    using labels = std::unordered_set<label_name>;
private:
    std::unordered_set<label_name> m_labels;
    std::unordered_map<label_name, address> m_label_map;

    void label_scan(ast_line_nodes const&);
    void translate_lines(std::vector<asmtree_node>& t_tree_nodes, ast_line_nodes const& t_lines);
    asmtree_node translate_line(AstLineNode const& list) const;

    AsmTree::Instruction::AsmTreeInstructionNode *decode_instruction(AstInstruction const& instruction) const;
    void number_labels(std::vector<std::unique_ptr<AsmTree::AsmTreeNode>> &vector);

    [[nodiscard]] AsmTree::AsmTreeNode* translate_instruction_node(AstLineNode const& node) const;
    [[nodiscard]] static AsmTree::AsmTreeNode* translate_directive_node(AstLineNode const& node) ;
    [[nodiscard]] static AsmTree::AsmTreeNode* translate_label_node(AstLineNode const& node) ;

public:
    [[nodiscard]]
    AsmTree::AsmTree transform(AstRootNode const&);

};


#endif //CS8_ASMTREETRANSFORMER_H
