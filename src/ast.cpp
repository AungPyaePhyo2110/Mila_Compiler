#include "ast.hpp"

GenContext::GenContext() : MilaContext(), MilaBuilder(MilaContext), MilaModule("mila", MilaContext) {}

void ASTNode::printIndent(int level) const
{
    for (int i = 0; i < level; ++i)
    {
        if (i == level - 1)
            std::cout << " |-";
        else
            std::cout << "  ";
    }
}

void WhileASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "While Node" << std::endl;
    m_condition->print(level + 1);
    m_body->print(level + 1);
}

void UnaryOperationASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Unary Operator node" << std::endl;
}

void IfElseASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "If else ast node " << std::endl;
    m_condition->print(level + 1);
    m_then->print(level + 1);
    if (m_else)
        m_else->print(level + 1);
}

void PrototypeASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Function prototype node" << std::endl;
}

void FunctionASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Function Node" << std::endl;
    m_prototype->print(level + 1);
    for (auto &variable : m_variables)
        variable->print(level + 1);
    for (auto &constant : m_constants)
        constant->print(level + 1);
    if (m_body)
        m_body->print(level + 1);
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
    std::cout << "Constant Declaration: " << m_variable << " as  " << m_value << "\n";
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

void AssignmentASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Assignment AST Node " << std::endl;
    m_variable->print(level + 1);
    m_expr->print(level + 1);
}

void ProgramASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Program Node\n";
    for (const auto &function : m_functions)
    {
        function->print(level + 1);
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

void FunctionCallExprASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Function Call Expression Node\n";
}

void ReadlnExprASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "ReadLine Expression Node\n";
}

void IncrementExprASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Increment Expression Node\n";
}

void DecrementExprASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Decrement Expression Node\n";
}

void BinaryOperationASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Binary Operation Expression Node " << m_operator << std::endl;
    m_LHS->print(level + 1);
    m_RHS->print(level + 1);
}

void VariableDeclarationASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Variable Declaration Node " << std::endl;
    if (m_value)
        m_value->print(level + 1);
}

void FunctionExitASTNode::print(int level) const
{
    printIndent(level);
    std::cout << "Function Exit Node " << std::endl;
}

llvm::Value *FunctionExitASTNode::codegen(GenContext &gen) const
{
    llvm::Function *TheFunction = gen.MilaBuilder.GetInsertBlock()->getParent();
    gen.MilaBuilder.CreateBr(gen.endBlock);
    return nullptr;
}

llvm::Value *UnaryOperationASTNode::codegen(GenContext &gen) const
{
    llvm::Value *expression = m_expr->codegen(gen);
    if (!expression)
        return nullptr;
    if (m_operator == '+')
        return expression;
    else
        return gen.MilaBuilder.CreateSub(llvm::ConstantInt::get(gen.MilaContext, llvm::APInt(32, 0)), expression, "UnaryMinus");
}

llvm::Value * IncrementExprASTNode::codegen(GenContext&gen) const
{
    llvm::Value * LHS = m_variable->codegen(gen);
    llvm::Value * incrementValue = gen.MilaBuilder.CreateAdd(LHS,llvm::ConstantInt::get(gen.MilaContext,llvm::APInt(32,1)));
    gen.MilaBuilder.CreateStore(incrementValue,m_variable->getStore(gen));
    return nullptr;
}

llvm::Value * DecrementExprASTNode::codegen(GenContext&gen) const
{
    llvm::Value * LHS = m_variable->codegen(gen);
    llvm::Value * incrementValue = gen.MilaBuilder.CreateSub(LHS,llvm::ConstantInt::get(gen.MilaContext,llvm::APInt(32,1)));
    gen.MilaBuilder.CreateStore(incrementValue,m_variable->getStore(gen));
    return nullptr;
}


llvm::Value *BinaryOperationASTNode::codegen(GenContext &gen) const
{
    llvm::Value *LHS = m_LHS->codegen(gen);
    llvm::Value *RHS = m_RHS->codegen(gen);
    if (!LHS || !RHS)
        return nullptr;
    switch (m_operator)
    {
    case tok_notequal:
        return gen.MilaBuilder.CreateICmpNE(LHS,RHS,"NotEqual");
    case tok_or:
        return gen.MilaBuilder.CreateOr(LHS, RHS, "Or");
    case tok_and:
        return gen.MilaBuilder.CreateAnd(LHS, RHS, "And");
    case '=':
        return gen.MilaBuilder.CreateICmpEQ(LHS, RHS, "Equal");
    case '+':
        return gen.MilaBuilder.CreateAdd(LHS, RHS, "Add");
    case '-':
        return gen.MilaBuilder.CreateSub(LHS, RHS, "Subtract");
    case '*':
        return gen.MilaBuilder.CreateMul(LHS, RHS, "Multiply");
    case tok_div:
        return gen.MilaBuilder.CreateSDiv(LHS, RHS, "Div");
    case tok_mod:
        return gen.MilaBuilder.CreateSRem(LHS, RHS, "Mod");
    case '<':
        return gen.MilaBuilder.CreateICmpSLT(LHS, RHS, "LessThan");
    case '>':
        return gen.MilaBuilder.CreateICmpSGT(LHS, RHS, "GreaterThan");
    case tok_lessequal:
        return gen.MilaBuilder.CreateICmpSLE(LHS, RHS, "lessthanEqual");
    case tok_greaterequal:
        return gen.MilaBuilder.CreateICmpSGE(LHS, RHS, "GreterthanEqual");
    default:
        throw std::logic_error("no operator");
    }
}

llvm::Value *ProgramASTNode::codegen(GenContext &gen) const
{
    for (auto &function : m_functions)
    {
        function->codegen(gen);
    }
    return nullptr;
}

llvm::Value *NumberASTNode::codegen(GenContext &gen) const
{
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.MilaContext), m_value);
}

llvm::Value *AssignmentASTNode::codegen(GenContext &gen) const
{
    auto store = m_variable->getStore(gen);
    auto e = m_expr->codegen(gen);
    return gen.MilaBuilder.CreateStore(e, store);
}

llvm::Value *VariableDeclarationASTNode::codegen(GenContext &gen) const
{
    if (gen.symbolTable.count(m_variable) > 0)
        throw std::logic_error("Variable already declared");

    llvm::AllocaInst *store = gen.MilaBuilder.CreateAlloca(llvm::Type::getInt32Ty(gen.MilaContext), nullptr, m_variable);
    gen.symbolTable[m_variable] = store;
    if (m_value)
    {
        auto e = m_value->codegen(gen);
        gen.MilaBuilder.CreateStore(e, store);
    }
    return nullptr;
}

llvm::Value *VariableASTNode::codePtrGen(GenContext &gen) const
{
    if (auto it = gen.symbolTable.find(m_identifier); it != gen.symbolTable.end())
    {
        return it->second;
    }
    return nullptr;
}

llvm::Value *VariableASTNode::codegen(GenContext &gen) const
{
    if (gen.symbolTable.count(m_identifier) <= 0 && gen.constantTable.count(m_identifier) <= 0)
        throw std::logic_error("variable not defined");
    if (gen.constantTable.count(m_identifier) > 0)
    {
        return gen.constantTable[m_identifier];
    }
    // if (llvm::isa<llvm::Argument>(gen.symbolTable[m_identifier]))
    // {
    //     return gen.symbolTable[m_identifier];
    // }

    llvm::AllocaInst *store = static_cast<llvm::AllocaInst *>(gen.symbolTable[m_identifier]);

    if (auto it = gen.symbolTable.find(m_identifier); it != gen.symbolTable.end())
    {
        return gen.MilaBuilder.CreateLoad(llvm::Type::getInt32Ty(gen.MilaContext), it->second , m_identifier);
    }
    return nullptr;
}

llvm::AllocaInst *VariableASTNode::getStore(GenContext &gen) const
{
    if (auto it = gen.symbolTable.find(m_identifier); it != gen.symbolTable.end())
    {
        return static_cast<llvm::AllocaInst *>(it->second);
    }
    else
    {
        throw std::logic_error("var not declared");
    }
}

llvm::Function *PrototypeASTNode::codegen(GenContext &gen) const
{
    if (m_name == "main")
    {
        llvm::FunctionType *functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.MilaContext), false);
        llvm::Function *mainFunction = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", gen.MilaModule);
        // block
        return mainFunction;
    }

    std::vector<llvm::Type *> Arguments(m_args.size(), llvm::Type::getInt32Ty(gen.MilaContext));
    llvm::FunctionType *FT = nullptr;
    if (m_type == PrototypeASTNode::PROCEDURE)
    {
        FT = llvm::FunctionType::get(llvm::Type::getVoidTy(gen.MilaContext), Arguments, false);
    }
    else
    {
        FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.MilaContext), Arguments, false);
    }

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_name, gen.MilaModule);




    unsigned index = 0;
    for (auto &Arg : F->args())
        Arg.setName(m_args[index++]);
    return F;
}

llvm::Function *FunctionASTNode::codegen(GenContext &gen) const
{

    gen.endBlock = nullptr;
    llvm::Function *function = gen.MilaModule.getFunction(m_prototype->getName());
    if (!function)
    {
        function = m_prototype->codegen(gen);
    }

    // llvm::Function *function = m_prototype->codegen(gen);

    if (!function)
        return nullptr;

    if (!m_body)
        return function;

    if (m_prototype->getName() == "main")
    {
        llvm::BasicBlock *BB = llvm::BasicBlock::Create(gen.MilaContext, "entry", function);
        gen.MilaBuilder.SetInsertPoint(BB);
        gen.symbolTable.clear();
        gen.constantTable.clear();
        for (auto &variable : m_variables)
        {
            variable->codegen(gen);
        }
        for (auto &constant : m_constants)
        {
            constant->codegen(gen);
        }
        m_body->codegen(gen);
        // return 0
        gen.MilaBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.MilaContext), 0));
        return function;
    }

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(gen.MilaContext, "entry", function);
    llvm::BasicBlock *endBB = llvm::BasicBlock::Create(gen.MilaContext, "end", function);
    gen.endBlock = endBB;
    gen.MilaBuilder.SetInsertPoint(BB);
    gen.symbolTable.clear();
    gen.constantTable.clear();
    for (auto &Arg : function->args())
    {
        llvm::AllocaInst * arg = gen.MilaBuilder.CreateAlloca(Arg.getType(),nullptr,Arg.getName());
        gen.MilaBuilder.CreateStore(&Arg,arg);
        gen.symbolTable[std::string(Arg.getName())] = arg;
    }
    m_prototype->getReturnValue()->codegen(gen);
    for (auto &variable : m_variables)
        variable->codegen(gen);
    for (auto &constant : m_constants)
        constant->codegen(gen);
    m_body->codegen(gen);
    gen.MilaBuilder.CreateBr(endBB);

    if (m_prototype->m_type == PrototypeASTNode::PROCEDURE)
    {
        gen.MilaBuilder.SetInsertPoint(endBB);
        gen.MilaBuilder.CreateRetVoid();
        llvm::verifyFunction(*function);
        return function;
    }

    // if(llvm::Value * RetVal = m_body->codegen(gen))
    // {
    //     gen.MilaBuilder.CreateRet(RetVal);
    //     llvm::verifyFunction(*function);
    //     return function;
    // }
    auto it = gen.symbolTable.find(m_prototype->getName());
    gen.MilaBuilder.SetInsertPoint(endBB);
    llvm::Value *retValue = gen.MilaBuilder.CreateLoad(llvm::Type::getInt32Ty(gen.MilaContext), it->second);
    gen.MilaBuilder.CreateRet(retValue);
    llvm::verifyFunction(*function);
    return function;

    // function->eraseFromParent();
    // return nullptr;
}

llvm::Value *ConstantDeclarationASTNode::codegen(GenContext &gen) const
{
    if (gen.symbolTable.count(m_variable) > 0)
        throw std::logic_error("Variable already declared");

    llvm::Constant *constantValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.MilaContext), m_value);
    gen.constantTable[m_variable] = constantValue;
    return nullptr;
}

llvm::Value *BlockStatmentASTNode::codegen(GenContext &gen) const
{
    for (auto &expression : m_expresions)
    {
        expression->codegen(gen);
    }

    return nullptr;
}

llvm::Value *MainFunctionBlockStatementASTNode::codegen(GenContext &gen) const
{

    for (auto &expression : m_expresions)
    {
        expression->codegen(gen);
    }

    return nullptr;
}


llvm::Value *ReadlnExprASTNode::codegen(GenContext &gen) const
{
    llvm::Function *calleeF = gen.MilaModule.getFunction("readln");
    std::vector<llvm::Value *> argsV;
    argsV.push_back(m_variable->codePtrGen(gen));
    return gen.MilaBuilder.CreateCall(calleeF, argsV, "readln");
}

llvm::Value *FunctionCallExprASTNode::codegen(GenContext &gen) const
{
    // lookup the fucntion name in the global table , not found > function not defined
    llvm::Function *calleeF = gen.MilaModule.getFunction(m_callee);
    if (!calleeF)
        throw std::logic_error("Function not defined");
    // check the argument matching
    if (calleeF->arg_size() != m_args.size())
        throw std::logic_error("Arguments Missmatch");
    std::vector<llvm::Value *> argsV;
    for (size_t i = 0; i < m_args.size(); ++i)
    {
        argsV.push_back(m_args[i]->codegen(gen));
        if (!argsV.back())
            return nullptr;
    }
    if(calleeF->getFunctionType()->getReturnType()->isVoidTy())
    {
        return gen.MilaBuilder.CreateCall(calleeF, argsV);
        // Return null for void functions
    }

    return gen.MilaBuilder.CreateCall(calleeF, argsV, m_callee);
}

llvm::Value *WhileASTNode::codegen(GenContext &gen) const
{
    gen.whileContinueBlock = nullptr;
    llvm::Function *TheFunction = gen.MilaBuilder.GetInsertBlock()->getParent();
    llvm::BasicBlock *conditionBB = llvm::BasicBlock::Create(gen.MilaContext, "whileCond", TheFunction);
    llvm::BasicBlock *whileContinueBB = llvm::BasicBlock::Create(gen.MilaContext, "merge: ", TheFunction);
    gen.whileContinueBlock = whileContinueBB;
    gen.MilaBuilder.CreateBr(conditionBB);
    gen.MilaBuilder.SetInsertPoint(conditionBB);
    llvm::Value *condition = m_condition->codegen(gen);
    if (!condition)
        return nullptr;
    condition = gen.MilaBuilder.CreateICmpNE(condition, llvm::ConstantInt::get(gen.MilaContext, llvm::APInt(1, 0)), "whileCond");
    llvm::BasicBlock *whileBodyBB = llvm::BasicBlock::Create(gen.MilaContext, "body: ", TheFunction);
    gen.MilaBuilder.CreateCondBr(condition, whileBodyBB, whileContinueBB);
    gen.MilaBuilder.SetInsertPoint(whileBodyBB);
    m_body->codegen(gen);
    gen.MilaBuilder.CreateBr(conditionBB);
    gen.MilaBuilder.SetInsertPoint(whileContinueBB);

    return nullptr;
}

llvm::Value *IfElseASTNode::codegen(GenContext &gen) const
{
    llvm::Value *condition = m_condition->codegen(gen);
    if (!condition)
        return nullptr;
    // condition = gen.MilaBuilder.CreateZExt(condition,llvm::Type::getInt32Ty(gen.MilaContext));
    condition = gen.MilaBuilder.CreateICmpNE(condition, llvm::ConstantInt::get(gen.MilaContext, llvm::APInt(1, 0)), "ifcond");
    llvm::Function *TheFunction = gen.MilaBuilder.GetInsertBlock()->getParent();

    llvm::BasicBlock *ThenBB =
        llvm::BasicBlock::Create(gen.MilaContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = nullptr;
    ElseBB = llvm::BasicBlock::Create(gen.MilaContext, "else", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(gen.MilaContext, "ifcont", TheFunction);
    gen.MilaBuilder.CreateCondBr(condition, ThenBB, ElseBB);
    gen.MilaBuilder.SetInsertPoint(ThenBB);

    llvm::Value *ThenV = m_then->codegen(gen);
    gen.MilaBuilder.CreateBr(MergeBB);

    //   // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    //   ThenBB = gen.MilaBuilder.GetInsertBlock();

    gen.MilaBuilder.SetInsertPoint(ElseBB);
    if (m_else)
        llvm::Value *elseV = m_else->codegen(gen);
    gen.MilaBuilder.CreateBr(MergeBB);
    gen.MilaBuilder.SetInsertPoint(MergeBB);
    return nullptr;
}