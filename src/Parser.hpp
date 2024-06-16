#ifndef PJPPROJECT_PARSER_HPP
#define PJPPROJECT_PARSER_HPP

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

#include "Lexer.hpp"
#include "ast.hpp"
#include <map>
#include <iostream>

static std::map<int, std::string> tokenMap = {
            { ')' , ")"},
            { '(' , "("},
            { ';' , "semi_colon"},
            { 61 , "equal_token"},
            {-1, "tok_eof"},
            {-2, "tok_identifier"},
            {-3, "tok_number"},
            {-4, "tok_begin"},
            {-5, "tok_end"},
            {-6, "tok_const"},
            {-7, "tok_procedure"},
            {-8, "tok_forward"},
            {-9, "tok_function"},
            {-10, "tok_if"},
            {-11, "tok_then"},
            {-12, "tok_else"},
            {-13, "tok_program"},
            {-14, "tok_while"},
            {-15, "tok_exit"},
            {-16, "tok_var"},
            {-17, "tok_integer"},
            {-18, "tok_for"},
            {-19, "tok_do"},
            {-25, "tok_mod"},
            {-26, "tok_div"},
            {-27, "tok_not"},
            {-28, "tok_and"},
            {-29, "tok_xor"},
            {-30, "tok_to"},
            {-31, "tok_downto"},
            {-32, "tok_array"},
            {-33, "tok_range"}
};

static std::map<int, int> BinopPrecedence = 
{
    {'+',10},
    {'-',10},
    { '*',20},
    { tok_div,20}
};

class Parser {
public:
    Parser();
    ~Parser() = default;

    bool Parse();                    // parse
    const llvm::Module& Generate();  // generate

private:
    int getNextToken();
    void handleConstantDeclaration();
    
    void parseConstantDeclarationBlock(std::vector<std::unique_ptr<StatementASTNode>>&);
    void parseGlobalVariableDeclarationBLock(std::vector<std::unique_ptr<StatementASTNode>>&);

    std::unique_ptr<ConstantDeclarationASTNode> parseConstantDeclaration();
    std::unique_ptr<VariableDeclarationASTNode> parseVariableDeclaration();
    std::unique_ptr<VariableASTNode> parseVariable();
    std::unique_ptr<NumberASTNode> parseNumber();
    std::unique_ptr<ProgramASTNode> parseProgram();
    std::unique_ptr<BlockStatmentASTNode> parseBlockStatement();
    std::unique_ptr<MainFunctionBlockStatementASTNode> parseMainFunctionBlock();

    std::unique_ptr<ExprASTNode> parseIdentiferExpression();
    std::unique_ptr<ExprASTNode> parseExpression();
    std::unique_ptr<ExprASTNode> parsePrimary();
    std::unique_ptr<ExprASTNode> parseNumberExpression();
    std::unique_ptr<ExprASTNode> parseParentheseExpression();
    int GetTokPrecedence();
    std::unique_ptr<ExprASTNode> ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprASTNode> LHS);

    

    Lexer m_Lexer;                   // lexer is used to read tokens
    int CurTok;                      // to keep the current token
    std::unique_ptr<ProgramASTNode> astRoot;

    GenContext gen;
};

#endif //PJPPROJECT_PARSER_HPP
