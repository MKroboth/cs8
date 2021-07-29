//
// Created by mkr on 25.07.21.
//

#ifndef CS8_MACROEXPANDER_H
#define CS8_MACROEXPANDER_H
#include "Ast.h"

class MacroExpander {
    std::unordered_map<std::string, Macro const*> macros;

    void scan_macros(std::list<Macro> const& macros);
    size_t replace_macro_lines(std::list<std::unique_ptr<AstLineNode>>& lines);
    static void cleanup(AstRootNode& node);
public:
    void expand_macros(AstRootNode& node);
};


#endif //CS8_MACROEXPANDER_H
