
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
    // SEMI,
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    IDENT,
    LET,
    EQ,
    PLUS,
    STAR,
    MINUS,
    F_SLASH,
    IF,
    LOG
};

inline std::optional<int> BinPrec(const TokenType type)
{
    switch (type)
    {
    case TokenType::PLUS:
    case TokenType::MINUS:
        return 1;
    case TokenType::STAR:
    case TokenType::F_SLASH:
        return 2;
    default:
        return {};
    }
}

struct Token
{
    TokenType type;
    std::optional<std::string> value{};
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

        int lineCount = 1;
        while (peek().has_value())
        {
            const char c = peek().value();
            if (std::isalpha(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value()))
                {
                    buf.push_back(consume());
                }
                if (buf == "exit")
                {
                    tokens.push_back({.type = TokenType::EXIT});
                    buf.clear();
                }
                else if (buf == "let")
                {
                    tokens.push_back({.type = TokenType::LET});
                    buf.clear();
                }
                else if (buf == "if")
                {
                    tokens.push_back({.type = TokenType::IF});
                    buf.clear();
                }
                else if (buf == "log")
                {
                    tokens.push_back({.type = TokenType::LOG});
                    buf.clear();
                }
                else
                {
                    tokens.push_back({.type = TokenType::IDENT, .value = buf});
                    buf.clear();
                }
            }
            else if (std::isdigit(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value()))
                {
                    buf.push_back(consume());
                }
                tokens.push_back({.type = TokenType::INT_LIT, .value = buf});
                buf.clear();
            }
            else if (std::isspace(peek().value()))
            {
                consume();
            }
            else
            {
                TokenType type;

                consume();

                switch (c)
                {
                case '\n':
                    lineCount++;
                case '(':
                    type = TokenType::OPEN_PAREN;
                    break;
                case ')':
                    type = TokenType::CLOSE_PAREN;
                    break;
                case '{':
                    type = TokenType::OPEN_BRACKET;
                    break;
                case '}':
                    type = TokenType::CLOSE_BRACKET;
                    break;
                case '=':
                    type = TokenType::EQ;
                    break;
                case '+':
                    type = TokenType::PLUS;
                    break;
                case '*':
                    type = TokenType::STAR;
                    break;
                case '/':
                    type = TokenType::F_SLASH;
                    break;
                case '-':
                    type = TokenType::MINUS;
                    break;
                // case ';':
                //     type = TokenType::SEMI;
                //     break;
                default:
                    std::cerr << "Error: Line " << lineCount << " Unexpected token '" << c << "'" << std::endl;
                    exit(EXIT_FAILURE);
                }
                tokens.push_back({.type = type});
            }
        }

        m_idx = 0;
        return tokens;
    }

  private:
    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const
    {
        if (m_idx + offset >= m_src.length())
        {
            return {};
        }
        else
            return m_src.at(m_idx + offset);
    }

    inline char consume()
    {
        return m_src.at(m_idx++);
    }

    const std::string m_src;
    int m_idx = 0;
};
