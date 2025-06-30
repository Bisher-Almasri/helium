//
// Created by Bisher Almasri on 2025-06-10.
//

#pragma once
#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>
enum class TokenType
{
    // KEYWORDS
    EXIT,
    LET,
    IF,
    ELSE,
    ELIF,

    // TYPES
    INT_TYPE,
    STRING_TYPE,

    // IDENTIFIERS & CONSTANTS
    IDENT,
    INT_LIT,
    STRING_LIT,

    // GROUPING SYMBOLS
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    COLON,

    // OPERATORS
    EQ,
    PLUS,
    MINUS,
    STAR,
    F_SLASH,

    // I/O
    PRINT,
    PRINTLN
};

inline std::string tokenTypeToString(const TokenType type)
{
    switch (type)
    {
    case TokenType::EXIT:
        return "`exit`";

    case TokenType::INT_LIT:
        return "int literal";

    case TokenType::STRING_LIT:
        return "string literal";

    case TokenType::OPEN_PAREN:
        return "`(`";

    case TokenType::CLOSE_PAREN:
        return "`)`";

    case TokenType::IDENT:
        return "identifier";

    case TokenType::LET:
        return "`let`";

    case TokenType::EQ:
        return "`=`";

    case TokenType::PLUS:
        return "`+`";

    case TokenType::STAR:
        return "`*`";

    case TokenType::MINUS:
        return "`-`";

    case TokenType::F_SLASH:
        return "`/`";

    case TokenType::OPEN_BRACKET:
        return "`{`";

    case TokenType::CLOSE_BRACKET:
        return "`}`";

    case TokenType::IF:
        return "`if`";

    case TokenType::ELIF:
        return "`elif`";

    case TokenType::ELSE:
        return "`else`";
    default:;
    }

    assert(false);
}

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
    int line;
    std::optional<std::string> value{};
};

class Tokenizer
{
  public:
    explicit Tokenizer(std::string src) : m_src(std::move(src))
    {
    }

    std::vector<Token> tokenize()
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
                    tokens.push_back({TokenType::EXIT, lineCount});
                    buf.clear();
                }
                else if (buf == "let")
                {
                    tokens.push_back({TokenType::LET, lineCount});
                    buf.clear();
                }
                else if (buf == "if")
                {
                    tokens.push_back({TokenType::IF, lineCount});
                    buf.clear();
                }
                else if (buf == "print")
                {
                    tokens.push_back({TokenType::PRINT, lineCount});
                    buf.clear();
                }
                else if (buf == "println")
                {
                    tokens.push_back({TokenType::PRINTLN, lineCount});
                    buf.clear();
                }
                else if (buf == "else")
                {
                    tokens.push_back({TokenType::ELSE, lineCount});
                    buf.clear();
                }
                else if (buf == "elif")
                {
                    tokens.push_back({TokenType::ELIF, lineCount});
                    buf.clear();
                }
                else if (buf == "int")
                {
                    tokens.push_back({TokenType::INT_TYPE, lineCount});
                    buf.clear();
                }
                else if (buf == "string")
                {
                    tokens.push_back({TokenType::STRING_TYPE, lineCount});
                    buf.clear();
                }
                else
                {
                    tokens.push_back({TokenType::IDENT, lineCount, buf});
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
                tokens.push_back({TokenType::INT_LIT, lineCount, buf});
                buf.clear();
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/')
            {
                while (peek().has_value() && peek().value() != '\n')
                {
                    consume();
                }
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*')
            {
                consume();
                consume();

                while (peek().has_value())
                {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/')
                    {
                        consume();
                        consume();
                        break;
                    }
                    if (peek().value() == '\n')
                    {
                        lineCount++;
                    }
                    consume();
                }
            }
            else if (c == '\n')
            {
                lineCount++;
                consume();
            }
            else if (std::isspace(peek().value()))
            {
                consume();
            }
            else if (c == '"')
            {
                consume();
                while (peek().has_value() && peek().value() != '"')
                {
                    buf.push_back(consume());
                }
                if (peek().has_value())
                {
                    consume();
                    tokens.push_back({.type = TokenType::STRING_LIT, .value = buf});
                    buf.clear();
                }
                else
                {
                    std::cerr << "Error: Unterminated string literal.\n";
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                TokenType type;

                consume();

                switch (c)
                {
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
                case ':':
                    type = TokenType::COLON;
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
                    std::cerr << "Error: Line " << lineCount << " Unexpected token '" << c << "'"
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
                tokens.push_back({type, lineCount});
            }
        }

        m_idx = 0;
        return tokens;
    }

  private:
    [[nodiscard]] std::optional<char> peek(const size_t offset = 0) const
    {
        if (m_idx + offset >= m_src.length())
        {
            return {};
        }
        return m_src.at(m_idx + offset);
    }

    char consume()
    {
        return m_src.at(m_idx++);
    }

    const std::string m_src;
    int m_idx = 0;
};
