//
// Created by balma on 6/11/25.
//

#pragma once
#include "parser.hpp"

#include <algorithm>
#include <map>
#include <ranges>
#include <sstream>
// #include <llvm/IR/>

// YOU SHOULD NOT USE MY CODE FOR REFERENCE, I HAVE NO IDEA WHAT IM DOING
// I'M LEARNING ASM WHILE DOING THIS, USING MY CODE IS EQUIVALENT TO DE OPTIMIZATION

class Generator
{
  public:
    explicit Generator(NodeProg prog) : m_prog(std::move(prog))
    {
    }

    TokenType genExpression(const NodeExpr* expr)
    {
        struct ExprVisitor
        {
            Generator& gen;
            TokenType type;

            void operator()(const NodeTerm* term)
            {
                type = gen.genTerm(term);
            }

            void operator()(const NodeBinExpr* bin_expr)
            {
                type = gen.genBinExpr(bin_expr);
            }
        };

        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);
        return visitor.type;
    }

    TokenType genTerm(const NodeTerm* term)
    {
        struct TermVisitor
        {
            Generator& gen;
            TokenType type;

            void operator()(const NodeTermIntLit* term_int_lit)
            {
                gen.mov("rax", term_int_lit->int_lit.value.value());
                gen.push("rax");
                type = TokenType::INT_TYPE;
            }

            void operator()(const NodeTermIdent* term_ident)
            {
                const auto it =
                    std::ranges::find_if(std::as_const(gen.m_vars), [&](const Var& var)
                                         { return var.name == term_ident->ident.value.value(); });

                if (it == gen.m_vars.cend())
                {
                    std::cerr << "Error: Undeclared Identifier.\n";
                    exit(1);
                }

                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]";

                gen.mov("rax", offset.str());
                gen.push("rax");
                type = it->type;
            }

            void operator()(const NodeTermParen* term_paren)
            {
                type = gen.genExpression(term_paren->expr);
            }

            void operator()(const NodeTermStringLit* string_lit)
            {
                const std::string label = ".str#" + std::to_string(gen.newLabelId());
                gen.m_strings.insert({label, string_lit->string_lit.value.value()});
                gen.mov("rax", label);
                gen.push("rax");
                type = TokenType::STRING_TYPE;
            }
        };

        TermVisitor visitor{.gen = *this};
        std::visit(visitor, term->var);
        return visitor.type;
    }

    TokenType genBinExpr(const NodeBinExpr* bin_expr)
    {
        struct BinExprVisitor
        {
            Generator& gen;

            void operator()(const NodeBinExprAdd* bin_expr_add) const
            {
                gen.genExpression(bin_expr_add->lhs);
                gen.genExpression(bin_expr_add->rhs);
                gen.pop("rbx");
                gen.pop("rax");
                gen.instr("add rax, rbx");
                gen.push("rax");
            }

            void operator()(const NodeBinExprMult* bin_expr_mult) const
            {
                gen.genExpression(bin_expr_mult->lhs);
                gen.genExpression(bin_expr_mult->rhs);
                gen.pop("rbx");
                gen.pop("rax");
                gen.instr("imul rax, rbx");
                gen.push("rax");
            }

            void operator()(const NodeBinExprSub* bin_expr_sub) const
            {
                gen.genExpression(bin_expr_sub->lhs);
                gen.genExpression(bin_expr_sub->rhs);
                gen.pop("rbx");
                gen.pop("rax");
                gen.instr("sub rax, rbx");
                gen.push("rax");
            }

            void operator()(const NodeBinExprDiv* bin_expr_div) const
            {
                gen.genExpression(bin_expr_div->lhs);
                gen.genExpression(bin_expr_div->rhs);
                gen.pop("rbx");
                gen.pop("rax");
                gen.instr("xor rdx, rdx");
                gen.instr("div rbx");
                gen.push("rax");
            }
        };

        BinExprVisitor visitor{.gen = *this};
        std::visit(visitor, bin_expr->var);
        return TokenType::INT_TYPE;
    }

    void genScope(const NodeScope* scope)
    {
        begin_scope();

        for (const NodeStmt* s : scope->stmts)
        {
            genStatement(s);
        }

        end_scope();
    }

    void genIfPred(const NodeIfPred* if_pred)
    {
        struct IfPredVisitor
        {
            Generator& gen;

            void operator()(const NodeIfPredElif* if_pred_elif) const
            {
                const std::string elif_true = ".elif_true#" + std::to_string(gen.newLabelId());
                const std::string after_if = ".after_if#" + std::to_string(gen.newLabelId());

                gen.genExpression(if_pred_elif->expr);
                gen.pop("rax");

                gen.instr("test rax, rax");
                gen.instr("jnz " + elif_true);

                if (if_pred_elif->if_pred.has_value())
                    gen.genIfPred(if_pred_elif->if_pred.value());

                gen.instr("jmp " + after_if);

                gen.createLabel(elif_true);
                gen.genScope(if_pred_elif->scope);
                gen.createLabel(after_if);
            }

            void operator()(const NodeIfPredElse* if_pred_else) const
            {
                gen.genScope(if_pred_else->scope);
            }
        };

        IfPredVisitor visitor{.gen = *this};
        std::visit(visitor, if_pred->var);
    }

    void genStatement(const NodeStmt* stmt)
    {
        struct StmtVisitor
        {
            Generator& gen;

            void operator()(const NodeStmtLet* stmt_let) const
            {
                const auto it =
                    std::ranges::find_if(std::as_const(gen.m_vars), [&](const Var& var)
                                         { return var.name == stmt_let->ident.value.value(); });

                if (it != gen.m_vars.cend())
                {
                    std::cerr << "Error: Identifier already exists\n";
                    exit(1);
                }

                gen.genExpression(stmt_let->expr);

                if (stmt_let->type == TokenType::INT_TYPE)
                {
                    gen.m_vars.push_back(
                        {stmt_let->ident.value.value(), gen.m_stack_size - 1, TokenType::INT_TYPE});
                }
                else if (stmt_let->type == TokenType::STRING_TYPE)
                {
                    if (const auto term = std::get_if<NodeTerm*>(&stmt_let->expr->var))
                    {
                        if ([[maybe_unused]] auto string_lit =
                                std::get_if<NodeTermStringLit*>(&(*term)->var))
                        {
                            gen.m_vars.push_back({stmt_let->ident.value.value(),
                                                  gen.m_stack_size - 1, TokenType::STRING_TYPE});
                        }
                        else
                        {
                            std::cerr
                                << "Error: Expected string literal for string type variable\n";
                            exit(1);
                        }
                    }
                    else
                    {
                        std::cerr << "Error: Expected string literal for string type variable\n";
                        exit(1);
                    }
                }
            }

            void operator()(const NodeStmtAssign* stmt_assign) const
            {
                const auto it =
                    std::ranges::find_if(std::as_const(gen.m_vars), [&](const Var& var)
                                         { return var.name == stmt_assign->ident.value.value(); });

                if (it == gen.m_vars.cend())
                {
                    std::cerr << "Error: Variable not declared: "
                              << stmt_assign->ident.value.value() << "\n";
                    exit(1);
                }

                const TokenType exprType = gen.genExpression(stmt_assign->expr);
                gen.pop("rax");
                if (exprType != it->type)
                {
                    std::cerr << "Error: Type mismatch in assignment to variable '"
                              << stmt_assign->ident.value.value() << "'\n";
                    exit(EXIT_FAILURE);
                }

                std::stringstream offset;
                offset << "[rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]";
                gen.mov(offset.str(), "rax");
            }

            void operator()(const NodeStmtLog* stmt_log) const
            {
                const TokenType type = gen.genExpression(stmt_log->expr);
                gen.pop("rax");

                if (type == TokenType::STRING_TYPE)
                    gen.printString(false);
                else
                    gen.print(false);
            }

            void operator()(const NodeStmtLogLn* stmt_log_ln) const
            {
                const TokenType type = gen.genExpression(stmt_log_ln->expr);
                gen.pop("rax");

                if (type == TokenType::STRING_TYPE)
                    gen.printString(true);
                else
                    gen.print(true);
            }

            void operator()(const NodeStmtExit* stmt_exit) const
            {
                gen.genExpression(stmt_exit->expr);
                gen.pop("rdi");

                gen.mov("rax", "60");

                gen.instr("syscall");
            }

            void operator()(const NodeScope* scope) const
            {
                gen.genScope(scope);
            }

            void operator()(const NodeStmtIf* stmt_if) const
            {
                const std::string if_true = ".if_true#" + std::to_string(gen.newLabelId());
                const std::string after_if = ".after_if#" + std::to_string(gen.newLabelId());

                gen.genExpression(stmt_if->expr);
                gen.pop("rax");

                gen.instr("test rax, rax");
                gen.instr("jnz " + if_true);

                if (stmt_if->if_pred.has_value())
                    gen.genIfPred(stmt_if->if_pred.value());

                gen.instr("jmp " + after_if);

                gen.createLabel(if_true);
                gen.genScope(stmt_if->scope);

                gen.createLabel(after_if);
            }
        };

        StmtVisitor visitor{.gen = *this};

        std::visit(visitor, stmt->var);
    }

    std::string genProgram()
    {
        instr("global _start", false);

        instr("section .bss", false);
        instr("buffer resb 32");

        instr("section .data", false);
        instr("newline db 10");

        instr("section .text", false);
        instr("_start:", false);
        for (const NodeStmt& stmt : m_prog.stmts)
        {
            genStatement(&stmt);
        }
        for (const auto& [label, string] : m_strings)
        {
            instr(label + " db \"" += string + "\", 0", false);
        }

        return m_output.str();
    }

  private:
    void instr(const std::string& instruction, const bool indent = true)
    {
        m_output << (indent ? "    " : "") << instruction << '\n';
    }

    void begin_scope()
    {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope()
    {
        const size_t pop_count = m_vars.size() - m_scopes.back();

        if (pop_count == 0)
            return;

        instr("add rsp, " + std::to_string(pop_count * 8));

        m_stack_size -= pop_count;

        for (size_t i = 0; i < static_cast<int>(pop_count); i++)
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
        instr(label + ':', false);
    }

    struct Var
    {
        std::string name;
        size_t stack_loc;
        TokenType type;
    };

    std::vector<Var> m_vars{};
    size_t m_stack_size = 0;

    std::vector<size_t> m_scopes{};

    std::map<std::string, std::string> m_strings{};

    std::stringstream m_output;
    const NodeProg m_prog;

    int m_labelCount = 0;

    int newLabelId()
    {
        return m_labelCount++;
    }

    void printString(const bool newLine = false)
    {
        const std::string print_loop = ".print_loop#" + std::to_string(newLabelId());
        const std::string print_done = ".print_done#" + std::to_string(newLabelId());

        instr("mov rsi, rax");
        instr("mov rcx, rax");
        instr("xor rdx, rdx");

        createLabel(print_loop);
        instr("mov bl, byte [rcx]");
        instr("test bl, bl");
        instr("jz " + print_done);
        instr("inc rcx");
        instr("inc rdx");
        instr("jmp " + print_loop);
        createLabel(print_done);

        instr("mov rax, 1");
        instr("mov rdi, 1");
        instr("syscall");

        if (newLine)
        {
            instr("mov rax, 1");       // syscall write
            instr("mov rdi, 1");       // fd = stdout
            instr("mov rsi, newline"); // buffer = newline
            instr("mov rdx, 1");       // length = 1
            instr("syscall");
        }
    }

    void print(const bool newLine = false)
    {
        const std::string label = ".print_loop#" + std::to_string(newLabelId());
        const std::string label_done = ".print_done#" + std::to_string(newLabelId());

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
