//
// Created by balma on 6/11/25.
//
#pragma once

#include "arena.hpp"
#include "tokenization.hpp"

#include <memory>
#include <variant>
#include <vector>

struct NodeExprIntLit
{
    Token int_lit;
};

struct NodeExprIdent
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
    std::variant<NodeBinExprAdd*, NodeBinExprMult*> var;
};

struct NodeExpr
{
    std::variant<NodeExprIntLit*, NodeExprIdent*, NodeBinExpr*> var;
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

    std::optional<NodeBinExpr*> parseBinExpr()
    {
        if (auto const lhs = parseExpression())
        {
            auto bin_expr = m_allocator.alloc<NodeBinExpr>();
            if (peek().has_value() && peek().value().type == TokenType::PLUS)
            {
                // plus
                auto const bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();

                bin_expr_add->lhs = lhs.value();
                consume();

                if (const auto rhs = parseExpression())
                {
                    bin_expr_add->rhs = rhs.value();
                }
                else
                {
                    std::cerr << "Error: Unsupported binary operator\n";
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                std::cerr << "Error: Unsupported binary operator\n";
                exit(EXIT_FAILURE);
            }

            return bin_expr;
        }
        else
        {
            return {};
        }
    }

    [[nodiscard]] std::optional<NodeExpr*> parseExpression()
    {
        if (peek().has_value() && peek().value().type == TokenType::INT_LIT)
        {
            auto node_expr_int_lit = m_allocator.alloc<NodeExprIntLit>();
            node_expr_int_lit->int_lit = consume();

            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = node_expr_int_lit;
            return expr;
        }
        else if (peek().has_value() && peek().value().type == TokenType::IDENT)
        {
            auto node_expr_ident = m_allocator.alloc<NodeExprIdent>();
            node_expr_ident->ident = consume();

            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = node_expr_ident;
            return expr;
        }
        else if (auto bin_expr = parseBinExpr())
        {
            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = bin_expr.value();
            return expr;
        }
        else
        {
            return {};
        }
    };

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
            if (peek().has_value() && peek().value().type == TokenType::CLOSE_PAREN)
            {
                consume();
            }
            else
            {
                std::cerr << "Error: Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::SEMI)
            {
                consume();
            }
            else
            {
                std::cerr << "Error: Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }

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
            if (peek().has_value() && peek().value().type == TokenType::SEMI)
            {
                consume();
            }
            else
            {
                // TODO: FIX, after adding bin expr, it still exepcts a ; after firsr number, eg
                // let x = 5 + 5;
                //           ^  ^ semi colon here
                //           expects semi colon
                std::cerr << "Error: Expected ';'" << std::endl;
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

    const std::vector<Token> m_tokens;
    int m_idx = 0;
    ArenaAllocator m_allocator;
};