#include <llvm/ADT/APFloat.h>
#include <ostream>
#include "ast.h"


GenContext::GenContext(const std::string& moduleName)
    : builder(ctx)
    , module(moduleName, ctx)
{
}

void ASTNode::gen() const
{
    GenContext gen("demo-pjp");
    codegen(gen);
    gen.module.print(llvm::outs(), nullptr);
}

llvm::Value* TypeASTNode::codegen(GenContext& gen) const { return nullptr; }

llvm::Value* BinOpASTNode::codegen(GenContext& gen) const
{
    auto* l = m_lhs->codegen(gen);
    auto* r = m_rhs->codegen(gen);


    switch (m_op) {
    case TokenType::PLUS:
        return gen.builder.CreateAdd(l, r, "add");
    case TokenType::ASTERISK:
        return gen.builder.CreateMul(l, r, "mul");
    case TokenType::MINUS:
        return gen.builder.CreateSub(l, r, "sub");
    case TokenType::EQ:
        return gen.builder.CreateICmpEQ(l, r, "sub");
    default:
        throw std::logic_error("Unimplemented binop");
    }
}

llvm::Value* UnaryOpASTNode::codegen(GenContext& gen) const { return nullptr; }

llvm::Value* AssignASTNode::codegen(GenContext& gen) const
{
    auto* e = m_expr->codegen(gen);
    auto* store = m_var->getStore(gen);
    return gen.builder.CreateStore(e, store);
}

llvm::Value* LiteralASTNode::codegen(GenContext& gen) const
{
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), m_value);
}

llvm::Value* DeclRefASTNode::codegen(GenContext& gen) const
{
    if (auto it = gen.symbTable.find(m_var); it != gen.symbTable.end()) {
        return gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), it->second.store);
    } else {
        throw std::logic_error("var not declared");
    }
}

llvm::AllocaInst* DeclRefASTNode::getStore(GenContext& gen) const
{
    if (auto it = gen.symbTable.find(m_var); it != gen.symbTable.end()) {
        return it->second.store;
    } else {
        throw std::logic_error("var not declared");
    }
}

llvm::Value* FunCallASTNode::codegen(GenContext& gen) const
{

    std::vector<llvm::Value*> param;
    for (const auto& e : m_args)
        param.emplace_back(e->codegen(gen));

    auto fn = gen.module.getFunction(m_func);
    if (fn == nullptr) {
        throw std::logic_error("Func not declared");
    }

    return gen.builder.CreateCall(fn, param);
}

llvm::Value* IfASTNode::codegen(GenContext& gen) const
{

    auto *bbThen = llvm::BasicBlock::Create(gen.ctx, "then", gen.builder.GetInsertBlock()->getParent());
    auto *bbAfter = llvm::BasicBlock::Create(gen.ctx, "after", gen.builder.GetInsertBlock()->getParent());

    auto* cond = m_cond->codegen(gen);
    if (cond->getType()->isIntegerTy(32)) {
        gen.builder.CreateICmpNEQ(...);
    }
    gen.builder.CreateCondBr(cond, bbThen, bbAfter);

    gen.builder.SetInsertPoint(bbThen);
    m_body->codegen(gen);
    gen.builder.CreateBr(bbAfter);

    gen.builder.SetInsertPoint(bbAfter);

    return nullptr;
}

llvm::Value* VarDeclASTNode::codegen(GenContext& gen) const
{
    if (gen.symbTable.contains(m_var))
        throw std::logic_error("Duplicate value");

    llvm::AllocaInst* store = gen.builder.CreateAlloca(llvm::Type::getInt32Ty(gen.ctx), 0, m_var);
    gen.symbTable[m_var] = {store, m_type.get()};

    if (m_expr) {
        auto* e = m_expr->codegen(gen);
        gen.builder.CreateStore(e, store);
    }

    return nullptr;
}

llvm::Value* ExpressionStatementASTNode::codegen(GenContext& gen) const { return m_expr->codegen(gen); }

llvm::Value* ProgramASTNode::codegen(GenContext& gen) const
{
    auto mainFnType = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), {});
    auto mainFn = llvm::Function::Create(mainFnType, llvm::Function::ExternalLinkage, "main", gen.module);

    auto printIType = llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), {llvm::Type::getInt32Ty(gen.ctx)}, false);
    auto printFn = llvm::Function::Create(printIType, llvm::Function::ExternalLinkage, "printI", gen.module);

    auto entryBB = llvm::BasicBlock::Create(gen.ctx, "entry", mainFn);
    gen.builder.SetInsertPoint(entryBB);

    for (const auto& stmt : m_statements)
        stmt->codegen(gen);

    auto* retVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 43);
    gen.builder.CreateRet(retVal);

    return nullptr;
}
