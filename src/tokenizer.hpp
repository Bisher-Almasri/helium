#include <utility>

//
// Created by Bisher Almasri on 2025-06-10.
//

#pragma once

enum class TokenType { EXIT, INT_LIT, SEMI };

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

class Tokenizer {
public:
    inline Tokenizer(std::string  src) : m_src(std::move(src)) {}

    inline std::vector<Token> tokenize()
    {
        std::vector<Token> tokens = {};

        std::string buf;
        for (int i = 0; i < m_src.length(); )
        {
            char c = m_src.at(i);
            if (std::isalpha(c))
            {
                buf.push_back(c);
                i++;

                while (i < m_src.length() && std::isalnum(m_src.at(i)))
                {
                    buf.push_back(m_src.at(i));
                    i++;
                }

                if (buf == "exit")
                {
                    tokens.push_back({.type = TokenType::EXIT});
                    buf.clear();
                } else
                {
                    std::cerr << "Error: Unknown keyword '" << buf << "'" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::isdigit(c))
            {
                buf.push_back(c);
                i++;

                while (i < m_src.length() && std::isdigit(m_src.at(i)))
                {
                    buf.push_back(m_src.at(i));
                    i++;
                }
                tokens.push_back({.type = TokenType::INT_LIT, .value = buf});
                buf.clear();
            }
            else if (c == ';')
            {
                tokens.push_back({.type = TokenType::SEMI});
                i++;
            }
            else if (std::isspace(c))
            {
                i++;
                continue;
            } else
            {
                std::cerr << "Error: Unexpected character '" << c << "'" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        return tokens;
    }

private:
    std::optional<char> peak() const
    {

    }
    const std::string m_src;
};
