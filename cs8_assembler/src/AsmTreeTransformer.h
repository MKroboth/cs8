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
    /// All labels in the transformation.
    std::unordered_set<label_name> m_labels;

    /// An association between present labels and their addresses.
    std::unordered_map<label_name, address> m_label_map;

    /**
     * \brief Scan the given ast nodes for labels and insert them into m_labels.
     */
    void label_scan(ast_line_nodes const&);

    /**
     * \brief Translate the given ast line nodes into their corresponding asmtree nodes.
     */
    inline void translate_lines(std::vector<asmtree_node>& t_tree_nodes, ast_line_nodes const& t_lines) {
        auto translate_line_wrapper = [this](ast_line_node const &x) { return translate_line(*x); };

        std::transform(t_lines.begin(), t_lines.end(),
                       std::back_inserter(t_tree_nodes),
                       translate_line_wrapper);
    }

    /**
     * \brief Translate the given line node to the corresponding asmtree node.
     * \param t_line_node a ast line node
     * \return the resulting asmtree node
     */
    asmtree_node translate_line(AstLineNode const& t_line_node) const;

    /**
     * \brief Decode the given ast instruction to an asmtree instruction node
     * \param instruction the given ast instruction
     * \return a new pointer to the created asmtree instruction node
     */
    AsmTree::Instruction::AsmTreeInstructionNode* decode_instruction(AstInstruction const& instruction) const;


    /**
     * \brief Determine the addresses of the labels in m_labels and store them in m_label_map;
     * \param asmtree_nodes the asmtree nodes to scan for label addresses.
     */
    void number_labels(std::vector<std::unique_ptr<AsmTree::AsmTreeNode>> const& asmtree_nodes);

    /**
     * \brief Translate the given ast line node to an asmtree node.
     * \param node the ast line node to translate.
     * \return a new pointer to the created instruction node.
     */
    [[nodiscard]] inline AsmTree::AsmTreeNode* translate_instruction_node(AstInstruction const& node) const {
        return decode_instruction(node);
    }

    /**
     * \brief Translate the given ast line node to an asmtree node
     * \param node the ast line node representing a directive
     * \return a new pointer to the created instruction node.
     */
    [[nodiscard]] static AsmTree::AsmTreeNode* translate_directive_node(AstDirective const& node);

    /**
     * \brief Translate the given ast line node to an asmtree node
     * \param node the ast line node representing a label
     * \return a new pointer to the created instruction node.
     */
    [[nodiscard]] static AsmTree::AsmTreeNode* translate_label_node(AstLabel const& node);

public:
    [[nodiscard]]
    AsmTree::AsmTree transform(AstRootNode const&);

};


#endif //CS8_ASMTREETRANSFORMER_H
