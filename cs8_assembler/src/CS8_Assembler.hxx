//
// Created by mkr on 7/28/21.
//

#ifndef CS8_CS8_ASSEMBLER_HXX
#define CS8_CS8_ASSEMBLER_HXX
#include <filesystem>
#include <iostream>

class cs8_assembler {
public:
    void assemble(std::filesystem::path const &output,
                  std::filesystem::path const &input);

    void assemble(std::ostream &output, std::istream &input); //<!TODO
};



#endif //CS8_CS8_ASSEMBLER_HXX
