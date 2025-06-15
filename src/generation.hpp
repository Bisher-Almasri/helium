//
// Created by balma on 6/11/25.
//

#pragma once
#include "parser.hpp"

#include <algorithm>
#include <map>
#include <sstream>

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
                // I don't even know what I wrote. who ever designed c++ lamda's, why just why
                const auto it =
                    std::ranges::find_if(std::as_const(gen->m_vars), [&](const Var& var)
                                         { return var.name == term_ident->ident.value.value(); });

                if (it == gen->m_vars.cend())
                {
                    std::cerr << "Error: Undeclared Identifier.\n";
                    exit(EXIT_FAILURE);
                }

                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - it->stack_loc - 1) * 8 << "]\n";
                gen->push(offset.str());
            }

            void operator()(const NodeTermParen* term_paren) const
            {
                gen->genExpression(term_paren->expr);
            }
        };

        TermVisitor visitor{.gen = this};
        std::visit(visitor, term->var);
    }

    void genBinExpr(const NodeBinExpr* bin_expr)
    {
        struct BinExprVisitor
        {
            Generator* gen;

            void operator()(const NodeBinExprAdd* bin_expr_add) const
            {
                gen->genExpression(bin_expr_add->rhs);
                gen->genExpression(bin_expr_add->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprMult* bin_expr_mult) const
            {
                gen->genExpression(bin_expr_mult->rhs);
                gen->genExpression(bin_expr_mult->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    mul rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprSub* bin_expr_sub) const
            {
                gen->genExpression(bin_expr_sub->rhs);
                gen->genExpression(bin_expr_sub->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    sub rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprDiv* bin_expr_div) const
            {
                gen->genExpression(bin_expr_div->rhs);
                gen->genExpression(bin_expr_div->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    div rbx\n";
                gen->push("rax");
            }
        };

        BinExprVisitor visitor{.gen = this};
        std::visit(visitor, bin_expr->var);
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
                gen->genBinExpr(bin_expr);
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
                const auto it =
                    std::ranges::find_if(std::as_const(gen->m_vars), [&](const Var& var)
                                         { return var.name == stmt_let->ident.value.value(); });
                if (it != gen->m_vars.cend())
                {
                    std::cerr << "Identifier has already been used.\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.push_back(
                    {.name = stmt_let->ident.value.value(), .stack_loc = gen->m_stack_size});
                gen->genExpression(stmt_let->expr);
            }

            void operator()(const NodeScope* scope) const
            {
                gen->begin_scope();
                for (const NodeStmt* stmt : scope->stmts)
                {
                    gen->genStatement(stmt);
                }
                gen->end_scope();
            }

            void operator()(const NodeStmtIf* stmt_if) const
            {
                assert(false);
            }

            void operator()(const NodeStmtLog* stmt_log) const
            {
                gen->genExpression(stmt_log->expr);
                // since now, we generate expr, var should be in rax so we can print int
                gen->printInt();
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }

    std::string genProgram()
    {
        m_output << "global _start\n";

        m_output << "section .bss\n";
        m_output << "buffer resb 32\n";

        m_output << "section .data\n";
        m_output << "newline db 10\n";

        m_output << "section .text\n";

        m_output << "_start:\n";

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
        m_output << "    mov " << reg << ", " << val << '\n';
    }

    void begin_scope()
    {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope()
    {
        const size_t pop_count = m_vars.size() - m_scopes.back();
        m_output << "    add rsp, " << pop_count * 8 << '\n';
        m_stack_size -= pop_count;
        for (int i = 0; i < pop_count; i++)
        {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }

    struct Var
    {
        std::string name;
        size_t stack_loc;
    };

    std::vector<Var> m_vars{};
    size_t m_stack_size = 0;

    std::vector<size_t> m_scopes{};

    std::stringstream m_output;
    const NodeProg m_prog;

    void printInt()
    {
        // this is one hellhole of a code so let me explain it

        // we init RCX to the end of hte buffer
        m_output << "    mov rcx, buffer + 31\n";
        // setup division stuff
        m_output << "    mov rbx, 10\n";
        m_output << "    mov rdx, 0\n";

        // create label print_loop it will be converting int to str essentially then print it
        m_output << ".print_loop:\n";
        // make sure rdx 0
        m_output << "    xor rdx, rdx\n";
        // now we divide since we essentially keep dividing till its 0 cuz the nwe convert all to
        // str
        m_output << "    div rbx\n";
        m_output << "    add rdx, '0'\n"; // make it char by adding '0' to make it ascii
        // push to buffer
        m_output << "    mov [rcx], dl\n";
        // move backwards to go to next character as we started at end of buffer
        m_output << "    dec rcx\n";
        // sets ZF (Zero Flag) if rax == 0
        m_output << "    test rax, rax\n";
        // if it isn't zero repeat the loop
        m_output << "    jnz .print_loop\n";

        // so if it is zero increment to go to first char
        m_output << "    inc rcx\n";

        // gen length of string
        m_output << "    mov rdx, buffer + 32\n";
        m_output << "    sub rdx, rcx\n";

        // syscalls
        m_output << "    mov rax, 1\n";   // sys write
        m_output << "    mov rdi, 1\n";   // stdout
        m_output << "    mov rsi, rcx\n"; // string
        m_output << "    syscall\n";

        // same as up
        m_output << "    mov rax, 1\n";
        m_output << "    mov rdi, 1\n";
        m_output << "    lea rsi, [rel newline]\n"; // print new line
        m_output << "    mov rdx, 1\n";
        m_output << "    syscall\n";
    }
};
