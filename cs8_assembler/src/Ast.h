//
// Created by mkr on 25.07.21.
//

#ifndef CS8_AST_H
#define CS8_AST_H

#include <list>
#include <memory>
#include <ostream>
#include <cstring>
#include <functional>
#include <cassert>

enum class AstNodeType {
    Root,
    Instruction,
    Directive,
    Redact, SymbolParameter, RegisterParameter, NumberParameter, ReplaceSymbolParameter, Label,
    StringParameter
};

class AstNode {
public:
    [[nodiscard]]
    virtual AstNodeType get_type() const noexcept = 0;
    virtual ~AstNode() = default;

    friend std::ostream &operator<<(std::ostream &os, const AstNode &node);

    virtual void write_to_ostream(std::ostream &ostream, const AstNode &node) const = 0;
    virtual void print_repr(std::ostream &ostream) = 0;
};

class AstParameterNode : public AstNode {
public:
    virtual AstParameterNode* duplicate() const = 0;
};



class AstLineNode : public AstNode {
public:
    virtual AstLineNode* duplicate() const = 0;
};
class Macro {
    std::string name;
    std::list<std::string> args;
    std::list<std::unique_ptr<AstLineNode>> lines;
public:
    explicit Macro(std::string_view name): name{name} {}

    friend std::ostream &operator<<(std::ostream &os, const Macro &macro) {
        os << "Macro[name: " << macro.name << " args:{ ";
        for(auto const& arg: macro.args) {
            os << arg << " ";
        }
        os << "} lines = {";
        for(auto const& line: macro.lines) {
            os << *line << "\n";
        }
        os << "}]";
        return os;
    }

    [[nodiscard]] std::string const& get_name() const { return name; }
    [[nodiscard]] const std::list<std::unique_ptr<AstLineNode>>& get_lines() const {
        return lines;
    }

    [[nodiscard]] const std::list<std::string>& get_args() const {
        return args;
    }
    void add_arg(std::string const& argument) {
        args.push_back(argument);
    }

    void add_line(AstLineNode const* node) {
        if (node->get_type() != AstNodeType::Redact) {
            lines.push_back(std::unique_ptr<AstLineNode>{node->duplicate()});
        }
    }
};

class AstRootNode final : public AstNode {
    std::string filename;
    std::list<std::unique_ptr<AstLineNode>> lines;
    std::list<Macro> macros;

public:
    void print_repr(std::ostream &ostream) override {
        for (auto const& line: lines) {
            line->print_repr(ostream);
        }
    }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::Root;
    }

    explicit AstRootNode(std::string_view filename): filename{filename} {
    }

    [[nodiscard]] std::string const& get_filename() const { return filename; }
    [[nodiscard]] const std::list<std::unique_ptr<AstLineNode>>& get_lines() const {
        return lines;
    }

    [[nodiscard]] std::list<std::unique_ptr<AstLineNode>>& get_lines() {
        return lines;
    }

    [[nodiscard]] const std::list<Macro>& get_macros() const {
        return macros;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "File[name='" << filename << "' lines={\n";
        for(auto const& line: lines) {
            ostream << *line << "\n";
        }
        ostream << "} macros={";
        for(auto const& macro: macros) {
            ostream << macro << "\n";
        }
        ostream << "}]";
    }
    void add_line(AstLineNode const* node) {
        if(node->get_type() != AstNodeType::Redact) {
            lines.push_back(std::unique_ptr<AstLineNode>{node->duplicate()});
        }
    }


    void add_macro(Macro macro) {
        macros.push_front(std::move(macro));
    }
};

class AstInstruction final : public AstLineNode {
    std::string name;
    std::list<std::unique_ptr<AstParameterNode>> parameters;

public:
    void print_repr(std::ostream &ostream) override {
        ostream << name << " ";
        for(auto iter = parameters.begin(); iter != parameters.end(); ++iter) {
            (*iter)->print_repr(ostream);
            if(++iter != parameters.end()) {
                ostream << ", ";
            }
            --iter;
        }
        ostream << "\n";
    }

    [[nodiscard]] std::string const& get_name() const { return this->name; }
    [[nodiscard]] std::list<std::unique_ptr<AstParameterNode>> const& get_parameters() const { return this->parameters; }
    [[nodiscard]] std::unique_ptr<AstParameterNode> const& get_parameter(size_t i) const {
        auto iter = get_parameters().begin();
        std::advance(iter, i);
        return *iter;
    }

    [[nodiscard]] std::list<std::unique_ptr<AstParameterNode>>& get_parameters() { return this->parameters; }
    [[nodiscard]] std::unique_ptr<AstParameterNode>& get_parameter(size_t i) {
        auto iter = get_parameters().begin();
        std::advance(iter, i);
        return *iter;
    }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::Instruction;
    }

    explicit AstInstruction(std::string_view name): name{name} {}
    AstInstruction(AstInstruction const& other): name{other.name} {
        for (auto const& param : other.parameters) {
            this->parameters.push_back(std::unique_ptr<AstParameterNode>(param->duplicate()));
        }
    }

    [[nodiscard]] AstLineNode *duplicate() const override {
        auto* instruction = new AstInstruction(*this);
        return instruction;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "Instruction[name='" << name << "'";
        if (!parameters.empty()) {
            ostream << " params={";
            const char *append = "";
            if (parameters.size() > 1) append = "\n";
            ostream << append;
            for (auto const &param: parameters) {
                ostream << *param << append;
            }
            ostream << "}";
        }

        ostream << "]";
    }

    void add_parameter(AstParameterNode const* node) {
        parameters.push_back(std::unique_ptr<AstParameterNode>{node->duplicate()});
    }
};

class AstRedactLine final : public AstLineNode {
public:
    void print_repr(std::ostream &ostream) override {
        ostream << ";; unknown \n";
    }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::Redact;
    }

    explicit AstRedactLine() = default;

    [[nodiscard]] AstLineNode *duplicate() const override {
        return new AstRedactLine;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "{Redacted Line}";
    }
};


class AstDirective final : public AstLineNode {
    std::string name;
    std::list<std::unique_ptr<AstParameterNode>> parameters;

public:

    [[nodiscard]] std::string const& get_name() const { return this->name; }
    [[nodiscard]] std::list<std::unique_ptr<AstParameterNode>> const& get_parameters() const { return this->parameters; }
    [[nodiscard]] std::unique_ptr<AstParameterNode> const& get_parameter(size_t i) const {
        auto iter = get_parameters().begin();
        std::advance(iter, i);
        return *iter;
    }

    [[nodiscard]] std::list<std::unique_ptr<AstParameterNode>>& get_parameters() { return this->parameters; }
    [[nodiscard]] std::unique_ptr<AstParameterNode>& get_parameter(size_t i) {
        auto iter = get_parameters().begin();
        std::advance(iter, i);
        return *iter;
    }


    void print_repr(std::ostream &ostream) override {
        ostream << "." << name << " ";
        for(auto iter = parameters.begin(); iter != parameters.end(); ++iter) {
            (*iter)->print_repr(ostream);
            if(++iter != parameters.end()) {
                ostream << ", ";
            }
            --iter;
        }
        ostream << "\n";
    }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::Directive;
    }

    explicit AstDirective(std::string_view name): name{name} {}
    AstDirective(AstDirective const& other): name{other.name} {
        for (auto const& param : other.parameters) {
            this->parameters.push_back(std::unique_ptr<AstParameterNode>(param->duplicate()));
        }
    }

    [[nodiscard]] AstLineNode *duplicate() const override {
        auto* directive = new AstDirective(*this);
        return directive;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "Directive[name='" << name << "'";
        if (!parameters.empty()) {
            ostream << " params={ ";
            const char *append = "";
            if (parameters.size() > 1) append = "\n";
            ostream << append;
            for (auto const &param: parameters) {
                ostream << *param << append;
            }
            ostream << "}";
        }

        ostream << "]";
    }
    void add_parameter(AstParameterNode const* node) {
        parameters.push_back(std::unique_ptr<AstParameterNode>{node->duplicate()});
    }
};


class AstRegisterParameter final : public AstParameterNode {
    std::string name;
public:
    void print_repr(std::ostream &ostream) override {
        ostream << "%" << name;
    }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::RegisterParameter;
    }

    explicit AstRegisterParameter(std::string_view name): name{name} {}
    AstRegisterParameter(AstRegisterParameter const& other): name{other.name} {}

    [[nodiscard]] AstParameterNode* duplicate() const override {
        auto* result = new AstRegisterParameter(*this);
        return result;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "RegisterParameter[name='" << name << "']";
    }

    std::string const& get_name() const {
        return name;
    }
};

class AstNumberParameter final : public AstParameterNode {
    int value;
public:
    void print_repr(std::ostream &ostream) override {
        if(value > 30) {
            ostream << std::hex << "0x";
        }

        ostream << value;

        ostream << std::dec;
    }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::NumberParameter;
    }

    explicit AstNumberParameter(int value): value{value} {}
    AstNumberParameter(AstNumberParameter const& other): value{other.value} {}

    [[nodiscard]] AstParameterNode* duplicate() const override {
        auto* result = new AstNumberParameter(*this);
        return result;
    }

    int get_value() const {
        return value;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "NumberParameter[" << value << "]";
    }
};

class AstSymbolParameter final : public AstParameterNode {
    std::string name;
public:
    void print_repr(std::ostream &ostream) override {
        ostream << name;
    }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::SymbolParameter;
    }

    explicit AstSymbolParameter(std::string_view name): name{name} {}
    AstSymbolParameter(AstSymbolParameter const& other): name{other.name} {}

    [[nodiscard]] AstParameterNode* duplicate() const override {
        auto* result = new AstSymbolParameter(*this);
        return result;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "SymbolParameter[name='" << name << "']";
    }

    std::string const& get_name() const {
        return name;
    }
};

class AstReplaceSymbolParameter final : public AstParameterNode {
    std::string name;
public:
    void print_repr(std::ostream &ostream) override {
        ostream << "\\" << name;
    }

    [[nodiscard]] std::string const& get_name() const { return name; }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::ReplaceSymbolParameter;
    }

    explicit AstReplaceSymbolParameter(std::string_view name): name{name} {}
    AstReplaceSymbolParameter(AstReplaceSymbolParameter const& other): name{other.name} {}

    [[nodiscard]] AstParameterNode* duplicate() const override {
        auto* result = new AstReplaceSymbolParameter(*this);
        return result;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "ReplaceSymbolParameter[name='" << name << "']";
    }
};

class AstStringParameter final : public AstParameterNode {
    std::string name;
public:
    void print_repr(std::ostream &ostream) override {
        ostream << '"' << name << '"';
    }

    [[nodiscard]] std::string const& get_name() const { return name; }

    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::StringParameter;
    }

    explicit AstStringParameter(std::string_view name): name{name} {}
    AstStringParameter(AstStringParameter const& other): name{other.name} {}

    [[nodiscard]] AstParameterNode* duplicate() const override {
        auto* result = new AstStringParameter(*this);
        return result;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "StringParameter[content=\"" << name << "\"]";
    }
};
class AstLabel final : public AstLineNode {
    std::string name;
public:
    void print_repr(std::ostream &ostream) override {
        ostream << name << ":\n";
    }

    [[nodiscard]] std::string const& get_name() const {
        return this->name;
    }


    [[nodiscard]] AstNodeType get_type() const noexcept override {
        return AstNodeType::Label;
    }

    explicit AstLabel(std::string_view name): name{name} {}
    AstLabel(AstLabel const& other): name{other.name} {}

    [[nodiscard]] AstLineNode* duplicate() const override {
        auto* result = new AstLabel(*this);
        return result;
    }

    void write_to_ostream(std::ostream &ostream, const AstNode &node) const override {
        ostream << "ReplaceSymbolParameter[name='" << name << "']";
    }
};

class AstMemoryManager {
    std::list<AstNode*> allocated_nodes;
    std::list<char*> allocated_cstrings;
    std::list<std::pair<void*, std::function<void (void*)>>> allocated_stuff;

    template<class T>
    std::function<void (void*)> make_deleter() {
        return [](void* ptr) { delete (T*)ptr; };
    }

public:
    AstInstruction* new_instruction(std::string_view name) {
        auto* instruction = new AstInstruction(name);
        allocated_nodes.push_front(instruction);
        return instruction;
    }

    AstDirective* new_directive(std::string_view name) {
        auto* instruction = new AstDirective(name);
        allocated_nodes.push_front(instruction);
        return instruction;
    }

    template<class T>
    std::list<T>* new_list() {
        auto* list = new std::list<T>;
        allocated_stuff.push_front({list, make_deleter<std::list<T>>()});
        return list;
    }

    AstNumberParameter* new_number_parameter(int value) {
        auto* param = new AstNumberParameter(value);
        allocated_nodes.push_front(param);
        return param;
    }

    AstSymbolParameter* new_symbol_parameter(std::string_view value) {
        auto* param = new AstSymbolParameter(value);
        allocated_nodes.push_front(param);
        return param;
    }

    AstReplaceSymbolParameter* new_replace_symbol_parameter(std::string_view value) {
        auto* param = new AstReplaceSymbolParameter(value);
        allocated_nodes.push_front(param);
        return param;
    }

    AstRegisterParameter* new_register_parameter(std::string_view value) {
        auto* param = new AstRegisterParameter(value);
        allocated_nodes.push_front(param);
        return param;
    }
    AstStringParameter* new_string_parameter(std::string_view value) {
        auto* param = new AstStringParameter(value);
        allocated_nodes.push_front(param);
        return param;
    }

    AstRedactLine* new_redact_line() {
        auto* ptr = new AstRedactLine();
        allocated_nodes.push_front(ptr);
        return ptr;
    }

    AstLabel* new_label(std::string name) {
        if(name.ends_with(":")) {
            name.pop_back();
        }
        assert(!name.ends_with(':'));
        auto* ptr = new AstLabel(name);
        allocated_nodes.push_front(ptr);
        return ptr;
    }


    char* copy_string(char* x) {
        auto len = strlen(x)+1;
        auto r = new char[(len) * sizeof(char)];
        r[len] = 0;
        strcpy(r, x);
        allocated_cstrings.push_front(r);
        return r;
    }

    ~AstMemoryManager() {
        for (auto* item : allocated_nodes) {
            delete item;
        }
        for (auto* item: allocated_cstrings) {
            delete [] item;
        }
        for (auto const& item: allocated_stuff) {
            item.second(item.first);
        }
    }
};

#endif //CS8_AST_H
