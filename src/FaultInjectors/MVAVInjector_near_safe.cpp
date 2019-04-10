#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mvav-safe"

std::string MVAVInjectorSAFE::toString() {
    return "MVAVSAFE";
};

MVAVInjectorSAFE::MVAVInjectorSAFE(bool alsoOverwritten) { // Missing variable assignment using a value
    Matcher.addMatcher(varDecl(hasAncestor(compoundStmt())).bind("varDecl"), createMatchHandler("varDecl"));
}

bool MVAVInjectorSAFE::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.binding.compare("varDecl") == 0) {
        SourceLocation start = current.stmt->getBeginLoc(), end = current.stmt->getEndLoc();
        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "MVAV-safe: Removed range for varDecl"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    } else {
        assert(false && "Unknown binding in MVAV-safe injector");
        std::cerr << "Unknown binding in MVAV-safe injector" << std::endl;
    }

    return false;
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
