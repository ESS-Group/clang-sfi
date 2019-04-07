#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mviv"

std::string MVIVInjector::toString() {
    return "MVIV";
};

// clang-format off
MVIVInjector::MVIVInjector() { // Missing variable initialization using a value
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
        
        ).bind("variable"), createMatchHandler("variable"));
}
// clang-format on

bool MVIVInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.isStmt) {
        SourceLocation start = current.stmt->getBeginLoc(), end = current.stmt->getEndLoc();
        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "MVIV: Removed range for stmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    } else {
        const VarDecl *vardecl = cast<VarDecl>(current.decl);
        const DeclStmt *declstmt = getParentOfType<DeclStmt>(current.decl, Context, 3);
        SourceLocation start = vardecl->getLocation().getLocWithOffset(vardecl->getNameAsString().length()),
                       end = vardecl->getInit()->getEndLoc();
        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "MVIV: Removed range for varDecl"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    }
    return false;
}
bool MVIVInjector::checkStmt(const Decl &decl, std::string binding, ASTContext &Context) {
    const DeclStmt *declstmt = getParentOfType<DeclStmt>(&decl, Context, 3);
    if (const ForStmt *forstmt = getParentOfType<ForStmt>(declstmt, Context, 3)) {
        assert(declstmt != NULL);
        return isParentOf(forstmt->getBody(), *declstmt) && !cast<VarDecl>(decl).isStaticLocal() && C2(decl, Context);
    } else if (declstmt != NULL) {
        const VarDecl &vardecl = cast<VarDecl>(decl);
        return !vardecl.getType().isConstant(Context) && !vardecl.isStaticLocal() && C2(decl, Context);
    } else
        return false;
    // C2 implementation implicitly excludes decl being part of a for
    // construct.
}
