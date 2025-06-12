//
// Created by balma on 6/11/25.
//

#pragma once
#include "parser.hpp"

#include <sstream>

class Generator
{
  public:
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog))
    {
    }

    [[nodiscard]] std::string gen_prog() const
    {
        std::stringstream output;
        output << "global _start\n_start:\n";

        // variable stuff blaah blah blah here kfrbghjmyngvfsjgmgngtnmbrfmkjr4efth,krefg k6j5hotmrbu7yjkn43rgv

        output << "    mov rax, 60\n";
        output << "    mov rdi, " << m_root.expr.int_lit.value.value() << '\n';
        output << "    syscall";
        return output.str();
    }

  private:
    const NodeProg m_prog;
};
