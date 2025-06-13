//
// Created by balma on 6/11/25.
//

#pragma once
#include "parser.hpp"

#include <sstream>
#include <unordered_map>


// YOU SHOULD NOT USE MY CODE FOR REFERENCE, I HAVE NO IDEA WHAT IM DOING
// I'M LEARNING ASM WHILE DOING THIS, USING MY CODE IS EQUIVALENT TO DE OPTIMIZATION

class Generator
{
  public:
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog))
    {
    }

    void genTerm(const NodeTerm* term)
    {
        struct TermVisitor
        {
            Generator* gen;
            void operator()(const NodeTermIntLit* term_int_lit) const
            {
                // MOVES TO STACK
                gen->mov("rax", term_int_lit->int_lit.value.value());
                gen->push("rax");
            }
            void operator()(const NodeTermIdent* term_ident) const
            {
                if (!gen->m_vars.contains(term_ident->ident.value.value()))
                {
                    std::cerr << "Error: Undeclared Identifier.\n";
                    exit(EXIT_FAILURE);
                }

                const auto& [stack_loc] = gen->m_vars.at(term_ident->ident.value.value());
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }
        };

        TermVisitor visitor {.gen = this};
        std::visit(visitor, term->var);
    }

    // HANDLE EXPRESSION
    void genExpression(const NodeExpr* expr)
    {
        struct ExprVisitor
        {
            Generator* gen;

            void operator()(const NodeTerm* term) const
            {
                gen->genTerm(term);
            }

            void operator()(const NodeBinExpr* bin_expr) const
            {
                gen->genExpression(bin_expr->var->lhs);
                gen->genExpression(bin_expr->var->rhs);
                // both are pushed to the stack of RAX register
                // add requires ax (where res will be restored, and another reg for add
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                // push it to the stack so we can use it
                gen->push("rax");

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
