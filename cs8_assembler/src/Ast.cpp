//
// Created by mkr on 25.07.21.
//

#include "Ast.h"

std::ostream &operator<<(std::ostream &os, const AstNode &node) {
    node.write_to_ostream(os, node);
    return os;
}


