
//
// Created by Bisher Almasri on 2025-06-10.
//

#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

enum class TokenType
{
    EXIT,
    INT_LIT,
    SEMI
};

struct Token
{
    TokenType type;
    std::optional<std::string> value;
};

class Tokenizer
{
  public:
    inline explicit Tokenizer(std::string src) : m_src(std::move(src))
    {
    }

    inline std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;

        std::string buf;
        while (peak().has_value())
        {
            if (std::isalpha(peak().value()))
            {
                buf.push_back(consume());
                while (peak().has_value() && std::isalnum(peak().value()))
                {
                    buf.push_back(consume());
                }
                if (buf == "exit")
                {
                    tokens.push_back({.type = TokenType::EXIT});
                    buf.clear();
                    // continue;
                }
                else
                {
                    std::cerr << "Error: Unknown keyword '" << buf << "'" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::isdigit(peak().value()))
            {
                buf.push_back(consume());
                while (peak().has_value() && std::isdigit(peak().value()))
                {
                    buf.push_back(consume());
                }
                tokens.push_back({.type = TokenType::INT_LIT, .value = buf});
                buf.clear();
                continue;
            }
            else if (peak().value() == ';')
            {
                consume();
                tokens.push_back({.type = TokenType::SEMI});
                continue;
            }
            else if (std::isspace(peak().value()))
            {
                consume();
                continue;
            }
            else
            {
                std::cerr << "Error: Unexpected token '" << peak().value() << "'" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        m_idx = 0;
        return tokens;
    }

  private:
    [[nodiscard]] inline std::optional<char> peak(int ahead = 1) const
    {
        if (m_idx + ahead > m_src.length())
        {
            return {};
        }
        else
            return m_src.at(m_idx);
    }

    inline char consume()
    {
        return m_src.at(m_idx++);
    }

    const std::string m_src;
    int m_idx = 0;
};
