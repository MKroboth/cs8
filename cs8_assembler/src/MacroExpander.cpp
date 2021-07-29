//
// Created by mkr on 25.07.21.
//

#include <cassert>
#include <memory>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "MacroExpander.h"

void MacroExpander::scan_macros(std::list<Macro> const &t_macros) {
    m_macros.clear();

    auto macro_to_name_value_pair = [](auto &macro) {
        return std::pair{macro.get_name(), &macro};
    };

    std::transform(t_macros.cbegin(), t_macros.cend(),
                   std::inserter(m_macros, m_macros.end()),
                   macro_to_name_value_pair);
}

size_t MacroExpander::replace_macro_lines(std::list<std::unique_ptr<AstLineNode>> &t_lines) {
    size_t replaced_macros = 0;
    for (auto iter = t_lines.begin(); iter != t_lines.end(); ++iter) {
        if ((*iter)->get_type() == AstNodeType::Instruction) {
            AstInstruction instruction(*dynamic_cast<AstInstruction *>((*iter).get()));
            auto const &name = instruction.get_name();

            if (this->m_macros.contains(name)) {
                Macro const &macro = *this->m_macros.at(name);

                // The macro usage must have the same number of parameters as the macro definition.
                assert(macro.get_args().size() == instruction.get_parameters().size());

                // Build a lookup for the ReplaceSymbol -> the corresponding macro usage parameter.
                std::map<std::string, AstParameterNode *> macro_params;
                for (int i = 0; i < instruction.get_parameters().size(); ++i) {
                    auto itr = macro.get_args().begin();
                    std::advance(itr, i);
                    macro_params[*itr] = instruction.get_parameter(i).get();
                }
                ++replaced_macros;

                /* Remove the macro invocation */ {
                    *iter = std::move(std::make_unique<AstRedactLine>());
                }

                std::list<std::unique_ptr<AstLineNode>> macro_lines;
                std::transform(macro.get_lines().begin(), macro.get_lines().end(), std::back_inserter(macro_lines),
                               [](auto const &x) { return std::unique_ptr<AstLineNode>(x->duplicate()); });


                for (auto const &macro_line: macro_lines) {
                    std::unique_ptr<AstLineNode> elem(macro_line->duplicate());

                    if (auto *target = dynamic_cast<AstInstruction *>(elem.get())) {
                        // If the current target contains ReplaceSymbol parameters, replace them.
                        for (auto &param : target->get_parameters()) {
                            if (param->get_type() == AstNodeType::ReplaceSymbolParameter) {
                                auto const &rsp = dynamic_cast<AstReplaceSymbolParameter const &>(*param);

                                param.reset(macro_params[rsp.get_name()]->duplicate());
                            }
                        }


                    }

                    t_lines.insert(iter++, std::move(elem));
                    --iter;
                }

            }
        }
    }
    return replaced_macros;
}

void MacroExpander::expand_macros(AstRootNode &t_ast_root_node) {
    scan_macros(t_ast_root_node.get_macros());
    while (replace_macro_lines(t_ast_root_node.get_lines()) > 0)
        /* replace macros until no macro invocations exist anymore */;
    cleanup(t_ast_root_node);
}

void MacroExpander::cleanup(AstRootNode &t_ast_root_node) {
    auto is_redact_line = [](auto const& line){ return line->get_type() == AstNodeType::Redact; };
    t_ast_root_node.get_lines().remove_if(is_redact_line);
}
