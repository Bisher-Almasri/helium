//
// Created by balma on 6/11/25.
//

#pragma once
#include "parser.hpp"

#include <assert.h>
#include <sstream>
#include <unordered_map>

class Generator
{
  public:
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog))
    {
    }

    // HANDLE EXPERISSION
    void genExpression(const NodeExpr* expr)
    {
        struct ExprVisitor
        {
            Generator* gen;

            void operator()(const NodeExprIntLit* expr_int_lit) const
            {
                // MOVES TO STACK
                gen->mov("rax", expr_int_lit->int_lit.value.value());
                gen->push("rax");
            }

            void operator()(const NodeExprIdent* expr_ident) const
            {
                if (!gen->m_vars.contains(expr_ident->ident.value.value()))
                {
                    std::cerr << "Error: Undeclared Identifier.\n";
                    exit(EXIT_FAILURE);
                }

                const auto& [stack_loc] = gen->m_vars.at(expr_ident->ident.value.value());
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }

            void operator()(const NodeBinExpr* bin_expr) const
            {
                assert(false); // not implemented
            }
        };

        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr->var);
    }

    void genStatement(const NodeStmt* stmt)
    {
        struct StmtVisitor
        {
            Generator* gen;

            void operator()(const NodeStmtExit* stmt_exit) const
            {
                gen->genExpression(stmt_exit->expr);
                gen->mov("rax", "60");
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtLet* stmt_let) const
            {
                if (gen->m_vars.contains(stmt_let->ident.value.value()))
                {
                    std::cerr << "Identifier has already been used.\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert(
                    {stmt_let->ident.value.value(), Var{.stack_loc = gen->m_stack_size}});
                gen->genExpression(stmt_let->expr);
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string genProgram()
    {
        m_output << "global _start\n_start:\n";

        for (const NodeStmt stmt : m_prog.stmts)
        {
            genStatement(&stmt);
        }

        mov("rax", "60");
        mov("rdi", "0");
        m_output << "    syscall\n";
        return m_output.str();
    }

  private:
    void push(const std::string& reg)
    {
        m_output << "    push " << reg << '\n';
        m_stack_size++;
    }

    void pop(const std::string& reg)
    {
        m_output << "    pop " << reg << '\n';
        m_stack_size--;
    }

    void mov(const std::string& reg, const std::string& val)
    {
        // OMFG AFTER DEBUGGING FOR 20 MINUTES
        // I FORGOT A COMMA I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM I HATE ASM
        m_output << "    mov " << reg << ", " << val << '\n';
        //                                ^ MY OPP RIGHT HERE
    }

    struct Var
    {
        size_t stack_loc;
    };

    std::unordered_map<std::string, Var> m_vars{};
    size_t m_stack_size = 0;

    std::stringstream m_output;
    const NodeProg m_prog;
};
