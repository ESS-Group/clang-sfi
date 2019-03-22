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
        )).bind("variable"), createMatchHandler("variable"));

    Matcher.addMatcher(
            varDecl(
                    allOf(
                    //unless(anyOf(hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt()))),
                    hasAncestor(compoundStmt()),
                    unless(varDecl(hasInitializer(expr())))
                    )
            ).bind("notInitialized"), createMatchHandler("notInitialized")); // in this case get next assignment

}
// clang-format on

bool MVIVInjectorSAFE::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    if (current.isStmt) {
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        const VarDecl *vardecl = cast<VarDecl>(current.decl);
        const DeclStmt *declstmt = getParentOfType<DeclStmt>(current.decl, Context, 3);
        SourceRange range(vardecl->getLocation().getLocWithOffset(vardecl->getNameAsString().length()),
                          vardecl->getInit()->getLocEnd());
        R.RemoveText(range);
    }
    return true;
}
bool MVIVInjectorSAFE::checkStmt(const Decl &decl, std::string binding, ASTContext &Context) {
    if (binding.compare("notInitialized") == 0 && isa<VarDecl>(decl)) {
        std::vector<const BinaryOperator *> list =
            getChildForFindInitForVar(getParentCompoundStmt(&decl, Context), cast<const VarDecl>(&decl));
        for (const BinaryOperator *op : list) {
            assert(op != NULL);
            if (isValueAssignment(op) && C2(*op, Context)) {
                nodeCallback(binding, *op);
            }
        }

        return false;
    } else {
        if (C2(decl, Context)) {
            return true;
        } else {
            const DeclStmt *declsstmt = getParentOfType<DeclStmt>(&decl, Context, 2);
            const ForStmt *stmt = getParentOfType<ForStmt>(declsstmt, Context, 5);

            return stmt != NULL && (isParentOf(stmt->getInit(), decl, Context) || stmt->getInit(), declsstmt);
        }
    }
}
