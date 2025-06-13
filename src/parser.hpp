//
// Created by balma on 6/11/25.
//
#pragma once

#include "arena.hpp"
#include "tokenization.hpp"

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

struct NodeBinExpr
{
    // std::variant<NodeBinExprAdd*, NodeBinExprMult*> var;
    NodeBinExprAdd* var;
};

struct NodeTerm
{
    std::variant<NodeTermIntLit*, NodeTermIdent*> var;
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
    NodeExpr* expr;
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
        else
        {
            return {};
        }
    }

    [[nodiscard]] std::optional<NodeExpr*> parseExpression()

    {
        if (auto term = parseTerm())
        {
            if (tryConsume(TokenType::PLUS).has_value())
            {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                auto const bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();

                const auto lhs = m_allocator.alloc<NodeExpr>();
                lhs->var = term.value();
                bin_expr_add->lhs = lhs;

                if (const auto rhs = parseExpression())
                {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->var = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                }
                else
                {
                    std::cerr << "Error: Unsupported binary operator\n";
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr->var = term.value();
                return expr;
            }
        }
        else
        {
            return {};
        }
        return {};
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
            tryConsume(TokenType::SEMI, "Error: Expected ';'");

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
            tryConsume(TokenType::SEMI, "Error: Expected ';'");

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
    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const
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