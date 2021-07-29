//
// Created by mkr on 25.07.21.
//

#ifndef CS8_ASMTREETRANSFORMER_H
#define CS8_ASMTREETRANSFORMER_H

#include <string>
#include <unordered_set>
#include "AsmTree.h"
#include "Ast.h"

using namespace std::literals;

class UnknownInstructionError : public std::exception {
    std::string error;
public:
    explicit UnknownInstructionError(std::string const& instruction): error{"Unknown Instruction: "s + instruction} {}

    [[nodiscard]] const char * what() const noexcept override {
        return error.c_str();
    }
};

class InvalidInstructionParameterCountError : public std::exception {
    std::string error;
public:
    explicit InvalidInstructionParameterCountError(std::string const& instruction, size_t expected, size_t given): error{"Instruction '"s + instruction + "' expects " + std::to_string(expected) + "parameters, but reveived " + std::to_string(given) + "parameters."} {}

    [[nodiscard]] const char * what() const noexcept override {
        return error.c_str();
    }
};

class AsmTreeTransformer {
    std::unordered_set<std::string> labels;
    std::unordered_map<std::string, size_t> label_map;

    void destring(std::vector<std::unique_ptr<AsmTree::AsmTreeNode>>& nodes);
    void label_scan(std::list<std::unique_ptr<AstLineNode>> const&);
    void translate_lines(std::vector<std::unique_ptr<AsmTree::AsmTreeNode>>& nodes, std::list<std::unique_ptr<AstLineNode>> const& list);
    std::unique_ptr<AsmTree::AsmTreeNode> translate_line(AstLineNode const& list) const;
    AsmTree::Instruction::AsmTreeInstructionNode *decode_instruction(const AstInstruction &instruction) const;
    void number_labels(std::vector<std::unique_ptr<AsmTree::AsmTreeNode>> &vector);

public:
    [[nodiscard]]
    AsmTree::AsmTree transform(AstRootNode const&);

};


#endif //CS8_ASMTREETRANSFORMER_H
