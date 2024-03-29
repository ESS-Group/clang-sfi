#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mviv-safe"

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

bool MVIVInjectorSAFE::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.isStmt) {
        SourceRange range(current.stmt->getBeginLoc(), current.stmt->getEndLoc());
        LLVM_DEBUG(dbgs() << "MVIV-safe: Removed range for stmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    } else {
        const VarDecl *vardecl = cast<VarDecl>(current.decl);
        SourceLocation start = vardecl->getLocation().getLocWithOffset(vardecl->getNameAsString().length()),
                       end = vardecl->getInit()->getEndLoc();
        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "MVIV-safe: Removed range for varDecl"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    }
    return false;
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
