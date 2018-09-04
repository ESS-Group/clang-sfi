#include "_all.h"

MVAVInjectorSAFE::MVAVInjectorSAFE(bool alsoOverwritten) {
    Matcher.addMatcher(varDecl(hasAncestor(compoundStmt())).bind("varDecl"),
                       createStmtHandler("varDecl"));
}

std::string MVAVInjectorSAFE::toString() { return "MVAVSAFE"; };

std::string MVAVInjectorSAFE::inject(StmtBinding current, ASTContext &Context) {
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());

    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);

    return getEditedString(R, Context);
}
bool MVAVInjectorSAFE::checkStmt(const Decl *decl, std::string binding,
                             ASTContext &Context) {
    if (binding.compare("varDecl") == 0 && isa<VarDecl>(decl)) {
        std::vector<const BinaryOperator *> list = getChildForFindVarAssignment(
            getParentCompoundStmt(decl, Context), (const VarDecl *)decl, true);
        for (const BinaryOperator *op : list) {
            if (isValueAssignment(op) &&
                isInitializedBefore((const DeclRefExpr *)((op)->getLHS()),
                                    Context)) {
                if (const ForStmt *forstmt =
                        getParentOfType<ForStmt>(decl, Context, 3)) {
                    if (isParentOf(forstmt->getCond(), decl, Context) ||
                        isParentOf(forstmt->getInc(), decl, Context)) {
                    } else {
                        nodeCallback(binding, op);
                    }
                } else {
                    nodeCallback(binding, op);
                }
            }
        }

        return false;
    }
    return false;
}
