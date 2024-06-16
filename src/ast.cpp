#include "ast.hpp"

GenContext::GenContext() : MilaContext(), MilaBuilder(MilaContext), MilaModule("mila", MilaContext) {}

void ASTNode::printIndent(int level) const
{
    for (int i = 0; i < level; ++i)
    {
        if (i == level - 1)
            std::cout << " -";
        else
            std::cout << "  ";
    }
}

void ExprASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Expression Node\n";
}

void StatementASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Statement Node\n";
}

void ConstantDeclarationASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Constant Declaration: " << m_variable << "\n";
    m_value->print(level + 1);
}

void VariableASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Variable: " << m_identifier << "\n";
}

void NumberASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Number: " << m_value << "\n";
}

void ProgramASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Program Node\n";
    for (const auto &stmt : m_statements)
    {
        stmt->print(level + 1);
    }
}

void BlockStatmentASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Block Node\n";
    for (const auto &expression : m_expresions)
    {
        expression->print(level + 1);
    }
}

void MainFunctionBlockStatementASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Main Block Node\n";
    for (const auto &expression : m_expresions)
    {
        expression->print(level + 1);
    }
}

void FunctinoCallExprASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Function Call Expression Node\n";
}

void BinaryOperationASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Binary Operation Expression Node " << m_operator << std::endl;
    m_LHS->print();
    m_RHS->print();
}
llvm::Value * BinaryOperationASTNode::codegen(GenContext &gen) const
{
    llvm::Value * LHS = m_LHS ->codegen(gen);
    llvm::Value * RHS = m_RHS -> codegen(gen);
    if(!LHS || !RHS)
        return nullptr;
    switch(m_operator)
    {
        case '+':
            return gen.MilaBuilder.CreateFAdd(LHS,RHS,"Add");
        case '-':
            return gen.MilaBuilder.CreateFSub(LHS,RHS,"Subtract");
        case '*':
            return gen.MilaBuilder.CreateFMul(LHS,RHS,"Multiply");
        case tok_div:
            return gen.MilaBuilder.CreateFDiv(LHS,RHS,"Div");
        default:
            throw std::logic_error("no operator");
    }
}


llvm::Value *ProgramASTNode::codegen(GenContext &gen) const
{
    for (auto &statement : m_statements)
        statement->codegen(gen);
    return nullptr;
}

llvm::Value *NumberASTNode::codegen(GenContext &gen) const
{
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.MilaContext), m_value);
}

llvm::Value *VariableASTNode::codegen(GenContext &gen) const
{
    llvm::Value * v = gen.symbolTable[m_identifier];
    if(!v)
        throw std::logic_error("Unkown Variable");
    return v;
}

llvm::Value *ConstantDeclarationASTNode::codegen(GenContext &gen) const
{
    if (gen.symbolTable.count(m_variable) > 0)
        throw std::logic_error("Variable already declared");

    llvm::Constant *constantValue = nullptr;
    if (m_value)
    {
        constantValue = llvm::dyn_cast<llvm::Constant>(m_value->codegen(gen));
        if (!constantValue)
            throw std::logic_error("constant must be defined");
    }
    else
    {
        constantValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.MilaContext), 0);
    }

    llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(gen.MilaModule, llvm::Type::getInt32Ty(gen.MilaContext),
                                                               true, llvm::GlobalValue::InternalLinkage, constantValue, m_variable);

    gen.symbolTable[m_variable] = globalVar;
    return globalVar;
}

llvm::Value * BlockStatmentASTNode::codegen(GenContext &gen) const
{
return nullptr;
}


llvm::Value * MainFunctionBlockStatementASTNode::codegen(GenContext &gen) const
{
    // create main function
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.MilaContext), false);
    llvm::Function *MainFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", gen.MilaModule);

    // block
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(gen.MilaContext, "entry", MainFunction);
    gen.MilaBuilder.SetInsertPoint(BB);

    for(auto & expression : m_expresions)
    {
        expression->codegen(gen);
    }

        // return 0
    gen.MilaBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.MilaContext), 0));
    
    return nullptr;
}

llvm::Value* FunctinoCallExprASTNode::codegen(GenContext & gen) const
{
    //lookup the fucntion name in the global table , not found > function not defined
    llvm::Function * calleeF = gen.MilaModule.getFunction(m_callee);
    if(!calleeF)
        throw std::logic_error("Function not defined");
    //check the argument matching
    if(calleeF->arg_size() != m_args.size())
        throw std::logic_error("Arguments Missmatch");
    std::vector<llvm::Value *> argsV;
    for(size_t i = 0 ; i < m_args.size() ; ++i)
    {
        argsV.push_back(m_args[i]->codegen(gen));
        if(!argsV.back())
            return nullptr; 
    }
    return gen.MilaBuilder.CreateCall(calleeF,argsV,"funcall");
}
