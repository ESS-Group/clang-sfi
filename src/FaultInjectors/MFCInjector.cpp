#include "_all.h"

std::string MFCInjector::toString() {
    return "MFC";
};

// clang-format off
MFCInjector::MFCInjector() { // Missing function call
    Matcher.addMatcher(
        callExpr(
            ignoringImplicit(
                unless(
                    anyOf( //not used in variable declaration, return statement, function call, operator call
                        hasAncestor(varDecl(isDefinition())),
                        hasParent(returnStmt()),
                        hasParent(callExpr()),
                        hasParent(binaryOperator()),
                        hasParent(unaryOperator()),
                        cxxOperatorCallExpr()
                    )
                )
            )
        ).bind("FunctionCall"), createStmtHandler("FunctionCall"));
}
// clang-format on

std::string MFCInjector::inject(StmtBinding current, ASTContext &Context) {
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MFCInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    return C2(stmt, Context);
}
