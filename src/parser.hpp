//
// Created by balma on 6/11/25.
//
#pragma once

#include "arena.hpp"
#include "tokenization.hpp"

#include <cassert>
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

struct NodeTermStringLit
{
    Token string_lit;
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

/**/
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
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*, NodeTermStringLit*> var;
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

struct NodeStmtLog
{
    NodeExpr* expr;
};

struct NodeStmtLogLn
{
    NodeExpr* expr;
};

// Node Statement Let
struct NodeStmtLet
{
    Token ident;
    TokenType type{};
    NodeExpr* expr{};
};

struct NodeStmtAssign
{
    Token ident;
    NodeExpr* expr{};
};

struct NodeStmt;

struct NodeScope
{
    std::vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif
{
    NodeExpr* expr{};
    NodeScope* scope{};
    std::optional<NodeIfPred*> if_pred{};
};

struct NodeIfPredElse
{
    NodeScope* scope{};
};

struct NodeIfPred
{
    std::variant<NodeIfPredElif*, NodeIfPredElse*> var;
};

struct NodeStmtIf
{
    NodeExpr* expr{};
    NodeScope* scope{};
    std::optional<NodeIfPred*> if_pred{};
};

// Node Statement
struct NodeStmt
{
    std::variant<NodeStmtExit*, NodeStmtLet*, NodeStmtAssign*, NodeScope*, NodeStmtIf*,
                 NodeStmtLog*, NodeStmtLogLn*>
        var;
};

// Node Program
struct NodeProg
{
    std::vector<NodeStmt> stmts;
};

class Parser
{
  public:
    explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4)
    {
    }

    [[noreturn]] void errExpected(const std::string& err) const
    {
        std::cerr << "[Parse Error] Expected " << err << " on line " << peek(-1).value().line
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<NodeScope*> parseScope()
    {
        if (!tryConsume(TokenType::OPEN_BRACKET).has_value())
            return {};

        auto scope = m_allocator.emplace<NodeScope>();
        while (auto stmt = parseStatement())
        {
            scope->stmts.push_back(stmt.value());
        }
        static_cast<void>(tryConsumeErr(TokenType::CLOSE_BRACKET));
        return scope;
    }

    std::optional<NodeTerm*> parseTerm()
    {
        if (const auto int_lit = tryConsume(TokenType::INT_LIT))
        {
            auto term_int_lit = m_allocator.emplace<NodeTermIntLit>(int_lit.value());

            auto term = m_allocator.emplace<NodeTerm>(term_int_lit);
            return term;
        }
        if (const auto ident = tryConsume(TokenType::IDENT))
        {
            auto term_ident = m_allocator.emplace<NodeTermIdent>(ident.value());

            auto term = m_allocator.emplace<NodeTerm>(term_ident);
            return term;
        }
        if (auto open_paren = tryConsume(TokenType::OPEN_PAREN))
        {
            const auto expr = parseExpression();
            if (!expr.has_value())
            {
                std::cout << "Error: Invalid Expression.";
                exit(EXIT_FAILURE);
            }
            static_cast<void>(tryConsumeErr(TokenType::CLOSE_PAREN));
            auto term_paren = m_allocator.emplace<NodeTermParen>(expr.value());

            auto term = m_allocator.emplace<NodeTerm>(term_paren);
            return term;
        }
        if (const auto string_lit = tryConsume(TokenType::STRING_LIT))
        {
            auto term_string_lit = m_allocator.emplace<NodeTermStringLit>(string_lit.value());
            auto term = m_allocator.emplace<NodeTerm>(term_string_lit);
            return term;
        }

        return {};
    }

    [[nodiscard]] std::optional<NodeExpr*> parseExpression(const int min_prec = 0)
    {
        const std::optional<NodeTerm*> term_lhs = parseTerm();
        if (!term_lhs.has_value())
            return {};

        const auto expr_lhs = m_allocator.emplace<NodeExpr>(term_lhs.value());

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

            auto [op_type, lineCount, op_value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parseExpression(next_min_prec);
            if (!expr_rhs.has_value())
            {
                std::cerr << "Error: Unable to parse expression.\n";
                exit(EXIT_FAILURE);
            }

            const auto expr = m_allocator.emplace<NodeBinExpr>();
            const auto expr_lhs2 = m_allocator.emplace<NodeExpr>();

            if (op_type == TokenType::PLUS)
            {
                auto add = m_allocator.emplace<NodeBinExprAdd>(expr_lhs2, expr_rhs.value());
                expr_lhs2->var = expr_lhs->var;
                expr->var = add;
            }
            else if (op_type == TokenType::STAR)
            {
                auto mult = m_allocator.emplace<NodeBinExprMult>(expr_lhs2, expr_rhs.value());
                expr_lhs2->var = expr_lhs->var;
                expr->var = mult;
            }
            else if (op_type == TokenType::MINUS)
            {
                auto sub = m_allocator.emplace<NodeBinExprSub>(expr_lhs2, expr_rhs.value());
                expr_lhs2->var = expr_lhs->var;
                expr->var = sub;
            }
            else if (op_type == TokenType::F_SLASH)
            {
                auto div = m_allocator.emplace<NodeBinExprDiv>(expr_lhs2, expr_rhs.value());
                expr_lhs2->var = expr_lhs->var;
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

    std::optional<NodeIfPred*> parseIfPredicate()
    {
        if (tryConsume(TokenType::ELIF))
        {
            static_cast<void>(tryConsumeErr(TokenType::OPEN_PAREN));
            const auto elif = m_allocator.emplace<NodeIfPredElif>();
            if (const auto expr = parseExpression())
            {
                elif->expr = expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }

            static_cast<void>(tryConsumeErr(TokenType::CLOSE_PAREN));

            if (const auto scope = parseScope())
            {
                elif->scope = scope.value();
            }
            else
            {
                std::cerr << "Error: Invalid Scope.\n";
                exit(EXIT_FAILURE);
            }

            elif->if_pred = parseIfPredicate();

            auto stmt_if_pred = m_allocator.emplace<NodeIfPred>();
            stmt_if_pred->var = elif;

            return stmt_if_pred;
        }
        else if (tryConsume(TokenType::ELSE))
        {
            const auto else_pred = m_allocator.emplace<NodeIfPredElse>();

            if (const auto scope = parseScope())
            {
                else_pred->scope = scope.value();
            }
            else
            {
                std::cerr << "Error: Invalid Scope.\n";
                exit(EXIT_FAILURE);
            }

            auto stmt_if_pred = m_allocator.emplace<NodeIfPred>();
            stmt_if_pred->var = else_pred;

            return stmt_if_pred;
        }
        else
        {
            return {};
        }
    }

    std::optional<NodeStmt*> parseStatement()
    {
        if (tryConsume(TokenType::EXIT) && tryConsume(TokenType::OPEN_PAREN))
        {
            // NodeStmtExit stmt_exit;
            auto stmt_exit = m_allocator.emplace<NodeStmtExit>();

            if (const auto node_expr = parseExpression())
            {
                stmt_exit->expr = node_expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }
            static_cast<void>(tryConsumeErr(TokenType::CLOSE_PAREN));

            auto stmt = m_allocator.emplace<NodeStmt>(stmt_exit);
            return stmt;
        }
        if (peek().value().type == TokenType::IDENT && peek(1).has_value() &&
            peek(1).value().type == TokenType::EQ)
        {
            // x = 5
            auto stmt_assign = m_allocator.emplace<NodeStmtAssign>(consume());

            // = 5
            consume();

            // 5
            if (const auto expr = parseExpression())
            {
                stmt_assign->expr = expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }

            auto stmt = m_allocator.emplace<NodeStmt>(stmt_assign);
            return stmt;
        }
        if (tryConsume(TokenType::LET) && peek().has_value() &&
            peek().value().type == TokenType::IDENT && peek(1).has_value() &&
            peek(1).value().type == TokenType::COLON)
        {
            auto stmt_let = m_allocator.emplace<NodeStmtLet>(consume());
            consume();

            if (peek().has_value() && (peek().value().type == TokenType::INT_TYPE ||
                                       peek().value().type == TokenType::STRING_TYPE))
            {
                stmt_let->type = consume().type;
            }
            else
            {
                std::cerr << "Error: Expected type annotation (int or string)." << std::endl;
                exit(EXIT_FAILURE);
            }

            if (!peek().has_value() || peek().value().type != TokenType::EQ)
            {
                std::cerr << "Error: Expected '=' after type annotation." << std::endl;
                exit(EXIT_FAILURE);
            }
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

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::OPEN_BRACKET)
        {
            if (auto scope = parseScope())
            {
                auto stmt = m_allocator.emplace<NodeStmt>(scope.value());
                return stmt;
            }
            std::cerr << "Error: Invalid Scope.\n";
            exit(EXIT_FAILURE);
        }
        if (tryConsume(TokenType::PRINT) && tryConsume(TokenType::OPEN_PAREN))
        {
            auto stmt_log = m_allocator.emplace<NodeStmtLog>();

            if (const auto node_expr = parseExpression())
            {
                stmt_log->expr = node_expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }
            static_cast<void>(tryConsumeErr(TokenType::CLOSE_PAREN));

            auto stmt = m_allocator.emplace<NodeStmt>(stmt_log);
            return stmt;
        }
        if (tryConsume(TokenType::PRINTLN) && tryConsume(TokenType::OPEN_PAREN))
        {
            auto stmt_log = m_allocator.emplace<NodeStmtLogLn>();

            if (const auto node_expr = parseExpression())
            {
                stmt_log->expr = node_expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }
            static_cast<void>(tryConsumeErr(TokenType::CLOSE_PAREN));

            auto stmt = m_allocator.emplace<NodeStmt>(stmt_log);
            return stmt;
        }
        if (auto if_ = tryConsume(TokenType::IF))
        {
            static_cast<void>(tryConsumeErr(TokenType::OPEN_PAREN));
            const auto stmt_if = m_allocator.emplace<NodeStmtIf>();
            if (const auto expr = parseExpression())
            {
                stmt_if->expr = expr.value();
            }
            else
            {
                std::cerr << "Error: Invalid Expression." << std::endl;
                exit(EXIT_FAILURE);
            }

            static_cast<void>(tryConsumeErr(TokenType::CLOSE_PAREN));

            if (const auto scope = parseScope())
            {
                stmt_if->scope = scope.value();
            }
            else
            {
                std::cerr << "Error: Invalid Scope.\n";
                exit(EXIT_FAILURE);
            }

            stmt_if->if_pred = parseIfPredicate();

            auto stmt = m_allocator.emplace<NodeStmt>(stmt_if);
            return stmt;
        }
        return {};
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
    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const
    {
        if (m_idx + offset >= m_tokens.size())
        {
            return {};
        }
        return m_tokens.at(m_idx + offset);
    }

    Token consume()
    {
        return m_tokens.at(m_idx++);
    }

    Token tryConsumeErr(const TokenType type)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        errExpected(tokenTypeToString(type));
    }


    std::optional<Token> tryConsume(const TokenType type)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        return {};
    }

    const std::vector<Token> m_tokens;
    int m_idx = 0;
    ArenaAllocator m_allocator;
};
