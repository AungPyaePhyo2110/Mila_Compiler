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
using ConstantValueTable = std::map<std::string, llvm::Constant *>;

class GenContext
{

public:
  GenContext();
  llvm::LLVMContext MilaContext; // llvm context
  llvm::IRBuilder<> MilaBuilder; // llvm builder
  llvm::Module MilaModule;       // llvm module
  SymbolTable symbolTable;
  ConstantValueTable constantTable;
};

class ASTNode
{
public:
  virtual ~ASTNode() = default;
  virtual void print(int level = 0) const = 0;
  virtual llvm::Value *codegen(GenContext &gen) const = 0;

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
  llvm::Value *codePtrGen(GenContext &gen) const;
  virtual void print(int level = 0) const override;
  llvm::AllocaInst *getStore(GenContext &gen) const;
};

class AssignmentASTNode : public ExprASTNode
{
  std::unique_ptr<VariableASTNode> m_variable;
  std::unique_ptr<ExprASTNode> m_expr;

public:
  AssignmentASTNode(std::unique_ptr<VariableASTNode> variable , std::unique_ptr<ExprASTNode> expression)
    :m_variable(std::move(variable)),m_expr(std::move(expression)) {}
  llvm::Value * codegen(GenContext & gen) const override;
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

class UnaryOperationASTNode : public ExprASTNode
{
  int m_operator;
  std::unique_ptr<ExprASTNode> m_expr;
  public:
    UnaryOperationASTNode(int op , std::unique_ptr<ExprASTNode> expression):
      m_operator(op),m_expr(std::move(expression)) {}
    llvm::Value * codegen(GenContext & gen) const override;
    virtual void print(int level =0) const override;
}


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

class ReadlnExprASTNode : public ExprASTNode
{
  std::unique_ptr<VariableASTNode> m_variable;

public:
  ReadlnExprASTNode(std::unique_ptr<VariableASTNode> variable) : m_variable(std::move(variable)) {}
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



class IfElseASTNode : public ExprASTNode
{
  std::unique_ptr<ExprASTNode> m_condition;
  std::unique_ptr<ASTNode> m_then;
  std::unique_ptr<ASTNode> m_else;
  public:
  IfElseASTNode(  std::unique_ptr<ExprASTNode> condition ,   std::unique_ptr<ASTNode> then ,   std::unique_ptr<ASTNode> elsebranch )
    : m_condition(std::move(condition)) , m_then(std::move(then)) , m_else(std::move(elsebranch)) {}
  virtual void print(int level = 0) const override;
  virtual llvm::Value * codegen(GenContext & gen) const override;
    
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
  int m_value;

public:
  ConstantDeclarationASTNode(std::string variable,
                             int value) : m_variable(variable), m_value(value) {}
  llvm::Value *codegen(GenContext &gen) const override;
  virtual void print(int level = 0) const override;
};

class VariableDeclarationASTNode : public StatementASTNode
{
  std::string m_variable;
  std::unique_ptr<ExprASTNode> m_value;

public:
  VariableDeclarationASTNode(std::string variable, std::unique_ptr<ExprASTNode> value) : m_variable(variable), m_value(std::move(value)) {}
  virtual void print(int level = 0) const override;
  virtual llvm::Value *codegen(GenContext &) const override;
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

class PrototypeASTNode : public ASTNode
{

  public:
  enum Type{ FUNCTION , PROCEDURE };

  PrototypeASTNode(std::string name , std::vector<std::string> args , Type type , std::unique_ptr<VariableDeclarationASTNode> returnValue) 
      :  m_type(type) , m_name(name) , m_args(std::move(args)) , m_returnValue(std::move(returnValue))  {}
  void print(int level = 0) const;
  const std::string & getName() { return m_name ;}
  llvm::Function * codegen(GenContext & gen) const;
  std::unique_ptr<VariableDeclarationASTNode> getReturnValue () { return std::move(m_returnValue) ;}
  Type m_type;
  private:
    std::string m_name;
    std::vector<std::string> m_args;
    std::unique_ptr<VariableDeclarationASTNode> m_returnValue;
};

class FunctionASTNode : public ASTNode
{
  std::unique_ptr<PrototypeASTNode> m_prototype;
  std::vector<std::unique_ptr<VariableDeclarationASTNode>> m_variables;
  std::vector<std::unique_ptr<ConstantDeclarationASTNode>> m_constants;
  std::unique_ptr<BlockStatmentASTNode> m_body;
  public:
    FunctionASTNode(std::unique_ptr<PrototypeASTNode> prototype ,std::vector<std::unique_ptr<VariableDeclarationASTNode>> variables,
                    std::vector<std::unique_ptr<ConstantDeclarationASTNode>> constants , std::unique_ptr<BlockStatmentASTNode> body):
                    m_prototype(std::move(prototype)),m_variables(std::move(variables)),m_constants(std::move(constants)),
                    m_body(std::move(body)) {}
    llvm::Function * codegen(GenContext & gen) const;
    void print(int level = 0) const override;
};

class ProgramASTNode : public ASTNode
{
  std::vector<std::unique_ptr<FunctionASTNode>> m_functions;

public:
  ProgramASTNode(std::vector<std::unique_ptr<FunctionASTNode>> functions) : m_functions(std::move(functions)) {}
  llvm::Value *codegen(GenContext &gen) const;
  virtual void print(int level = 0) const override;
};

#endif // PJPPROJECT_AST_HPP