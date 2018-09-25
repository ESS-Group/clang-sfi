#include "_all.h"

std::string MVIVInjector::toString() {
    return "MVIV";
};

// clang-format off
MVIVInjector::MVIVInjector() {
    Matcher.addMatcher(
        varDecl(//variable declaration
            allOf(
                hasInitializer(//variable declaration that has an initializer
                unless(
                    anyOf(//assure that initializer is an value
                        callExpr(),
                        hasDescendant(callExpr()),
                        cxxNewExpr(),
                        hasDescendant(cxxNewExpr()),
                        binaryOperator(),
                        hasDescendant(binaryOperator()),
                        unaryOperator(),
                        hasDescendant(unaryOperator()),
                        cxxConstructExpr(),
                        hasDescendant(cxxConstructExpr()),
                        declRefExpr(),
                        hasDescendant(declRefExpr()),
                        memberExpr(), 
                        hasDescendant(memberExpr()),
                        hasDescendant(conditionalOperator()),
                        hasDescendant(binaryConditionalOperator()),
                        conditionalOperator(),
                        binaryConditionalOperator()
                        /*castExpr(anyOf(
                                hasDescendant(declRefExpr()),
                                hasDescendant(memberExpr())))*/
                ))
            )

            ,hasDeclContext(functionDecl())//declaration in function context (local)
        )
        
        ).bind("variable"), createStmtHandler("variable"));
}
// clang-format on

std::string MVIVInjector::inject(StmtBinding current, ASTContext &Context) {
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
bool MVIVInjector::checkStmt(const Decl *decl, std::string binding, ASTContext &Context) {
    const DeclStmt *declstmt = getParentOfType<DeclStmt>(decl, Context, 3);
    if (const ForStmt *forstmt = getParentOfType<ForStmt>(declstmt, Context, 3)) {
        return isParentOf(forstmt->getBody(), declstmt) && !((const VarDecl *)decl)->isStaticLocal() &&
               C2(decl, Context);
    } else {
        return !((const VarDecl *)decl)->isStaticLocal() && C2(decl, Context);
    }
    // C2 implementation implicitly excludes decl being part of an for
    // construct.
}
