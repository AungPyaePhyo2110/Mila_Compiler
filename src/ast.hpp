#ifndef PJPPROJECT_AST_HPP
#define PJPPROJECT_AST_HPP

#include "Lexer.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <map>

#include <vector>
using SymbolTable = std::map<std::string, llvm::Value *>;

class GenContext
{

public:
  GenContext();
  llvm::LLVMContext MilaContext; // llvm context
  llvm::IRBuilder<> MilaBuilder; // llvm builder
  llvm::Module MilaModule;       // llvm module
  SymbolTable symbolTable;
};

class ASTNode
{
public:
  virtual ~ASTNode() = default;
  virtual llvm::Value *codegen(GenContext &gen) const = 0;
  virtual void print(int level = 0) const = 0;

protected:
  void printIndent(int level) const;
};

// expressions
class ExprASTNode : public ASTNode
{
public:
  virtual ~ExprASTNode(){};
  virtual void print(int level = 0) const override;
};

class VariableASTNode : public ExprASTNode
{
  const std::string m_identifier;

public:
  VariableASTNode(const std::string &name) : m_identifier(name) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

class NumberASTNode : public ExprASTNode
{
  int m_value;

public:
  NumberASTNode(int value) : m_value(value) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

class BinaryOperationASTNode : public ExprASTNode
{
  int m_operator;
  std::unique_ptr<ExprASTNode> m_LHS;
  std::unique_ptr<ExprASTNode> m_RHS;

public:
  BinaryOperationASTNode(int operatorType, std::unique_ptr<ExprASTNode> LHS, std::unique_ptr<ExprASTNode> RHS)
      : m_operator(operatorType), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

class FunctinoCallExprASTNode : public ExprASTNode
{
  std::string m_callee;
  std::vector<std::unique_ptr<ExprASTNode>> m_args;

public:
  FunctinoCallExprASTNode(std::string callee, std::vector<std::unique_ptr<ExprASTNode>> args) : m_callee(callee), m_args(std::move(args)) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

// statements
class StatementASTNode : public ASTNode
{
public:
  virtual ~StatementASTNode(){};
  virtual void print(int level = 0) const override;
};

class ConstantDeclarationASTNode : public StatementASTNode
{
  std::string m_variable;
  std::unique_ptr<ExprASTNode> m_value;

public:
  ConstantDeclarationASTNode(std::string variable,
                             std::unique_ptr<ExprASTNode> value) : m_variable(variable), m_value(std::move(value)) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

class BlockStatmentASTNode : public StatementASTNode
{
  std::vector<std::unique_ptr<ExprASTNode>> m_expresions;

public:
  BlockStatmentASTNode(std::vector<std::unique_ptr<ExprASTNode>> expresions) : m_expresions(std::move(expresions)) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

class MainFunctionBlockStatementASTNode : public StatementASTNode
{
  std::vector<std::unique_ptr<ExprASTNode>> m_expresions;

public:
  MainFunctionBlockStatementASTNode(std::vector<std::unique_ptr<ExprASTNode>> expresions) : m_expresions(std::move(expresions)) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

//

class ProgramASTNode : public ASTNode
{
  std::vector<std::unique_ptr<StatementASTNode>> m_statements;

public:
  ProgramASTNode(std::vector<std::unique_ptr<StatementASTNode>> statments) : m_statements(std::move(statments)) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

#endif // PJPPROJECT_AST_HPP