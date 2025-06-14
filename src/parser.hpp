//
// Created by balma on 6/11/25.
//
#pragma once

#include "arena.hpp"
#include "tokenization.hpp"

#include <assert.h>
#include <memory>
#include <variant>
#include <vector>

struct NodeTermIntLit
{
    Token int_lit;
};

struct NodeTermIdent
{
    Token ident;
};
struct NodeExpr;

struct NodeTermParen
{
    NodeExpr* expr;
};

struct NodeBinExprAdd
{
    NodeExpr* rhs;
    NodeExpr* lhs;
};

struct NodeBinExprMult
{
    NodeExpr* rhs;
    NodeExpr* lhs;
};

struct NodeBinExprSub
{
    NodeExpr* rhs;
    NodeExpr* lhs;
};

struct NodeBinExprDiv
{
    NodeExpr* rhs;
    NodeExpr* lhs;
};

struct NodeBinExpr
{
    std::variant<NodeBinExprAdd*, NodeBinExprMult*, NodeBinExprSub*, NodeBinExprDiv*> var;
    // NodeBinExprAdd* var;
};

struct NodeTerm
{
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr
{
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

// Node Statement Exit
struct NodeStmtExit
{
    NodeExpr* expr;
};

// Node Statement Let
struct NodeStmtLet
{
    Token ident;
    NodeExpr* expr{};
};

// Node Statement
struct NodeStmt
{
    std::variant<NodeStmtExit*, NodeStmtLet*> var;
};

// Node Program
struct NodeProg
{
    std::vector<NodeStmt> stmts;
};

class Parser
{
  public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4)
    {
    }

    std::optional<NodeTerm*> parseTerm()
    {
        if (const auto int_lit = tryConsume(TokenType::INT_LIT))
        {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();

            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        else if (const auto ident = tryConsume(TokenType::IDENT))
        {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();

            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        else if (auto open_paren = tryConsume(TokenType::OPEN_PAREN))
        {
            const auto expr = parseExpression();
            if (!expr.has_value())
            {
                std::cout << "Error: Invalid Expression.";
                exit(EXIT_FAILURE);
            }
            tryConsume(TokenType::CLOSE_PAREN, "Error: Expected `)`");
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();

            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;
            return term;
        }
        else
        {
            return {};
        }
    }

    [[nodiscard]] std::optional<NodeExpr*> parseExpression(const int min_prec = 0)

    {

        const std::optional<NodeTerm*> term_lhs = parseTerm();
        if (!term_lhs.has_value())
            return {};

        const auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();

        while (true)
        {
            std::optional<Token> curr_tok = peek();
            std::optional<int> prec;
            if (curr_tok.has_value())
            {
                prec = BinPrec(curr_tok->type);
                if (!prec.has_value() || prec < min_prec)
                {
                    break;
                }
            }
            else
            {
                break;
            }

            auto [op_type, op_value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parseExpression(next_min_prec);
            if (!expr_rhs.has_value())
            {
                std::cerr << "Error: Unable to parse expression.\n";
                exit(EXIT_FAILURE);
            }

            const auto expr = m_allocator.alloc<NodeBinExpr>();
            const auto expr_lhs2 = m_allocator.alloc<NodeExpr>();

            if (op_type == TokenType::ADD)
            {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_lhs2->var = expr_lhs->var;
                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                expr->var = add;
            }
            else if (op_type == TokenType::MULT)
            {
                auto mult = m_allocator.alloc<NodeBinExprMult>();
                expr_lhs2->var = expr_lhs->var;
                mult->lhs = expr_lhs2;
                mult->rhs = expr_rhs.value();
                expr->var = mult;
            }
            else if (op_type == TokenType::SUB)
            {
                auto sub = m_allocator.alloc<NodeBinExprSub>();
                expr_lhs2->var = expr_lhs->var;
                sub->lhs = expr_lhs2;
                sub->rhs = expr_rhs.value();
                expr->var = sub;
            }
            else if (op_type == TokenType::DIV)
            {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                expr_lhs2->var = expr_lhs->var;
                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                expr->var = div;
            }
            else
            {
                assert(false);
            }

            expr_lhs->var = expr;
        }
        return expr_lhs;
    }

    std::optional<NodeStmt*> parseStatement()
    {

        if (peek().value().type == TokenType::EXIT && peek(1).value().type == TokenType::OPEN_PAREN)
        {
            consume();
            consume();
            // NodeStmtExit stmt_exit;
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();

            if (const auto node_expr = parseExpression())
            {
                stmt_exit->expr = node_expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }
            tryConsume(TokenType::CLOSE_PAREN, "Error: Expected `)`");

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }

        /*
         * let ident = (int_lit)
         * TODO: more data types?
         */
        else if (peek().has_value() && peek().value().type == TokenType::LET &&
                 peek(1).has_value() && peek(1).value().type == TokenType::IDENT &&
                 peek(2).has_value() && peek(2).value().type == TokenType::EQ)
        {
            consume();
            // auto stmt_let = NodeStmtLet{.ident = consume()};
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            if (const auto expr = parseExpression())
            {
                stmt_let->expr = expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }
        else
        {
            return {};
        }
    };

    std::optional<NodeProg> parseProgram()
    {
        NodeProg prog;
        while (peek().has_value())
        {
            if (auto stmt = parseStatement())
            {
                prog.stmts.push_back(*stmt.value());
            }
            else
            {
                std::cerr << "Invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

  private:
    [[nodiscard]] inline std::optional<Token> peek(const int offset = 0) const
    {
        if (m_idx + offset >= m_tokens.size())
        {
            return {};
        }
        else
        {
            return m_tokens.at(m_idx + offset);
        }
    }

    inline Token consume()
    {
        return m_tokens.at(m_idx++);
    }

    inline Token tryConsume(const TokenType type, const std::string& err_msg)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> tryConsume(const TokenType type)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            return {};
        }
    }

    const std::vector<Token> m_tokens;
    int m_idx = 0;
    ArenaAllocator m_allocator;
};