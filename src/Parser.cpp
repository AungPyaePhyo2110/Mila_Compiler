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
    astRoot->print(1);
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

void Parser::parseConstantDeclarationBlock(std::vector<std::unique_ptr<StatementASTNode>> &statments)
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
    if (expression)
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
    }
}

int Parser::GetTokPrecedence()
{
    if (!isascii(CurTok))
        return -1;
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

std::unique_ptr<ExprASTNode> Parser::parseIdentiferExpression()
{
    std::string identifier = m_Lexer.identifierStr();
    getNextToken(); // eat identifier
    if (CurTok != '(')
    {
        return std::make_unique<VariableASTNode>(identifier); // just a variable
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

std::unique_ptr<MainFunctionBlockStatementASTNode> Parser::parseMainFunctionBlock()
{
    std::vector<std::unique_ptr<ExprASTNode>> expressions;
    while (CurTok != tok_end)
    {
        switch (CurTok)
        {
        case tok_identifier:
            std::unique_ptr<ExprASTNode> expression = parseIdentiferExpression();
            expressions.push_back(std::move(expression));
            getNextToken(); // eat ;
            break;
        }
    }
    return std::make_unique<MainFunctionBlockStatementASTNode>(std::move(expressions));
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

    std::vector<std::unique_ptr<StatementASTNode>> statements;
    while (true)
    {
        if (CurTok == tok_eof)
            break;

        switch (CurTok)
        {
        case tok_const:
            getNextToken(); // eat const token
            parseConstantDeclarationBlock(statements);
            break;
        case tok_begin:
            getNextToken(); // eat begin
            std::unique_ptr<MainFunctionBlockStatementASTNode> mainBlock = parseMainFunctionBlock();
            statements.push_back(std::move(mainBlock));
            getNextToken(); // eat end
            getNextToken(); // eat .
            break;
        }
    }

    return std::make_unique<ProgramASTNode>(std::move(statements));
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
    double value = m_Lexer.numVal();
    getNextToken(); // eat the number
    return std::make_unique<NumberASTNode>(value);
}

std::unique_ptr<ConstantDeclarationASTNode> Parser::parseConstantDeclaration()
{
    std::string variable = m_Lexer.identifierStr();
    getNextToken(); // eat identifier
    if (CurTok != '=')
        return nullptr;
    getNextToken(); // eat =
    std::unique_ptr<NumberASTNode> value = parseNumber();
    if (CurTok != ';')
        return nullptr;
    getNextToken(); // eat ;
    return std::make_unique<ConstantDeclarationASTNode>(variable, std::move(value));
}

const llvm::Module &Parser::Generate()
{

    // create writeln function
    std::vector<llvm::Type *> Ints(1, llvm::Type::getInt32Ty(gen.MilaContext));
    llvm::FunctionType *writelnFT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.MilaContext), Ints, false);
    llvm::Function *writelnF = llvm::Function::Create(writelnFT, llvm::Function::ExternalLinkage, "writeln", gen.MilaModule);
    for (auto &Arg : writelnF->args())
        Arg.setName("x");

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
    std::cout << tokenMap[CurTok] << " " << std::endl;
    return CurTok;
}
