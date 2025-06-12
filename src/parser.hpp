//
// Created by balma on 6/11/25.
//
#pragma once

#include "tokenization.hpp"

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

struct NodeExpr
{
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

// Node Statement Exit
struct NodeStmtExit
{
    NodeExpr expr;
};

// Node Statement Let
struct NodeStmtLet
{
    Token ident;
    NodeExpr expr;
};

// Node Statement
struct NodeStmt
{
    std::variant<NodeStmtExit, NodeStmtLet> var;
};

// Node Program
struct NodeProg
{
    std::vector<NodeStmt> stmt;
};

class Parser
{
  public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens))
    {
    }

    [[nodiscard]] std::optional<NodeExpr> parseExpression()
    {
        if (peek().has_value() && peek().value().type == TokenType::INT_LIT)
        {
            return NodeExpr{.var = NodeExprIntLit{.int_lit = consume()}};
        }
        else if (peek().has_value() && peek().value().type == TokenType::IDENT)
        {
            return NodeExpr{.var = NodeExprIdent{.ident = consume()}};
        }
        else
        {
            return {};
        }
    };

    std::optional<NodeStmt> parseStatement()
    {

        if (peek().value().type == TokenType::EXIT && peek(1).value().type == TokenType::OPEN_PAREN)
        {
            consume();
            consume();
            NodeStmtExit stmt_exit;
            if (auto node_expr = parseExpression())
            {
                stmt_exit = NodeStmtExit{.expr = node_expr.value()};
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
            return NodeStmt{.var = stmt_exit};
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
            NodeStmtLet stmt_let = NodeStmtLet{.ident = consume()};
            if (auto expr = parseExpression())
            {
                stmt_let.expr = expr.value();
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
                std::cerr << "Error: Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }

            return NodeStmt{.var = stmt_let};
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
                prog.stmt.push_back(stmt.value());
            } else
            {
                std::cerr << "Error: Invalid Statement." << std::endl;
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
};