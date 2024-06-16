#pragma once
#include <map>
#include <memory>
#include <ostream>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "lexer.h"

class TypeASTNode;

struct Symbol { llvm::AllocaInst* store; TypeASTNode* type; };
using SymbolTable = std::map<std::string, Symbol>;

struct GenContext {
    GenContext(const std::string& moduleName);

    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module module;

    SymbolTable symbTable;
};

class ASTNode {
public:
    virtual ~ASTNode();
    virtual void print(std::ostream& os, unsigned indent = 0) const = 0;
    void gen() const;
    virtual llvm::Value* codegen(GenContext& gen) const = 0;
};


class TypeASTNode : public ASTNode {
public:
    enum class Type { INT,
                      DOUBLE,
    };

    TypeASTNode(Type type);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;

private:
    Type m_type;
};


class ExprASTNode : public ASTNode {
public:
    virtual ~ExprASTNode();
};

class BinOpASTNode : public ExprASTNode {
    TokenType m_op;
    std::unique_ptr<ExprASTNode> m_lhs;
    std::unique_ptr<ExprASTNode> m_rhs;

public:
    BinOpASTNode(TokenType op, std::unique_ptr<ExprASTNode> lhs, std::unique_ptr<ExprASTNode> rhs);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};

class UnaryOpASTNode : public ExprASTNode {
    TokenType m_op;
    std::unique_ptr<ExprASTNode> m_expr;

public:
    UnaryOpASTNode(TokenType op, std::unique_ptr<ExprASTNode> expr);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};

class LiteralASTNode : public ExprASTNode {
    int64_t m_value;

public:
    LiteralASTNode(int64_t value);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};

class DeclRefASTNode : public ExprASTNode {
    std::string m_var;

public:
    DeclRefASTNode(std::string var);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
    llvm::AllocaInst* getStore(GenContext& gen) const;
};

class FunCallASTNode : public ExprASTNode {
    std::string m_func;
    std::vector<std::unique_ptr<ExprASTNode>> m_args;

public:
    FunCallASTNode(std::string func, std::vector<std::unique_ptr<ExprASTNode>> args);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};


class StatementASTNode : public ASTNode {
public:
    virtual ~StatementASTNode();
};

class IfASTNode : public StatementASTNode {
    std::unique_ptr<ExprASTNode> m_cond;
    std::unique_ptr<StatementASTNode> m_body;

public:
    IfASTNode(std::unique_ptr<ExprASTNode> cond, std::unique_ptr<StatementASTNode> body);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};

class VarDeclASTNode : public StatementASTNode {
    std::string m_var;
    std::unique_ptr<TypeASTNode> m_type;
    std::unique_ptr<ExprASTNode> m_expr;

public:
    VarDeclASTNode(std::string var, std::unique_ptr<TypeASTNode> type, std::unique_ptr<ExprASTNode> expr);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};

class AssignASTNode : public StatementASTNode {
    std::unique_ptr<DeclRefASTNode> m_var;
    std::unique_ptr<ExprASTNode> m_expr;

public:
    AssignASTNode(std::unique_ptr<DeclRefASTNode> var, std::unique_ptr<ExprASTNode> expr);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};

class ExpressionStatementASTNode : public StatementASTNode {
    std::unique_ptr<ExprASTNode> m_expr;

public:
    ExpressionStatementASTNode(std::unique_ptr<ExprASTNode> expr);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};

class ProgramASTNode : public ASTNode {
    std::vector<std::unique_ptr<StatementASTNode>> m_statements;

public:
    ProgramASTNode(std::vector<std::unique_ptr<StatementASTNode>> statements);
    void print(std::ostream& os, unsigned indent = 0) const override;
    llvm::Value* codegen(GenContext& gen) const override;
};
