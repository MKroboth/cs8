//
// Created by mkr on 25.07.21.
//

#include "AsmTreeTransformer.h"
#include <limits>

AsmTree::AsmTree AsmTreeTransformer::transform(const AstRootNode &ast) {
    AsmTree::AsmTree result;

    label_scan(ast.get_lines());
    translate_lines(result.nodes, ast.get_lines());
    number_labels(result.nodes);

    result.label_map = this->label_map;
    return result;
}

void AsmTreeTransformer::label_scan(
        const std::list<std::unique_ptr<AstLineNode>> &lines) {
    labels.clear();
    for (auto const &line: lines) {
        if (line->get_type() == AstNodeType::Label) {
            labels.insert(dynamic_cast<AstLabel *>(line.get())->get_name());
        }
    }
}

void AsmTreeTransformer::translate_lines(
        std::vector<std::unique_ptr<AsmTree::AsmTreeNode>> &nodes,
        std::list<std::unique_ptr<AstLineNode>> const &lines) {
    for (auto const &line : lines) {
        nodes.push_back(this->translate_line(*line));
    }
}

std::unique_ptr<AsmTree::AsmTreeNode>
AsmTreeTransformer::translate_line(AstLineNode const &node) const {
    std::unique_ptr<AsmTree::AsmTreeNode> result;
    switch (node.get_type()) {
        case AstNodeType::Instruction: {
            auto const &input = dynamic_cast<AstInstruction const &>(node);
            AsmTree::Instruction::AsmTreeInstructionNode *instruction;

            instruction = decode_instruction(input);

            result.reset(instruction);
        }
            break;
        case AstNodeType::Directive: {
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
            result.reset(directive);
        }
            break;
        case AstNodeType::Label: {
            auto const &input = dynamic_cast<AstLabel const &>(node);
            auto *label = new AsmTree::AsmTreeLabel;
            label->name = input.get_name();
            label->position = std::nullopt;
            result.reset(label);
        }
            break;
        default:
            throw std::runtime_error("Invalid node type");
    }
    return std::move(result);
}

AsmTree::Instruction::AsmTreeInstructionNode *
AsmTreeTransformer::decode_instruction(
        AstInstruction const &instruction) const {
    static std::map<std::string, std::function<AsmTree::Instruction::AsmTreeInstructionNode *(
            AstInstruction const &)>> const
            instruction_builders{
            {"limm",     [&labels = this->labels](
                    AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeLoadImmediateInstruction;
                if (instruction.get_parameters().size() != 1)
                    throw InvalidInstructionParameterCountError("limm", 1,
                                                                instruction.get_parameters().size());


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
            }},
            {"lmem",     [&labels = this->labels](
                    AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeLoadDirectInstruction;
                if (instruction.get_parameters().size() != 1)
                    throw InvalidInstructionParameterCountError("lmem", 1,
                                                                instruction.get_parameters().size());


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
            }},
            {"smem",     [&labels = this->labels](
                    AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeStoreDirectInstruction;
                if (instruction.get_parameters().size() != 1)
                    throw InvalidInstructionParameterCountError("smem", 1,
                                                                instruction.get_parameters().size());


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
            }},
            {"lidx",     [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeLoadIndexedInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("lidx", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"sidx",     [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeStoreIndexedInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("sidx", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"add",    [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeAddInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("add", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"sub",    [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeSubtractInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("sub", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"mul",    [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeMultiplyInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("mul", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"divmod", [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeDivideModuloInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("divmod", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"psh0",   [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreePush0Instruction;

                if (!instruction.get_parameters().size() != 1)
                    throw InvalidInstructionParameterCountError("psh0", 1,
                                                                instruction.get_parameters().size());
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
            }},
            {"psh1",   [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreePush1Instruction;

                if (!instruction.get_parameters().size() != 1)
                    throw InvalidInstructionParameterCountError("psh1", 1,
                                                                instruction.get_parameters().size());
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
            }},
            {"pop0",   [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreePop0Instruction;

                if (!instruction.get_parameters().size() != 1)
                    throw InvalidInstructionParameterCountError("pop0", 1,
                                                                instruction.get_parameters().size());
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
            }},
            {"pop1",   [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreePop1Instruction;

                if (!instruction.get_parameters().size() != 1)
                    throw InvalidInstructionParameterCountError("pop1", 1,
                                                                instruction.get_parameters().size());
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
            }},
            {"nand",   [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeNandInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("nand", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"jle",    [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeJumpIfLessOrEqualInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("jle", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"jmp",    [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeJumpInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("jmp", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"rtm",    [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeRestoreTMPInstruction;

                if (!instruction.get_parameters().empty())
                    throw InvalidInstructionParameterCountError("rtm", 0,
                                                                instruction.get_parameters().size());

                return ptr;
            }},
            {"tr",     [](AstInstruction const &instruction) {
                auto ptr = new AsmTree::Instruction::AsmTreeTransferRegisterInstruction;
                if (instruction.get_parameters().size() != 2)
                    throw InvalidInstructionParameterCountError("tr", 2,
                                                                instruction.get_parameters().size());


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
            }},
    };

    if (instruction_builders.contains(instruction.get_name())) {
        return instruction_builders.at(instruction.get_name())(instruction);
    } else throw UnknownInstructionError(instruction.get_name());

}

void AsmTreeTransformer::number_labels(
        std::vector<std::unique_ptr<AsmTree::AsmTreeNode>> &nodes) {

    std::unordered_map<std::string, size_t> sections = {{"flat", 0}};
    std::string current_section = "flat";
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

        this->label_map = positions;
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

