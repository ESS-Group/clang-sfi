#include "_all.h"

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
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        VarDecl temp(*((const VarDecl *)current.decl));
        temp.setInit(NULL);
        const VarDecl *tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());

        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
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
