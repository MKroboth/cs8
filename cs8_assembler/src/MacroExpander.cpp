//
// Created by mkr on 25.07.21.
//

#include <cassert>
#include <memory>
#include <map>
#include "MacroExpander.h"

void MacroExpander::scan_macros(const std::list<Macro> &macros) {
    this->macros.clear();

    for(auto const& macro: macros) {
        this->macros[macro.get_name()] = &macro;
    }
}

size_t MacroExpander::replace_macro_lines(std::list<std::unique_ptr<AstLineNode>>& lines) {
    size_t replaced_macros = 0;
    for(auto iter = lines.begin(); iter != lines.end(); ++iter) {
        if ((*iter)->get_type() == AstNodeType::Instruction) {
            AstInstruction instruction(*dynamic_cast<AstInstruction*>((*iter).get()));
            auto const& name = instruction.get_name();

            if(this->macros.contains(name)) {
                Macro const& macro = *this->macros.at(name);


                assert(macro.get_args().size() == instruction.get_parameters().size());
                std::map<std::string, AstParameterNode*> macro_params;
                for(int i = 0; i < instruction.get_parameters().size(); ++i) {
                    auto itr = macro.get_args().begin();
                    std::advance(itr, i);
                    macro_params[*itr] = instruction.get_parameter(i).get();
                }
                ++replaced_macros;

                {
                    *iter = std::move(std::make_unique<AstRedactLine>());
                }



                std::list<std::unique_ptr<AstLineNode>> macro_lines;
                std::transform(macro.get_lines().begin(), macro.get_lines().end(), std::back_inserter(macro_lines),
                               [](auto const& x) { return std::unique_ptr<AstLineNode>(x->duplicate());});


                for(auto const& macro_line: macro_lines) {
                    std::unique_ptr<AstLineNode> elem(macro_line->duplicate());

                    if(auto* ins = dynamic_cast<AstInstruction*>(elem.get())) {
                        {


                            for(auto& param : ins->get_parameters()) {
                                if(param->get_type() == AstNodeType::ReplaceSymbolParameter) {
                                    auto const& rsp = dynamic_cast<AstReplaceSymbolParameter const&>(*param);

                                    param.reset(macro_params[rsp.get_name()]->duplicate());
                                }
                            }
                        }

                    }

                    lines.insert(iter++, std::move(elem));
                    --iter;
                }

            }
        }
    }
    return replaced_macros;
}

void MacroExpander::expand_macros(AstRootNode &node) {
    scan_macros(node.get_macros());
    while(replace_macro_lines(node.get_lines()) > 0) /* replace macros until no macros exist anymore */;
    cleanup(node);
}

void MacroExpander::cleanup(AstRootNode &node) {
    node.get_lines().remove_if([](std::unique_ptr<AstLineNode>& line) {return line->get_type() == AstNodeType::Redact;});
}
