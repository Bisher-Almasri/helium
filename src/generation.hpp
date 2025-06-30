//
// Created by balma on 6/11/25.
//

#pragma once
#include "parser.hpp"

#include <algorithm>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <ranges>
#include <sstream>

// YOU SHOULD NOT USE MY CODE FOR REFERENCE, I HAVE NO IDEA WHAT IM DOING
// I'M LEARNING LLVM WHILE DOING THIS, USING MY CODE IS EQUIVALENT TO DE OPTIMIZATION

class Generator
{
  public:
    explicit Generator(NodeProg prog) : m_prog(std::move(prog))
    {
        m_context = std::make_unique<llvm::LLVMContext>();
        m_module = std::make_unique<llvm::Module>("helium", *m_context);
        m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);

        llvm::FunctionType* mainType =
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*m_context), false);

        m_mainFunction = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main",
                                                m_module.get());

        llvm::BasicBlock* entryBlock =
            llvm::BasicBlock::Create(*m_context, "entry", m_mainFunction);
        m_builder->SetInsertPoint(entryBlock);
    }

    llvm::Value* genExpression(const NodeExpr* expr)
    {
        struct ExprVisitor
        {
            Generator& gen;
            llvm::Value* value;

            void operator()(const NodeTerm* term)
            {
                value = gen.genTerm(term);
            }

            void operator()(const NodeBinExpr* bin_expr)
            {
                value = gen.genBinExpr(bin_expr);
            }
        };

        ExprVisitor visitor{.gen = *this, .value = nullptr};
        std::visit(visitor, expr->var);
        return visitor.value;
    }

    llvm::Value* genTerm(const NodeTerm* term)
    {
        struct TermVisitor
        {
            Generator& gen;
            llvm::Value* value;

            void operator()(const NodeTermIntLit* term_int_lit)
            {
                int intV = std::stoi(term_int_lit->int_lit.value.value());
                value = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*gen.m_context), intV);
                value = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*gen.m_context), intV);
            }

            void operator()(const NodeTermIdent* term_ident)
            {
                const auto it =
                    std::ranges::find_if(std::as_const(gen.m_vars), [&](const Var& var)
                                         { return var.name == term_ident->ident.value.value(); });

                if (it == gen.m_vars.cend())
                {
                    std::cerr << "Error: Undeclared Identifier.\n";
                    exit(1);
                }

                value = gen.m_builder->CreateLoad(llvm::Type::getInt64Ty(*gen.m_context),
                                                  it->alloca, it->name);
            }

            void operator()(const NodeTermParen* term_paren)
            {
                value = gen.genExpression(term_paren->expr);
            }

            void operator()(const NodeTermStringLit* string_lit)
            {
                // TODO: FINISH
            }
        };

        TermVisitor visitor{.gen = *this, .value = nullptr};
        std::visit(visitor, term->var);
        return visitor.value;
    }

    llvm::Value* genBinExpr(const NodeBinExpr* bin_expr)
    {
        struct BinExprVisitor
        {
            Generator& gen;

            llvm::Value* operator()(const NodeBinExprAdd* bin_expr_add) const
            {
                llvm::Value* LHS = gen.genExpression(bin_expr_add->lhs);
                llvm::Value* RHS = gen.genExpression(bin_expr_add->rhs);
                return gen.m_builder->CreateAdd(LHS, RHS, "addtmp");
            }

            llvm::Value* operator()(const NodeBinExprMult* bin_expr_mult) const
            {
                llvm::Value* LHS = gen.genExpression(bin_expr_mult->lhs);
                llvm::Value* RHS = gen.genExpression(bin_expr_mult->rhs);
                return gen.m_builder->CreateMul(LHS, RHS, "multmp");
            }

            llvm::Value* operator()(const NodeBinExprSub* bin_expr_sub) const
            {
                llvm::Value* LHS = gen.genExpression(bin_expr_sub->lhs);
                llvm::Value* RHS = gen.genExpression(bin_expr_sub->rhs);
                return gen.m_builder->CreateSub(LHS, RHS, "subtmp");
            }

            llvm::Value* operator()(const NodeBinExprDiv* bin_expr_div) const
            {
                llvm::Value* LHS = gen.genExpression(bin_expr_div->lhs);
                llvm::Value* RHS = gen.genExpression(bin_expr_div->rhs);
                return gen.m_builder->CreateSDiv(LHS, RHS, "divtmp");
            }
        };

        BinExprVisitor visitor{.gen = *this};
        return std::visit(visitor, bin_expr->var);
    }

    void genScope(const NodeScope* scope)
    {
        begin_scope();

        for (const NodeStmt* s : scope->stmts)
        {
            genStatement(s);
        }

        end_scope();
    }

    void genIfPred(const NodeIfPred* if_pred)
    {
        // TODO: Implement if predicates
        struct IfPredVisitor
        {
            Generator& gen;

            void operator()(const NodeIfPredElif* if_pred_elif) const
            {
                // TODO: Implement elif
            }

            void operator()(const NodeIfPredElse* if_pred_else) const
            {
            }
        };

        IfPredVisitor visitor{.gen = *this};
        std::visit(visitor, if_pred->var);
    }

    void genStatement(const NodeStmt* stmt)
    {
        struct StmtVisitor
        {
            Generator& gen;

            void operator()(const NodeStmtLet* stmt_let) const
            {
                const auto it =
                    std::ranges::find_if(std::as_const(gen.m_vars), [&](const Var& var)
                                         { return var.name == stmt_let->ident.value.value(); });

                if (it != gen.m_vars.cend())
                {
                    std::cerr << "Error: Identifier already exists\n";
                    exit(1);
                }

                llvm::Value* exprV = gen.genExpression(stmt_let->expr);

                if (stmt_let->type == TokenType::INT_TYPE)
                {
                    llvm::AllocaInst* alloca =
                        gen.m_builder->CreateAlloca(llvm::Type::getInt64Ty(*gen.m_context), nullptr,
                                                    stmt_let->ident.value.value());

                    gen.m_builder->CreateStore(exprV, alloca);

                    gen.m_vars.push_back(
                        {stmt_let->ident.value.value(), alloca, TokenType::INT_TYPE});
                }
            }

            void operator()(const NodeStmtAssign* stmt_assign) const
            {
                const auto it =
                    std::ranges::find_if(std::as_const(gen.m_vars), [&](const Var& var)
                                         { return var.name == stmt_assign->ident.value.value(); });

                if (it == gen.m_vars.cend())
                {
                    std::cerr << "Error: Variable not declared: "
                              << stmt_assign->ident.value.value() << "\n";
                    exit(1);
                }

                llvm::Value* exprValue = gen.genExpression(stmt_assign->expr);

                gen.m_builder->CreateStore(exprValue, it->alloca);
            }

            void operator()(const NodeStmtLog* stmt_log) const
            {
            }

            void operator()(const NodeStmtLogLn* stmt_log_ln) const
            {
            }

            void operator()(const NodeStmtExit* stmt_exit) const
            {
                llvm::Value* exprV = gen.genExpression(stmt_exit->expr);

                llvm::Value* exitCode = gen.m_builder->CreateTrunc(
                    exprV, llvm::Type::getInt32Ty(*gen.m_context), "exitcode");

                llvm::Function* exitFunc = gen.m_module->getFunction("exit");
                if (!exitFunc)
                {
                    llvm::FunctionType* exitType =
                        llvm::FunctionType::get(llvm::Type::getVoidTy(*gen.m_context),
                                                {llvm::Type::getInt32Ty(*gen.m_context)}, false);

                    exitFunc = llvm::Function::Create(exitType, llvm::Function::ExternalLinkage,
                                                      "exit", gen.m_module.get());
                }

                gen.m_builder->CreateCall(exitFunc, {exitCode});
            }

            void operator()(const NodeScope* scope) const
            {
                gen.genScope(scope);
            }

            void operator()(const NodeStmtIf* stmt_if) const
            {
                // TODO: Implement if statements
            }
        };

        StmtVisitor visitor{.gen = *this};

        std::visit(visitor, stmt->var);
    }

    std::string genProgram()
    {
        for (const NodeStmt& stmt : m_prog.stmts)
        {
            genStatement(&stmt);
        }

        m_builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_context), 0));

        std::string ir;
        llvm::raw_string_ostream os(ir);
        m_module->print(os, nullptr);
        return ir;
    }

  private:
    void begin_scope()
    {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope()
    {
        const size_t pop_count = m_vars.size() - m_scopes.back();

        if (pop_count == 0)
            return;

        for (size_t i = 0; i < pop_count; i++)
        {
            m_vars.pop_back();
        }

        m_scopes.pop_back();
    }

    struct Var
    {
        std::string name;
        llvm::AllocaInst* alloca;
        TokenType type;
    };

    std::vector<Var> m_vars{};
    std::vector<size_t> m_scopes{};
    const NodeProg m_prog;

    std::unique_ptr<llvm::LLVMContext> m_context;
    std::unique_ptr<llvm::Module> m_module;
    std::unique_ptr<llvm::IRBuilder<>> m_builder;
    llvm::Function* m_mainFunction;
};
