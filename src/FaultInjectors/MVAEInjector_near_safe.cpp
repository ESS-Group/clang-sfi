#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mvae-safe"

std::string MVAEInjectorSAFE::toString() {
    return "MVAESAFE";
};

// clang-format off
MVAEInjectorSAFE::MVAEInjectorSAFE() { // Missing variable assignment using an expression
    Matcher.addMatcher(
            varDecl(
                //commented to include global assignments
                    hasAncestor(compoundStmt())
            ).bind("varDecl"), createMatchHandler("varDecl"));
}
// clang-format on

bool MVAEInjectorSAFE::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    if (current.isStmt) {
        SourceLocation start = current.stmt->getLocStart(),
            end = current.stmt->getLocEnd();
        SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
        R.RemoveText(range);
    } else {
        VarDecl temp(*((const VarDecl *)current.decl));
        temp.setInit(NULL);
        const VarDecl *tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());

        SourceLocation start = current.decl->getLocStart(),
            end = current.decl->getLocEnd();
        SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
        R.ReplaceText(range, withoutInit);
    }

    return true;
}

bool MVAEInjectorSAFE::checkStmt(const Decl &decl, std::string binding, ASTContext &Context) {
    if (binding.compare("varDecl") == 0 && isa<VarDecl>(decl)) {
        std::vector<const BinaryOperator *> list =
            getChildForFindVarAssignment(getParentCompoundStmt(&decl, Context), cast<const VarDecl>(&decl), true);
        for (const BinaryOperator *op : list) {
            assert(op != NULL);
            auto lhs = op->getLHS();
            assert(lhs != NULL);
            if (isExprAssignment(op) && isInitializedBefore(cast<const DeclRefExpr>(*lhs), Context)) {
                if (const ForStmt *forstmt = getParentOfType<ForStmt>(&decl, Context, 3)) {
                    if (isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl, Context)) {
                    } else if (C2(*op, Context)) {
                        nodeCallback(binding, *op);
                    }
                } else if (C2(*op, Context)) {
                    nodeCallback(binding, *op);
                }
            }
        }

        return false;
    } /*else{
         if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
             if(isParentOf(forstmt->getCond(), decl, Context) ||
     isParentOf(forstmt->getInc(), decl,Context)){
             } else {
                return C2(decl, Context);
             }
         } else {
             return C2(decl, Context);
         }
     }*/
    return false;
}
