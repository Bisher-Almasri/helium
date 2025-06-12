//
// Created by balma on 6/11/25.
//
#pragma once

#include "tokenization.hpp"

#include <vector>

struct NodeExpr
{
    Token int_lit;
};

struct NodeExit
{
    NodeExpr expr;
};

class Parser
{
  public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens))
    {
    }

    std::optional<NodeExpr> parseExpression()
    {
        if (peek().has_value() && peek().value().type == TokenType::INT_LIT)
        {
            return NodeExpr{.int_lit = consume()};
        }
        else
        {
            return {};
        }
    };

    std::optional<NodeExit> parse()
    {
        std::optional<NodeExit> exit_node;
        while (peek().has_value())
        {
            if (peek().value().type == TokenType::EXIT)
            {
                consume();
                if (auto node_expr = parseExpression())
                {
                    exit_node = NodeExit{.expr = node_expr.value()};
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
                    std::cerr << "Error: Invalid Expression." << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        m_idx = 0;
        return exit_node;
    };

  private:
    [[nodiscard]] inline std::optional<Token> peek(int ahead = 1) const
    {
        if (m_idx + ahead > m_tokens.size())
        {
            return {};
        }
        else
        {
            return m_tokens.at(m_idx);
        }
    }

    inline Token consume()
    {
        return m_tokens.at(m_idx++);
    }

    const std::vector<Token> m_tokens;
    int m_idx = 0;
};