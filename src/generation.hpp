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

    void genExpression(const NodeExpr &expr)
    {
        struct ExprVisitor
        {
            Generator *gen;

            void operator()(const NodeExprIntLit &expr_int_lit) const
            {
                gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() + '\n';
                gen->push("rax");
            }

            void operator()(const NodeExprIdent &expr_ident) const
            {
            }
        };

        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr.var);
    }

    void genStatement(const NodeStmt &stmt)
    {
        struct StmtVisitor
        {
            Generator *gen;

            void operator()(const NodeStmtExit &stmt_exit) const
            {
                gen->genExpression(stmt_exit.expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtLet &stmt_let) const
            {
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] std::string genProgram()
    {
        m_output << "global _start\n_start:\n";

        // variable stuff blaah blah blah here kfrbghjmyngvfsjgmgngtnmbrfmkjr4efth,krefg
        // k6j5hotmrbu7yjkn43rgv

        for (const NodeStmt &stmt : m_prog.stmt)
        {
            genStatement(stmt);
        }

        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";
        return m_output.str();
    }

  private:
    void push(const std::string &reg)
    {
        m_output << "    push " << reg << '\n';
        m_stack_size++;
    }

    void pop(const std::string &reg)
    {
        m_output << "    pop " << reg << '\n';
        m_stack_size--;
    }

    std::stringstream m_output;
    const NodeProg m_prog;
    int m_stack_size = 0;
};
