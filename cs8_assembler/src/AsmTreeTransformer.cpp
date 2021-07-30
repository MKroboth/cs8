//
// Created by mkr on 25.07.21.
//

#include "AsmTreeTransformer.h"
#include <limits>
#include <ranges>

AsmTree::AsmTree AsmTreeTransformer::transform(const AstRootNode &ast) {
    AsmTree::AsmTree result;

    label_scan(ast.get_lines());
    translate_lines(result.nodes, ast.get_lines());
    number_labels(result.nodes);

    result.label_map = this->m_label_map;
    return result;
}

void AsmTreeTransformer::label_scan(AsmTreeTransformer::ast_line_nodes const &t_lines) {
    auto is_label = [](ast_line_node const &x) { return x->get_type() == AstNodeType::Label; };
    auto get_label_name = [](ast_line_node const &x) { return dynamic_cast<AstLabel *>(x.get())->get_name(); };

    auto label_nodes = std::views::filter(t_lines, is_label);

    m_labels.clear();

    std::transform(label_nodes.begin(), label_nodes.end(),
                   std::inserter(m_labels, m_labels.end()),
                   get_label_name);
}

void AsmTreeTransformer::translate_lines(
        std::vector<std::unique_ptr<AsmTree::AsmTreeNode>> &t_tree_nodes,
        std::list<std::unique_ptr<AstLineNode>> const &t_lines) {
    auto translate_line_wrapper = [this](ast_line_node const &x) { return translate_line(*x); };

    std::transform(t_lines.begin(), t_lines.end(),
                   std::back_inserter(t_tree_nodes),
                   translate_line_wrapper);
}

std::unique_ptr<AsmTree::AsmTreeNode>
AsmTreeTransformer::translate_line(AstLineNode const &node) const {
    std::map<AstNodeType, std::function<AsmTree::AsmTreeNode *(AstLineNode const &)>> const translators{
            {AstNodeType::Instruction, [this](AstLineNode const &x) { return this->translate_instruction_node(x); }},
            {AstNodeType::Directive,   translate_directive_node},
            {AstNodeType::Label,       translate_label_node}
    };

    if (translators.contains(node.get_type())) {
        return std::unique_ptr<AsmTree::AsmTreeNode>(translators.at(node.get_type())(node));
    } else {
        throw std::runtime_error("Invalid node type");
    }
}

AsmTree::AsmTreeNode *
AsmTreeTransformer::translate_instruction_node(AstLineNode const &node) const {
    return decode_instruction(dynamic_cast<AstInstruction const &>(node));
}


namespace translate_instruction {
    using labels = AsmTreeTransformer::labels;
    using instruction_node = AsmTreeTransformer::asmtree_instruction_node;

    /**
     * \brief Check the given instruction for correct parameter size.
     * \param instruction a instruction
     * \param instruction_name the instructions mnemonic
     * \param required_parameters the number of parameters required.
     * \throws InvalidInstructionParameterCountError when the parameter size requirement is violated.
     */
    inline void require_instruction_parameter_size(AstInstruction const& instruction,
                                                             std::string_view instruction_name,
                                                             size_t required_parameters) {
        size_t instruction_size = instruction.get_parameters().size();
        if (instruction_size != required_parameters) {
            throw InvalidInstructionParameterCountError(instruction_name, required_parameters, instruction_size);
        }
    }


    instruction_node *limm(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeLoadImmediateInstruction;

        require_instruction_parameter_size(instruction, "limm", 1);

        auto const &parameter0 = instruction.get_parameter(0);

        switch (parameter0->get_type()) {
            case AstNodeType::SymbolParameter: {
                auto const &parameter = dynamic_cast<AstSymbolParameter const &>(*parameter0);

                if (labels.contains(parameter.get_name())) {
                    ptr->label = std::make_optional(
                            parameter.get_name());
                } else
                    throw std::logic_error(
                            "given symbol is not a label");

            }
                break;

            case AstNodeType::NumberParameter: {
                auto const &parameter = dynamic_cast<AstNumberParameter const &>(*parameter0);
                ptr->immediate = std::bit_cast<int16_t>(
                        static_cast<uint16_t>(parameter.get_value()));
            }
                break;
            default:
                throw std::logic_error("Invalid parameter");
        }
        return ptr;
    }

    instruction_node *lmem(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeLoadDirectInstruction;

        require_instruction_parameter_size(instruction, "lmem", 1);

        auto const &parameter0 = instruction.get_parameter(0);

        switch (parameter0->get_type()) {
            case AstNodeType::SymbolParameter: {
                auto const &parameter = dynamic_cast<AstSymbolParameter const &>(*parameter0);

                if (labels.contains(parameter.get_name())) {
                    ptr->label = std::make_optional(
                            parameter.get_name());
                } else
                    throw std::logic_error(
                            "given symbol is not a label");

            }
                break;

            case AstNodeType::NumberParameter: {
                auto const &parameter = dynamic_cast<AstNumberParameter const &>(*parameter0);
                ptr->address = std::bit_cast<int16_t>(
                        static_cast<uint16_t>(parameter.get_value()));
            }
                break;
            default:
                throw std::logic_error("Invalid parameter");
        }
        return ptr;
    }

    instruction_node *smem(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeStoreDirectInstruction;

        require_instruction_parameter_size(instruction, "smem", 1);

        auto const &parameter0 = instruction.get_parameter(0);

        switch (parameter0->get_type()) {
            case AstNodeType::SymbolParameter: {
                auto const &parameter = dynamic_cast<AstSymbolParameter const &>(*parameter0);

                if (labels.contains(parameter.get_name())) {
                    ptr->label = std::make_optional(
                            parameter.get_name());
                } else
                    throw std::logic_error(
                            "given symbol is not a label");

            }
                break;

            case AstNodeType::NumberParameter: {
                auto const &parameter = dynamic_cast<AstNumberParameter const &>(*parameter0);
                ptr->address = std::bit_cast<int16_t>(
                        static_cast<uint16_t>(parameter.get_value()));
            }
                break;
            default:
                throw std::logic_error("Invalid parameter");
        }
        return ptr;
    }

    instruction_node *lidx(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeLoadIndexedInstruction;

        require_instruction_parameter_size(instruction, "lidx", 0);

        return ptr;
    }

    instruction_node *sidx(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeStoreIndexedInstruction;

        require_instruction_parameter_size(instruction, "sidx", 0);
        return ptr;
    }

    instruction_node *add(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeAddInstruction;

        require_instruction_parameter_size(instruction, "add", 0);
        return ptr;
    }

    instruction_node *sub(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeSubtractInstruction;

        require_instruction_parameter_size(instruction, "sub", 0);
        return ptr;
    }

    instruction_node *mul(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeMultiplyInstruction;

        require_instruction_parameter_size(instruction, "mul", 0);

        return ptr;
    }

    instruction_node *divmod(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeDivideModuloInstruction;

        require_instruction_parameter_size(instruction, "divmod", 0);

        return ptr;
    }

    instruction_node *psh0(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreePush0Instruction;

        require_instruction_parameter_size(instruction, "psh0", 1);

        auto const &parameter0 = instruction.get_parameter(0);
        switch (parameter0->get_type()) {
            case AstNodeType::RegisterParameter: {
                auto const &parameter = dynamic_cast<AstRegisterParameter const &>(*parameter0);
                ptr->source = AsmTree::register_from_name(
                        parameter.get_name());
            }
                break;

            default:
                throw std::logic_error("Invalid parameter");
        }
        return ptr;
    }

    instruction_node *psh1(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreePush1Instruction;

        require_instruction_parameter_size(instruction, "psh1", 1);

        auto const &parameter0 = instruction.get_parameter(0);
        switch (parameter0->get_type()) {
            case AstNodeType::RegisterParameter: {
                auto const &parameter = dynamic_cast<AstRegisterParameter const &>(*parameter0);
                ptr->source = AsmTree::register_from_name(
                        parameter.get_name());
            }
                break;

            default:
                throw std::logic_error("Invalid parameter");
        }
        return ptr;
    }

    instruction_node *pop0(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreePop0Instruction;

        require_instruction_parameter_size(instruction, "pop0", 1);

        auto const &parameter0 = instruction.get_parameter(0);
        switch (parameter0->get_type()) {
            case AstNodeType::RegisterParameter: {
                auto const &parameter = dynamic_cast<AstRegisterParameter const &>(*parameter0);
                ptr->source = AsmTree::register_from_name(
                        parameter.get_name());
            }
                break;

            default:
                throw std::logic_error("Invalid parameter");
        }
        return ptr;
    }

    instruction_node *pop1(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreePop1Instruction;

        require_instruction_parameter_size(instruction, "pop1", 1);

        auto const &parameter0 = instruction.get_parameter(0);
        switch (parameter0->get_type()) {
            case AstNodeType::RegisterParameter: {
                auto const &parameter = dynamic_cast<AstRegisterParameter const &>(*parameter0);
                ptr->source = AsmTree::register_from_name(
                        parameter.get_name());
            }
                break;

            default:
                throw std::logic_error("Invalid parameter");
        }
        return ptr;
    }

    instruction_node *nand(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeNandInstruction;

        require_instruction_parameter_size(instruction, "nand", 0);

        return ptr;
    }

    instruction_node *jle(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeJumpIfLessOrEqualInstruction;

        require_instruction_parameter_size(instruction, "jle", 0);

        return ptr;
    }

    instruction_node *jmp(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeJumpInstruction;

        require_instruction_parameter_size(instruction, "jmp", 0);

        return ptr;
    }

    instruction_node *rtm(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeRestoreTMPInstruction;

        require_instruction_parameter_size(instruction, "rtm", 0);

        return ptr;
    }

    instruction_node *tr(labels const& labels, AstInstruction const &instruction) {
        auto ptr = new AsmTree::Instruction::AsmTreeTransferRegisterInstruction;

        require_instruction_parameter_size(instruction, "tr", 2);

        auto const &parameter0 = instruction.get_parameter(0);
        auto const &parameter1 = instruction.get_parameter(1);

        switch (parameter0->get_type()) {
            case AstNodeType::RegisterParameter: {
                auto const &parameter = dynamic_cast<AstRegisterParameter const &>(*parameter0);
                ptr->source = AsmTree::register_from_name(
                        parameter.get_name());
            }
                break;

            default:
                throw std::logic_error("Invalid parameter");
        }

        switch (parameter1->get_type()) {
            case AstNodeType::RegisterParameter: {
                auto const &parameter = dynamic_cast<AstRegisterParameter const &>(*parameter1);
                ptr->target = AsmTree::register_from_name(
                        parameter.get_name());
            }
                break;

            default:
                throw std::logic_error("Invalid parameter");
        }

        return ptr;
    }
}

AsmTree::Instruction::AsmTreeInstructionNode *
AsmTreeTransformer::decode_instruction(AstInstruction const &instruction) const {
    using instruction_transformer =
    std::function<translate_instruction::instruction_node *(translate_instruction::labels const&,
                                                            AstInstruction const &)>;

    static std::map<std::string, instruction_transformer> const
            instruction_builders{
            {"limm",   translate_instruction::limm},
            {"lmem",   translate_instruction::lmem},
            {"smem",   translate_instruction::smem},
            {"lidx",   translate_instruction::lidx},
            {"sidx",   translate_instruction::sidx},
            {"add",    translate_instruction::add},
            {"sub",    translate_instruction::sub},
            {"mul",    translate_instruction::mul},
            {"divmod", translate_instruction::divmod},
            {"psh0",   translate_instruction::psh0},
            {"psh1",   translate_instruction::psh1},
            {"pop0",   translate_instruction::pop0},
            {"pop1",   translate_instruction::pop1},
            {"nand",   translate_instruction::nand},
            {"jle",    translate_instruction::jle},
            {"jmp",    translate_instruction::jmp},
            {"rtm",    translate_instruction::rtm},
            {"tr",     translate_instruction::tr},
    };

    if (instruction_builders.contains(instruction.get_name())) {
        instruction_transformer transformer = instruction_builders.at(instruction.get_name());
        return transformer(this->m_labels, instruction);
    } else throw UnknownInstructionError(instruction.get_name());

}

void AsmTreeTransformer::number_labels(
        std::vector<std::unique_ptr<AsmTree::AsmTreeNode>> &nodes) {

    std::unordered_map<section_name, size_t> sections = {{"flat", 0}};
    section_name current_section = "flat";
    size_t *position = &sections.at(current_section);

    std::unordered_map<std::string, size_t> positions;

    for (auto &node : nodes) {
        switch (node->get_type()) {

            case AsmTree::AsmTreeType::Label: {
                auto &lbl = dynamic_cast<AsmTree::AsmTreeLabel &>(*node);
                lbl.position = *position;
                lbl.section = current_section;
                positions[lbl.name] = *position;
            }
                break;
            case AsmTree::AsmTreeType::Instruction: {
                auto const &instruction = dynamic_cast<AsmTree::Instruction::AsmTreeInstructionNode const &>(*node);
                *position += instruction.get_length();
            }
                break;
            case AsmTree::AsmTreeType::Directive: {
                auto const &directive = dynamic_cast<AsmTree::AsmTreeDirective const &>(*node);

                if (directive.name == "section") {
                    auto section_name = directive.args.at(0);

                    if (!sections.contains(section_name)) {
                        sections[section_name] = std::stoull(
                                directive.args.at(1));
                    }

                    current_section = section_name;
                    position = &sections.at(current_section);

                } else if (directive.name == "skip") {
                    *position += std::stoull(directive.args.at(0));
                } else if (directive.name == "byte") {
                    *position += 1;
                } else if (directive.name == "word") {
                    *position += 2;
                } else if (directive.name == "bytes") {
                    *position += directive.args.size();
                }
            }
                break;
        }

        this->m_label_map = positions;
    }

    for (auto &node : nodes) {
        switch (node->get_type()) {
            case AsmTree::AsmTreeType::Instruction: {
                auto &instruction = dynamic_cast<AsmTree::Instruction::AsmTreeInstructionNode &>(*node);

                switch (instruction.get_instruction_type()) {

                    case AsmTree::Instruction::AsmTreeInstructionType::LoadImmediate: {
                        auto &instr = dynamic_cast<AsmTree::Instruction::AsmTreeLoadImmediateInstruction &>(instruction);

                        if (instr.label.has_value()) {
                            if (positions.contains(*instr.label)) {
                                auto v = std::bit_cast<int16_t>(
                                        (uint16_t) positions.at(*instr.label));
                                instr.immediate = v;
                            }
                        }
                    }
                        break;
                    case AsmTree::Instruction::AsmTreeInstructionType::LoadDirect: {
                        {
                            auto &instr = dynamic_cast<AsmTree::Instruction::AsmTreeLoadDirectInstruction &>(instruction);

                            if (instr.label.has_value()) {
                                if (positions.contains(*instr.label)) {
                                    instr.address = positions.at(*instr.label);
                                }
                            }
                        }
                    }
                        break;
                    case AsmTree::Instruction::AsmTreeInstructionType::StoreDirect: {
                        {
                            auto &instr = dynamic_cast<AsmTree::Instruction::AsmTreeStoreDirectInstruction &>(instruction);

                            if (instr.label.has_value()) {
                                if (positions.contains(*instr.label)) {
                                    instr.address = positions.at(*instr.label);
                                }
                            }
                        }
                    }
                        break;
                    default:
                        break;
                }

            }
                break;
            default:
                break;
        }
    }

}

AsmTree::AsmTreeNode *AsmTreeTransformer::translate_directive_node(const AstLineNode &node) {
    auto const &input = dynamic_cast<AstDirective const &>(node);

    auto *directive = new AsmTree::AsmTreeDirective;
    directive->name = input.get_name();

    for (auto const &x: input.get_parameters()) {
        switch (x->get_type()) {
            default:
                throw std::runtime_error("Invalid node type");

            case AstNodeType::SymbolParameter:
                directive->args.push_back(
                        dynamic_cast<AstSymbolParameter const &>(*x).get_name());
                break;

            case AstNodeType::RegisterParameter:
                directive->args.push_back(
                        dynamic_cast<AstRegisterParameter const &>(*x).get_name());
                break;

            case AstNodeType::NumberParameter:
                directive->args.push_back(std::to_string(
                        dynamic_cast<AstNumberParameter const &>(*x).get_value()));
                break;
            case AstNodeType::StringParameter: {
                auto const &string = dynamic_cast<AstStringParameter const &>(*x).get_name();
                for (char c : string) {
                    directive->args.push_back(std::to_string((int) c));
                }
                break;
            }
        }
    }
    return directive;
}

AsmTree::AsmTreeNode *AsmTreeTransformer::translate_label_node(const AstLineNode &node) {
    auto const &input = dynamic_cast<AstLabel const &>(node);
    auto *label = new AsmTree::AsmTreeLabel;
    label->name = input.get_name();
    label->position = std::nullopt;
    return label;
}

