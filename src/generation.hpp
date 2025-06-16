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

    void genTerm(const NodeTerm* term)
    {
        struct TermVisitor
        {
            Generator* gen;

            void operator()(const NodeTermIntLit* term_int_lit) const
            {
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
                    exit(1);
                }

                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - it->stack_loc - 1) * 8 << "]";

                gen->mov("rax", offset.str());
                gen->push("rax");
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
                gen->genExpression(bin_expr_add->lhs);
                gen->genExpression(bin_expr_add->rhs);
                gen->pop("rbx");
                gen->pop("rax");
                gen->instr("add rax, rbx");

                gen->push("rax");
            }

            void operator()(const NodeBinExprMult* bin_expr_mult) const
            {
                gen->genExpression(bin_expr_mult->lhs);
                gen->genExpression(bin_expr_mult->rhs);
                gen->pop("rbx"); // rhs
                gen->pop("rax"); // lhs
                gen->instr("imul rax, rbx");

                gen->push("rax"); // push result back
            }

            void operator()(const NodeBinExprSub* bin_expr_sub) const
            {
                gen->genExpression(bin_expr_sub->lhs);
                gen->genExpression(bin_expr_sub->rhs);
                gen->pop("rbx"); // rhs
                gen->pop("rax"); // lhs
                gen->instr("sub rax, rbx");

                gen->push("rax"); // push result back
            }

            void operator()(const NodeBinExprDiv* bin_expr_div) const
            {
                gen->genExpression(bin_expr_div->lhs);
                gen->genExpression(bin_expr_div->rhs);
                gen->pop("rbx");            // rhs
                gen->pop("rax");            // lhs
                gen->instr("xor rdx, rdx"); // clear rdx
                gen->instr("div rbx");

                gen->push("rax"); // push result back
            }
        };

        BinExprVisitor visitor{.gen = this};

        std::visit(visitor, bin_expr->var);
    }

    void genStatement(const NodeStmt* stmt)
    {
        struct StmtVisitor
        {
            Generator* gen;

            void operator()(const NodeStmtLet* stmt_let) const
            {
                const auto it =
                    std::ranges::find_if(std::as_const(gen->m_vars), [&](const Var& var)
                                         { return var.name == stmt_let->ident.value.value(); });

                if (it != gen->m_vars.cend())
                {
                    std::cerr << "Error: Identifier already exists\n";
                    exit(1);
                }

                gen->genExpression(stmt_let->expr);
                gen->m_vars.push_back({stmt_let->ident.value.value(), gen->m_stack_size - 1});
            }

            void operator()(const NodeStmtLogLn* stmt_log_ln) const
            {
                gen->genExpression(stmt_log_ln->expr);
                gen->pop("rax");

                gen->printInt(true);
            }

            void operator()(const NodeStmtLog* stmt_log) const
            {
                gen->genExpression(stmt_log->expr);
                gen->pop("rax");

                gen->printInt(false);
            }

            void operator()(const NodeStmtExit* stmt_exit) const
            {
                gen->genExpression(stmt_exit->expr);
                gen->pop("rdi");

                gen->mov("rax", "60");

                gen->instr("syscall");
            }

            void operator()(const NodeScope* scope) const
            {
                gen->begin_scope();

                for (const NodeStmt* s : scope->stmts)
                {
                    gen->genStatement(s);
                }

                gen->end_scope();
            }

            void operator()(const NodeStmtIf*) const
            {
                assert(false);
            }
        };

        StmtVisitor visitor{.gen = this};

        std::visit(visitor, stmt->var);
    }

    std::string genProgram()
    {
        instr("global _start");

        instr("section .bss");

        instr("buffer resb 32");

        instr("section .data");

        instr("newline db 10");

        instr("section .text");

        instr("_start:");
        for (const NodeStmt& stmt : m_prog.stmts)
        {
            genStatement(&stmt);
        }

        instr("mov rax, 60");
        instr("mov rdi, 0");
        instr("syscall");

        return m_output.str();
    }

  private:
    void instr(const std::string& instruction)
    {
        m_output << "    " << instruction << '\n';
    }

    void begin_scope()
    {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope()
    {
        const size_t pop_count = m_vars.size() - m_scopes.back();
        instr("add rsp" + pop_count * 8);

        m_stack_size -= pop_count;
        for (int i = 0; i < pop_count; i++)
        {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }

    void push(const std::string& reg)
    {
        instr("push " + reg);
        m_stack_size++;
    }

    void pop(const std::string& reg)
    {
        instr("pop " + reg);
        m_stack_size--;
    }

    void mov(const std::string& reg, const std::string& val)
    {
        instr("mov " + reg + ", " + val);
    }

    void createLabel(const std::string& label)
    {
        instr(label + ':');
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

    void printInt(const bool newLine = false)
    {
        static int label_count = 0;
        int id = label_count++;
        std::string label = ".print_loop_" + std::to_string(id);
        std::string label_done = ".print_done_" + std::to_string(id);

        instr("mov rcx, buffer + 31"); // move to the last byte
        instr("mov rbx, 10");          // base 10
        instr("mov rdx, 0");

        createLabel(label);
        instr("xor rdx, rdx");  // clear rdx
        instr("div rbx");       // rax = rax / rbx
        instr("add rdx, '0'");  // rdx = remainder + ascii '0'
        instr("mov [rcx], dl"); // write character
        instr("dec rcx");       // move backwards
        instr("test rax, rax"); // if rax == 0, set zero flag aka we're done the loop
        instr("jnz " + label);

        instr("inc rcx");

        instr("mov rdx, buffer + 32"); // buffer end
        instr("sub rdx, rcx");         // length = buffer_end - rcx
        instr("mov rax, 1");           // sys_write
        instr("mov rdi, 1");           // fd = stdout
        instr("mov rsi, rcx");         // buffer
        instr("syscall");

        if (newLine)
        {
            instr("mov rax, 1");             // sys_write
            instr("mov rdi, 1");             // fd = stdout
            instr("lea rsi, [rel newline]"); // buffer = newline
            instr("mov rdx, 1");             // length = 1
            instr("syscall");
        }

        createLabel(label_done);
    }
};
