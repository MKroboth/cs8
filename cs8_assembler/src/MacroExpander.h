//
// Created by mkr on 25.07.21.
//

#ifndef CS8_MACROEXPANDER_H
#define CS8_MACROEXPANDER_H
#include "Ast.h"
#include <unordered_map>
#include <concepts>

/**
 * \brief Provide methods for expanding macro invocations in an ast.
 * \author Maximilian Kroboth
 */
class MacroExpander final {
    ///! The macro lookup table.
    std::unordered_map<std::string, Macro const*> m_macros;

    /**
     * \brief Scan macros to a map for fast lookup.
     * \param t_macros a list of macros.
     */
    void scan_macros(std::list<Macro> const& t_macros);

    /**
     * \brief Replace macro invocations with the macros content.
     * @param t_lines all lines of the program
     * @return the number of replaced macro invocations
     */
    size_t replace_macro_lines(std::list<std::unique_ptr<AstLineNode>>& t_lines);

    /**
     * \brief Remove Redact line nodes from the ast root node.
     * @param t_ast_root_node the ast root node
     */
    static void cleanup(AstRootNode& t_ast_root_node);
public:

    /**
     * \brief Expand all macro invocations in the given ast
     * @param t_ast_root_node
     */
    void expand_macros(AstRootNode& t_ast_root_node);
};


#endif //CS8_MACROEXPANDER_H
