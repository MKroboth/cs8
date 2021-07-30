//
// Created by mkr on 25.07.21.
//

#ifndef CS8_ASMTREE_H
#define CS8_ASMTREE_H

#include <cstddef>
#include <vector>
#include <cstdint>
#include <string>
#include <array>
#include <optional>
#include <any>
#include <map>
#include <memory>
#include <unordered_map>

namespace AsmTree {
    enum class AsmTreeType {
        Label,
        Instruction,
        Directive
    };

    class AsmTreeNode {
    public:
        [[nodiscard]] virtual AsmTreeType get_type() const = 0;

        virtual void to_ostream(std::ostream &os) const = 0;
        virtual ~AsmTreeNode() = default;
    };

    class AsmTree {
    public:
        std::vector<std::unique_ptr<AsmTreeNode>> nodes;
        std::unordered_map<std::string, size_t> label_map;
    };

    class AsmTreeDirective final : public AsmTreeNode {
    public:
        std::string name;
        std::vector<std::string> args;
        std::map<std::string, std::any> data;
        [[nodiscard]] AsmTreeType get_type() const final {
            return AsmTreeType::Directive;
        }

        void to_ostream(std::ostream &os) const override {
            os << "Directive\t" << name << " ";
            for(auto const& arg: args) {
                os << arg << " ";
            }

            os << '\n';
        }
    };

    class AsmTreeLabel final : public AsmTreeNode {
    public:
        std::string name;
        std::string section;
        std::optional<size_t> position;

        [[nodiscard]] AsmTreeType get_type() const final {
            return AsmTreeType::Label;
        }

        void to_ostream(std::ostream &os) const override {
            os << "Label\t" << name << "\t@";

            if (position.has_value()) os << std::hex << *position << std::dec;
            else os << "?";

            os << " in " << section << '\n';
        }
    };

    namespace Instruction {
        enum class AsmTreeInstructionType {
            LoadImmediate,
            LoadDirect,
            StoreDirect,
            LoadIndexed,
            StoreIndexed,
            TransferRegister,
            Push0,
            Push1,
            Pop0,
            Pop1,
            Add,
            Subtract,
            Multiply,
            DivideModulo,
            Nand,
            JumpIfLessOrEqual, RestoreTMP, Jump, RotateRight, And, Or, ShiftRight, Invert, ShiftLeft, RotateLeft
        };

        class AsmTreeInstructionNode : public AsmTreeNode {
        public:
            [[nodiscard]] AsmTreeType get_type() const final { return AsmTreeType::Instruction; }
            [[nodiscard]] virtual AsmTreeInstructionType get_instruction_type() const = 0;

            [[nodiscard]] virtual std::vector<uint8_t> emit_binary() const = 0;

            [[nodiscard]] virtual size_t get_length() const = 0;
        };


        class AsmTreeInstruction1BNode : public AsmTreeInstructionNode {
        public:
            [[nodiscard]] size_t get_length() const final { return 1; }
            [[nodiscard]] virtual std::array<uint8_t, 1> emit() const = 0;
            [[nodiscard]] std::vector<uint8_t> emit_binary() const final {
                auto array = emit();
                std::vector<uint8_t> result(array.size());
                std::move(array.begin(), array.end(), std::back_inserter(result));
                return result;
            }
        };

        class AsmTreeInstruction2BNode : public AsmTreeInstructionNode {
        public:
            [[nodiscard]] size_t get_length() const final { return 2; }
            [[nodiscard]] virtual std::array<uint8_t, 2> emit() const = 0;
            [[nodiscard]] std::vector<uint8_t> emit_binary() const final {
                auto array = emit();
                std::vector<uint8_t> result(array.size());
                std::move(array.begin(), array.end(), std::back_inserter(result));
                return result;
            }
        };

        class AsmTreeInstruction3BNode : public AsmTreeInstructionNode {
        public:
            [[nodiscard]] size_t get_length() const final { return 3; }
            [[nodiscard]] virtual std::array<uint8_t, 3> emit() const = 0;
            [[nodiscard]] std::vector<uint8_t> emit_binary() const final {
                auto array = emit();
                std::vector<uint8_t> result(array.size());
                std::move(array.begin(), array.end(), std::back_inserter(result));
                return result;
            }
        };

        inline std::string instruction_type_to_string(AsmTreeInstructionType type) {
            switch (type) {

                case AsmTreeInstructionType::LoadImmediate:
                    return "Load Immediate";
                case AsmTreeInstructionType::LoadDirect:
                    return "Load Direct";
                case AsmTreeInstructionType::StoreDirect:
                    return "Store Direct";
                case AsmTreeInstructionType::LoadIndexed:
                    return "Load Indexed";
                case AsmTreeInstructionType::StoreIndexed:
                    return "Store Indexed";
                case AsmTreeInstructionType::TransferRegister:
                    return "Transfer Register";
                case AsmTreeInstructionType::Push0:
                    return "Push to Stack 0";
                case AsmTreeInstructionType::Push1:
                    return "Push to Stack 1";
                case AsmTreeInstructionType::Pop0:
                    return "Pop from Stack 0";
                case AsmTreeInstructionType::Pop1:
                    return "Pop from Stack 1";
                case AsmTreeInstructionType::Add:
                    return "Add";
                case AsmTreeInstructionType::Subtract:
                    return "Subtract";
                case AsmTreeInstructionType::Multiply:
                    return "Multiply";
                case AsmTreeInstructionType::DivideModulo:
                    return "Divide and Modulo";
                case AsmTreeInstructionType::Nand:
                    return "NAND";
                case AsmTreeInstructionType::JumpIfLessOrEqual:
                    return "Jump if Less of Equal";
                case AsmTreeInstructionType::Jump:
                    return "Jump";
                case AsmTreeInstructionType::RestoreTMP:
                    return "RestoreTMP";
                case AsmTreeInstructionType::RotateRight:
                    return "Rotate Right";
                case AsmTreeInstructionType::And:
                    return "And";
                case AsmTreeInstructionType::Or:
                    return "Or";
                case AsmTreeInstructionType::ShiftRight:
                    return "ShiftRight";
                case AsmTreeInstructionType::Invert:
                    return "Invert";
                case AsmTreeInstructionType::ShiftLeft:
                    return "ShiftLeft";
                case AsmTreeInstructionType::RotateLeft:
                    return "RotateLeft";
            }
            throw std::logic_error("illegal state");
        }

        class AsmTreeLoadImmediateInstruction final : public AsmTreeInstruction3BNode {
        public:
            std::optional<uint16_t> immediate{0};
            std::optional<std::string> label;

            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\t";

                if(label.has_value()) {
                    os << *label << "(" << std::hex;
                    if(immediate.has_value()) os << "0x" <<  *immediate;
                    else os << "?";
                    os <<std::dec << ")\n";
                } else {
                    os << std::hex;
                    if(immediate.has_value()) os << "0x"<< *immediate;
                    else os << "?";
                    os << std::dec << "\n";

                }
            }

            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const final {
                return AsmTreeInstructionType::LoadImmediate;
            }

            [[nodiscard]] std::array<uint8_t, 3> emit() const final {
                return {0x00,
                        (uint8_t)((std::bit_cast<uint16_t>(*immediate) & 0xFF00) >> 8),
                        (uint8_t)(((std::bit_cast<uint16_t>(*immediate)) & 0x00FF)),
                };
            }
        };
        class AsmTreeLoadDirectInstruction final : public AsmTreeInstruction3BNode {
        public:
            std::optional<uint16_t> address;
            std::optional<std::string> label;

            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\t";

                if(label.has_value()) {
                    os << *label << "(" << std::hex;
                    if(address.has_value())  os << "0x" << *address;
                    else os << "?";
                    os << std::dec << ")\n";
                } else {
                    os << std::hex;
                    if(address.has_value()) os << "0x" << *address;
                    else os << "?";
                    os  << std::dec << "\n";

                }
            }

            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::LoadDirect;
            }

            [[nodiscard]] std::array<uint8_t, 3> emit() const final {
                return {0x01,
                        (uint8_t)(((*address) & 0xFF00) >> 8),
                        (uint8_t)(((*address) & 0x00FF)),
                };
            }
        };
        class AsmTreeStoreDirectInstruction final : public AsmTreeInstruction3BNode {
        public:
            std::optional<int16_t> address{};
            std::optional<std::string> label;

            void to_ostream(std::ostream &os) const final {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\t";

                if(label.has_value()) {
                    os << *label << "(" << std::hex ;
                    if(address.has_value()) os << "0x" << *address;
                    else os << "?";
                    os << std::dec << ")\n";
                } else {
                    os << std::hex;
                    if(address.has_value()) os <<"0x" <<  *address;
                    else os << "?";
                    os << std::dec << "\n";

                }
            }

            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::StoreDirect;
            }

            [[nodiscard]] std::array<uint8_t, 3> emit() const final {
                return {0x02,
                        (uint8_t)(((*address) & 0xFF00) >> 8),
                        (uint8_t)(((*address) & 0x00FF)),
                };
            }
        };

        class AsmTreeLoadIndexedInstruction final : public AsmTreeInstruction1BNode {
        public:

            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }

            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::LoadIndexed;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return {0x03,};
            }
        };

        class AsmTreeStoreIndexedInstruction final : public AsmTreeInstruction1BNode {
        public:
            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }

            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::StoreIndexed;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return {0x04,};
            }
        };

        enum class AsmTreeRegister: uint8_t  {
            dst = 0,
            sc0 = 1,
            sc1 = 2,
            idx = 3,
            tmp = 4,
            sp0 = 5,
            sp1 = 6,
            dt0 = 7,
            dt1 = 8,
            dt2 = 9,
            dt3 = 0xA,
            dt4 = 0xB,
            dt5 = 0xC,
            lnk = 0xD,
            cnt = 0xE,
            bse = 0xF,
        };

        inline std::string asm_tree_register_to_string(AsmTreeRegister reg) {
            switch (reg) {

                case AsmTreeRegister::dst:
                    return "dst";
                case AsmTreeRegister::sc0:
                    return "sc0";
                case AsmTreeRegister::sc1:
                    return "sc1";
                case AsmTreeRegister::idx:
                    return "idx";
                case AsmTreeRegister::tmp:
                    return "tmp";
                case AsmTreeRegister::sp0:
                    return "sp0";
                case AsmTreeRegister::sp1:
                    return "sp1";
                case AsmTreeRegister::dt0:
                    return "dt0";
                case AsmTreeRegister::dt1:
                    return "dt1";
                case AsmTreeRegister::dt2:
                    return "dt2";
                case AsmTreeRegister::dt3:
                    return "dt3";
                case AsmTreeRegister::dt4:
                    return "dt4";
                case AsmTreeRegister::dt5:
                    return "dt5";
                case AsmTreeRegister::lnk:
                    return "lnk";
                case AsmTreeRegister::cnt:
                    return "cnt";
                case AsmTreeRegister::bse:
                    return "bse";
            }
            throw std::logic_error("illegal state");
        }

        class AsmTreeTransferRegisterInstruction final : public AsmTreeInstruction2BNode {
        public:
            AsmTreeRegister source{AsmTreeRegister::bse};
            AsmTreeRegister target{AsmTreeRegister::bse};

            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type())
                << " " << asm_tree_register_to_string(source)
                        << " -> " << asm_tree_register_to_string(target) << '\n';

            }

            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::TransferRegister;
            }

            [[nodiscard]] std::array<uint8_t, 2> emit() const final {
                return{static_cast<uint8_t>(0x05 | (static_cast<uint8_t>(source) << 4)), static_cast<uint8_t>(target)};
            }
        };

        class AsmTreePush0Instruction final : public AsmTreeInstruction1BNode {
        public:
            AsmTreeRegister source{AsmTreeRegister::bse};

            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type())
                   << " " << asm_tree_register_to_string(source)  << '\n';

            }

            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Push0;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{static_cast<uint8_t>(0x06 | (static_cast<uint8_t>(source) << 4))};
            }
        };

        class AsmTreePush1Instruction final : public AsmTreeInstruction1BNode {
        public:
            AsmTreeRegister source{AsmTreeRegister::bse};
            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type())
                   << " " << asm_tree_register_to_string(source)  << '\n';

            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Push1;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{static_cast<uint8_t>(0x07 | (static_cast<uint8_t>(source) << 4))};
            }
        };

        class AsmTreePop0Instruction final : public AsmTreeInstruction1BNode {
        public:
            AsmTreeRegister source{AsmTreeRegister::bse};
            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type())
                   << " " << asm_tree_register_to_string(source)  << '\n';

            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Pop0;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{static_cast<uint8_t>(0x08 | (static_cast<uint8_t>(source) << 4))};
            }
        };

        class AsmTreePop1Instruction final : public AsmTreeInstruction1BNode {
        public:
            AsmTreeRegister source{AsmTreeRegister::bse};
            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type())
                   << " " << asm_tree_register_to_string(source)  << '\n';

            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Pop1;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{static_cast<uint8_t>(0x09 | (static_cast<uint8_t>(source) << 4))};
            }
        };

        class AsmTreeAddInstruction final : public AsmTreeInstruction1BNode {
        public:
            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Add;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x0A};
            }
        };

        class AsmTreeSubtractInstruction final : public AsmTreeInstruction1BNode {
        public:
            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Subtract;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x0B};
            }
        };
        class AsmTreeMultiplyInstruction final : public AsmTreeInstruction1BNode {
        public:
            void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Multiply;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x0C};
            }
        };
        class AsmTreeDivideModuloInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::DivideModulo;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x0D};
            }
        };
        class AsmTreeNandInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Nand;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x0E};
            }
        };
        class AsmTreeOrInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Or;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x1E};
            }
        };
        class AsmTreeAndInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::And;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x2E};
            }
        };
        class AsmTreeInvertInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::Invert;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x3E};
            }
        };
        class AsmTreeShiftLeftInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::ShiftLeft;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x4E};
            }
        };
        class AsmTreeShiftRightInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::ShiftRight;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x5E};
            }
        };
        class AsmTreeRotateLeftInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::RotateLeft;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x6E};
            }
        };
        class AsmTreeRotateRightInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
                os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
            }
            [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
                return AsmTreeInstructionType::RotateRight;
            }

            [[nodiscard]] std::array<uint8_t, 1> emit() const final {
                return{0x7E};
            }
        };

        class AsmTreeJumpIfLessOrEqualInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
            os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
        }
        [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
            return AsmTreeInstructionType::JumpIfLessOrEqual;
        }

        [[nodiscard]] std::array<uint8_t, 1> emit() const final {
            return{0x0F};
        }
        };
        class AsmTreeJumpInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
            os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
        }
        [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
            return AsmTreeInstructionType::Jump;
        }

        [[nodiscard]] std::array<uint8_t, 1> emit() const final {
            return{0x1F};
        }
        };
        class AsmTreeRestoreTMPInstruction final : public AsmTreeInstruction1BNode {
        public:void to_ostream(std::ostream &os) const override {
            os << "Instruction\t" << instruction_type_to_string(get_instruction_type()) << "\n";
        }
        [[nodiscard]] AsmTreeInstructionType get_instruction_type() const override {
            return AsmTreeInstructionType::RestoreTMP;
        }

        [[nodiscard]] std::array<uint8_t, 1> emit() const final {
            return{0x2F};
        }
        };
    }


    inline Instruction::AsmTreeRegister register_from_name(const std::string &name) {
        static std::map<std::string, Instruction::AsmTreeRegister> const registers = {
                { "dst", Instruction::AsmTreeRegister::dst },
                { "sc0", Instruction::AsmTreeRegister::sc0 },
                { "sc1", Instruction::AsmTreeRegister::sc1 },
                { "idx", Instruction::AsmTreeRegister::idx },
                { "tmp", Instruction::AsmTreeRegister::tmp },
                { "sp0", Instruction::AsmTreeRegister::sp0 },
                { "sp1", Instruction::AsmTreeRegister::sp1 },
                { "dt0", Instruction::AsmTreeRegister::dt0 },
                { "dt1", Instruction::AsmTreeRegister::dt1 },
                { "dt2", Instruction::AsmTreeRegister::dt2 },
                { "dt3", Instruction::AsmTreeRegister::dt3 },
                { "dt4", Instruction::AsmTreeRegister::dt4 },
                { "dt5", Instruction::AsmTreeRegister::dt5 },
                { "lnk", Instruction::AsmTreeRegister::lnk },
                { "cnt", Instruction::AsmTreeRegister::cnt },
                { "bse", Instruction::AsmTreeRegister::bse },
        };

        if(registers.contains(name)) {
            return registers.at(name);
        } else throw std::logic_error("Unknown register name");
    }
}
#endif //CS8_ASMTREE_H
