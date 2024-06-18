#include "Parser.hpp"
#include "ast.hpp"

Parser::Parser()
{
}

bool Parser::Parse()
{
    // getNextToken();
    // while(CurTok!=tok_eof)
    //     getNextToken();
    astRoot = parseProgram();
    // astRoot->print(1); 
    if (!astRoot)
        return false;
    return true;
}

std::unique_ptr<BlockStatmentASTNode> Parser::parseBlockStatement()
{
    std::vector<std::unique_ptr<ExprASTNode>> expressions;
    while (CurTok != tok_end)
    {
        switch (CurTok)
        {
        case tok_identifier:
            std::unique_ptr<ExprASTNode> expression = parseIdentiferExpression();
            expressions.push_back(std::move(expression));
        }
    }
    return std::make_unique<BlockStatmentASTNode>(std::move(expressions));
}

void Parser::parseVariableDeclarationBLock(std::vector<std::unique_ptr<VariableDeclarationASTNode>> &statements)
{
    while (CurTok == tok_identifier)
    {
        std::unique_ptr<VariableDeclarationASTNode> declaration = parseVariableDeclaration();
        statements.push_back(std::move(declaration));
    }
}

void Parser::parseConstantDeclarationBlock(std::vector<std::unique_ptr<ConstantDeclarationASTNode>> &statments)
{
    while (CurTok == tok_identifier)
    {
        std::unique_ptr<ConstantDeclarationASTNode> declaration = parseConstantDeclaration();
        statments.push_back(std::move(declaration));
    }
}

std::unique_ptr<ExprASTNode> Parser::parseNumberExpression()
{
    std::unique_ptr<NumberASTNode> result = std::make_unique<NumberASTNode>(m_Lexer.numVal());
    getNextToken(); // eat number
    return result;
}

std::unique_ptr<ExprASTNode> Parser::parseParentheseExpression()
{
    getNextToken(); // eat (
    std::unique_ptr<ExprASTNode> expression = parseExpression();
    if (!expression)
        return nullptr;
    if (CurTok != ')')
        throw std::logic_error("Missing paranthesis");
    getNextToken(); // eat )
    return expression;
}

std::unique_ptr<ExprASTNode> Parser::parsePrimary()
{
    switch (CurTok)
    {
    default:
        throw std::logic_error("unknown token when expecting an expression");
    case tok_identifier:
        return parseIdentiferExpression();
    case tok_number:
        return parseNumberExpression();
    case '(':
        return parseParentheseExpression();
    // case '-':
    //     return parseUnaryExpression();
    // case '+':
    //     return parseUnaryExpression();
    }
}

int Parser::GetTokPrecedence()
{
    int currentTokenPrecedence = BinopPrecedence[CurTok];
    if (currentTokenPrecedence <= 0)
        return -1;
    return currentTokenPrecedence;
}

std::unique_ptr<ExprASTNode> Parser::ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprASTNode> LHS)
{
    // If this is a binop, find its precedence.
    while (true)
    {
        int TokPrec = GetTokPrecedence();

        // if current token is binary operator we need to continue , otherwise
        //  parsing the expression is done
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = parsePrimary();
        if (!RHS)
            return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec)
        {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        // Merge LHS/RHS.
        LHS =
            std::make_unique<BinaryOperationASTNode>(BinOp, std::move(LHS), std::move(RHS));
    }
}

std::unique_ptr<ExprASTNode> Parser::parseExpression()
{
    auto LHS = parsePrimary();
    if (!LHS)
        return nullptr;
    return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ExprASTNode> Parser::parseAssignemntExpression(const std::string identifier)
{
    std::unique_ptr<VariableASTNode> variable = std::make_unique<VariableASTNode>(identifier);
    getNextToken(); // eat assigment;
    std::unique_ptr<ExprASTNode> expression = parseExpression();
    return std::make_unique<AssignmentASTNode>(std::move(variable), std::move(expression));
}

std::unique_ptr<ExprASTNode> Parser::parseIdentiferExpression()
{
    std::string identifier = m_Lexer.identifierStr();
    getNextToken(); // eat identifier

    if (CurTok == tok_assign)
        return parseAssignemntExpression(identifier);

    if (CurTok != '(')
    {
        return std::make_unique<VariableASTNode>(identifier); // just a variable
    }
    if (identifier == "readln")
    {
        getNextToken(); // eat (
        std::unique_ptr<VariableASTNode> arg = parseVariable();
        getNextToken(); // eat )
        return std::make_unique<ReadlnExprASTNode>(std::move(arg));
    }

    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprASTNode>> args;
    if (CurTok != ')')
    {
        while (true)
        {
            if (std::unique_ptr<ExprASTNode> arg = parseExpression())
                args.push_back(std::move(arg));
            else
                return nullptr;
            if (CurTok == ')')
                break;
            if (CurTok != ',')
                throw std::logic_error("Arguemnt should be separated by comma");
            getNextToken();
        }
    }
    getNextToken(); // eat )
    return std::make_unique<FunctinoCallExprASTNode>(identifier, std::move(args));
}

std::unique_ptr<ExprASTNode> Parser::parseIfElseExpression()
{
    getNextToken(); // eat if
    std::unique_ptr<ExprASTNode> condition = parseExpression();
    getNextToken(); // eat then
    std::unique_ptr<ExprASTNode> then = parseIdentiferExpression();
    getNextToken(); // eat else
    std::unique_ptr<ExprASTNode> elseBranch = parseIdentiferExpression();
    return std::make_unique<IfElseASTNode> (std::move(condition),std::move(then),std::move(elseBranch));
}

std::unique_ptr<BlockStatmentASTNode> Parser::parseMainFunctionBlock()
{
    std::vector<std::unique_ptr<ExprASTNode>> expressions;
    while (CurTok != tok_end)
    {
        std::unique_ptr<ExprASTNode> expression = nullptr;
        switch (CurTok)
        {
        case tok_if:
            expression = parseIfElseExpression();
            expressions.push_back(std::move(expression));
            getNextToken(); // eat;
            break;
        case tok_identifier:
            expression = parseIdentiferExpression();
            expressions.push_back(std::move(expression));
            getNextToken(); // eat ;
            break;
        }
    }
    return std::make_unique<BlockStatmentASTNode>(std::move(expressions));
}

std::unique_ptr<VariableASTNode> Parser::parseVariable()
{
    if (CurTok != tok_identifier)
        return nullptr;
    const std::string identitfier = m_Lexer.identifierStr();
    getNextToken(); // eat the identitifer
    return std::make_unique<VariableASTNode>(identitfier);
}

std::unique_ptr<NumberASTNode> Parser::parseNumber()
{
    if (CurTok != tok_number)
        return nullptr;
    int value = m_Lexer.numVal();
    getNextToken(); // eat the number
    return std::make_unique<NumberASTNode>(value);
}

std::unique_ptr<VariableDeclarationASTNode> Parser::parseVariableDeclaration()
{
    std::string variable = m_Lexer.identifierStr();
    getNextToken(); // eat identifier
    if (CurTok != ':')
        return nullptr;
    getNextToken(); // eat :
    if (CurTok != tok_integer)
        return nullptr;
    getNextToken(); // eat integer
    getNextToken(); // eat ;
    return std::make_unique<VariableDeclarationASTNode>(variable, nullptr);
}

std::unique_ptr<ConstantDeclarationASTNode> Parser::parseConstantDeclaration()
{
    std::string variable = m_Lexer.identifierStr();
    getNextToken(); // eat identifier
    if (CurTok != '=')
        return nullptr;
    getNextToken(); // eat =
    int value = m_Lexer.numVal();
    getNextToken();
    if (CurTok != ';')
        return nullptr;
    getNextToken(); // eat ;
    return std::make_unique<ConstantDeclarationASTNode>(variable, value);
}

std::string Parser::parseFunctionParameter()
{
    std::string paraName = m_Lexer.identifierStr();
    getNextToken(); // eat para
    getNextToken(); // eat :
    getNextToken(); // eat integer
    return paraName;
}

std::unique_ptr<VariableDeclarationASTNode> Parser::parseReturnValue()
{
    getNextToken(); // eat identifier
    std::string retrunValueName = m_Lexer.identifierStr();
    return std::make_unique<VariableDeclarationASTNode>(retrunValueName,nullptr);
}

std::unique_ptr<PrototypeASTNode> Parser::parseProtoType()
{
    int tokenType = CurTok;
    getNextToken(); // eat function
    std::string functionName = m_Lexer.identifierStr();
    std::unique_ptr<VariableDeclarationASTNode> returnValue = parseReturnValue();
    std::vector<std::string> parameters;
    while (CurTok != ')')
    {
        getNextToken();
        parameters.push_back(parseFunctionParameter());

    }
    getNextToken(); // eat )
    if (tokenType == tok_function)
    {
        getNextToken(); // eat :
        getNextToken(); // eat integer;
        getNextToken(); // eat ;
        return std::make_unique<PrototypeASTNode>(functionName, parameters, PrototypeASTNode::FUNCTION , std::move(returnValue));
    }
    else if (tokenType == tok_procedure)
    {
        getNextToken(); // eat ;
        return std::make_unique<PrototypeASTNode>(functionName, parameters, PrototypeASTNode::PROCEDURE , std::move(returnValue));
    }
    return nullptr;
}

std::unique_ptr<FunctionASTNode> Parser::parseFunction()
{
    std::unique_ptr<PrototypeASTNode> prototype = parseProtoType();
    std::vector<std::unique_ptr<VariableDeclarationASTNode>> variables;
    std::vector<std::unique_ptr<ConstantDeclarationASTNode>> constants;
    while (CurTok == tok_var || CurTok == tok_const)
    {    
        if (CurTok == tok_const)
        {
            getNextToken();
            parseConstantDeclarationBlock(constants);
        }
        if (CurTok == tok_var)
        {
            getNextToken();
            parseVariableDeclarationBLock(variables);
        }
    }

    getNextToken(); // eat begin
    std::unique_ptr<BlockStatmentASTNode> mainBlock = parseMainFunctionBlock();
    getNextToken(); // eat end
    getNextToken(); // eat semicolon
    return std::make_unique<FunctionASTNode>(std::move(prototype), std::move(variables), std::move(constants), std::move(mainBlock));
}

std::unique_ptr<FunctionASTNode> Parser::parseMainFunction()
{
    std::unique_ptr<PrototypeASTNode> prototype = std::make_unique<PrototypeASTNode>("main", std::vector<std::string>(), PrototypeASTNode::FUNCTION,nullptr);
    std::vector<std::unique_ptr<VariableDeclarationASTNode>> variables;
    std::vector<std::unique_ptr<ConstantDeclarationASTNode>> constants;
    while (CurTok == tok_var || CurTok == tok_const)
    {
        if (CurTok == tok_const)
        {
            getNextToken();
            parseConstantDeclarationBlock(constants);
        }
        if (CurTok == tok_var)
        {
            getNextToken();
            parseVariableDeclarationBLock(variables);
        }
    }
    getNextToken(); // eat begin    
    std::unique_ptr<BlockStatmentASTNode> mainBlock = parseMainFunctionBlock();
    getNextToken(); // eat end
    getNextToken(); // eat .

    return std::make_unique<FunctionASTNode>(std::move(prototype), std::move(variables), std::move(constants), std::move(mainBlock));
}

std::unique_ptr<ProgramASTNode> Parser::parseProgram()
{
    if (getNextToken() != tok_program)
        return nullptr;
    if (getNextToken() != tok_identifier)
        return nullptr;
    if (getNextToken() != ';')
        return nullptr;
    getNextToken();

    std::vector<std::unique_ptr<FunctionASTNode>> functions;
    while (true)
    {
        if (CurTok == tok_eof)
            break;
        std::unique_ptr<FunctionASTNode> function = nullptr;
        switch (CurTok)
        {
        case tok_function:
            function = parseFunction();
            functions.push_back(std::move(function));
            break;
        case tok_procedure:
            break;
        default:
            function = parseMainFunction();
            functions.push_back(std::move(function));
            break;
        }
    }

    return std::make_unique<ProgramASTNode>(std::move(functions));
}

const llvm::Module &Parser::Generate()
{

    // create writeln function
    {
        std::vector<llvm::Type *> Ints(1, llvm::Type::getInt32Ty(gen.MilaContext));
        llvm::FunctionType *writelnFT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.MilaContext), Ints, false);
        llvm::Function *writelnF = llvm::Function::Create(writelnFT, llvm::Function::ExternalLinkage, "writeln", gen.MilaModule);
        for (auto &Arg : writelnF->args())
            Arg.setName("x");
    }

    {
        std::vector<llvm::Type *> Ints(1, llvm::Type::getInt32PtrTy(gen.MilaContext));
        llvm::FunctionType *readlnFT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.MilaContext), Ints, false);
        llvm::Function *readlnF = llvm::Function::Create(readlnFT, llvm::Function::ExternalLinkage, "readln", gen.MilaModule);
        for (auto &Arg : readlnF->args())
            Arg.setName("x");
    }

    astRoot->codegen(gen);

    return this->gen.MilaModule;
}

/**
 * @brief Simple token buffer.
 *
 * CurTok is the current token the parser is looking at
 * getNextToken reads another token from the lexer and updates curTok with ts result
 * Every function in the parser will assume that CurTok is the cureent token that needs to be parsed
 */
int Parser::getNextToken()
{
    CurTok = m_Lexer.gettok();
    // std::cout << CurTok << " - " << tokenMap[CurTok] << " " << std::endl;
    return CurTok;
}
