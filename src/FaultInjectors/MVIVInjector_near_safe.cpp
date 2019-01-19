#include "_all.h"

std::string MVIVInjectorSAFE::toString() {
    return "MVIVSAFE";
};

// clang-format off
MVIVInjectorSAFE::MVIVInjectorSAFE() { // Missing variable initialization using a value
    Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(anyOf(
                callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator()
                //commented to also include initialisations inside of loops
                //,hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
            ))
            ,hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));

    Matcher.addMatcher(
            varDecl(
                    allOf(
                    //unless(anyOf(hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt()))),
                    hasAncestor(compoundStmt()),
                    unless(varDecl(hasInitializer(expr())))
                    )
            ).bind("notInitialized"), createStmtHandler("notInitialized")); // in this case get next assignment

}
// clang-format on

bool MVIVInjectorSAFE::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
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
    // return getEditedString(R, Context);
    return true;
}
bool MVIVInjectorSAFE::checkStmt(const Decl *decl, std::string binding, ASTContext &Context) {
    if (binding.compare("notInitialized") == 0 && isa<VarDecl>(decl)) {
        std::vector<const BinaryOperator *> list =
            getChildForFindInitForVar(getParentCompoundStmt(decl, Context), (const VarDecl *)decl);
        for (const BinaryOperator *op : list) {
            if (isValueAssignment(op) && C2(op, Context)) {
                nodeCallback(binding, op);
            }
        }

        return false;
    } else {
        if (C2(decl, Context)) {
            return true;
        } else {
            const DeclStmt *declsstmt = getParentOfType<DeclStmt>(decl, Context, 2);
            const ForStmt *stmt = getParentOfType<ForStmt>(declsstmt, Context, 5);

            return stmt != NULL && (isParentOf(stmt->getInit(), decl, Context) || stmt->getInit(), declsstmt);
        }
    }
}
