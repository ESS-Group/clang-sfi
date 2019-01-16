#include "_all.h"

std::string MVAEInjectorSAFE::toString() {
    return "MVAESAFE";
};

// clang-format off
MVAEInjectorSAFE::MVAEInjectorSAFE() { // Missing variable assignment using an expression
    /*
        Matcher.addMatcher(
        varDecl(
                hasInitializer(
                    allOf(
                        expr(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator())),
                        hasAncestor(compoundStmt())
                    )
                )
            ).bind("variable"), createStmtHandler("variable"));
    */

    Matcher.addMatcher(
            varDecl(
                //commented to include global assignments
                    hasAncestor(compoundStmt())
            ).bind("varDecl"), createStmtHandler("varDecl"));
}
// clang-format on

std::string MVAEInjectorSAFE::inject(StmtBinding current, ASTContext &Context) {
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
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

    return getEditedString(R, Context);
}

bool MVAEInjectorSAFE::checkStmt(const Decl *decl, std::string binding, ASTContext &Context) {
    if (binding.compare("varDecl") == 0 && isa<VarDecl>(decl)) {
        std::vector<const BinaryOperator *> list =
            getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl *)decl, true);
        for (const BinaryOperator *op : list) {
            if (isExprAssignment(op) && (isInitializedBefore((const DeclRefExpr *)((op)->getLHS()), Context))) {
                if (const ForStmt *forstmt = getParentOfType<ForStmt>(decl, Context, 3)) {
                    if (isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl, Context)) {
                    } else if (C2(op, Context)) {
                        nodeCallback(binding, op);
                    }
                } else if (C2(op, Context)) {
                    nodeCallback(binding, op);
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
