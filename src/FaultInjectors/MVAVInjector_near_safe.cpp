#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mvav-safe"

std::string MVAVInjectorSAFE::toString() {
    return "MVAVSAFE";
};

MVAVInjectorSAFE::MVAVInjectorSAFE(bool alsoOverwritten) { // Missing variable assignment using a value
    Matcher.addMatcher(varDecl(hasAncestor(compoundStmt())).bind("varDecl"), createMatchHandler("varDecl"));
}

bool MVAVInjectorSAFE::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);

    return true;
}
bool MVAVInjectorSAFE::checkStmt(const Decl &decl, std::string binding, ASTContext &Context) {
    if (binding.compare("varDecl") == 0 && isa<VarDecl>(decl)) {
        std::vector<const BinaryOperator *> list =
            getChildForFindVarAssignment(getParentCompoundStmt(&decl, Context), cast<const VarDecl>(&decl), true);
        for (const BinaryOperator *op : list) {
            assert(op != NULL);
            auto lhs = op->getLHS();
            if (isValueAssignment(op) && isInitializedBefore(cast<const DeclRefExpr>(*lhs), Context)) {
                if (const ForStmt *forstmt = getParentOfType<ForStmt>(&decl, Context, 3)) {
                    if (isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl, Context)) {
                    } else {
                        nodeCallback(binding, *op);
                    }
                } else {
                    nodeCallback(binding, *op);
                }
            }
        }

        return false;
    }
    return false;
}
