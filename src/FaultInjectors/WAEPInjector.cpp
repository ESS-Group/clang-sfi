#include "_all.h"

std::string WAEPInjector::toString() {
    return "WAEP";
};

// clang-format off
WAEPInjector::WAEPInjector() { // Wrong arithmetic expressino in parameter of function call
    Matcher.addMatcher(
        callExpr(
            allOf(
                unless(anyOf(
                    cudaKernelCallExpr(),
                    cxxOperatorCallExpr(),
                    userDefinedLiteral()
                )),
                hasDescendant(expr(anyOf(
                    binaryOperator(),
                    hasDescendant(binaryOperator())
                )))
            )
        ).bind("functionCall"),
        createStmtHandler("functionCall")
    );
    //Matcher.addMatcher(callExpr(unless(anyOf(hasParent(varDecl(isDefinition())), hasParent(returnStmt()), cxxOperatorCallExpr(), hasParent(callExpr()), hasParent(expr())))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    //Matcher.addMatcher(callExpr().bind("FunctionCall"), createStmtHandler("FunctionCall"));
}
// clang-format on

bool WAEPInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    SourceLocation start, end;

    start = ((const BinaryOperator *)current.stmt)->getOperatorLoc(); //.getLocWithOffset(-1);//.getLocWithOffset(1);
    end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();

    SourceRange range(start, end);
    R.RemoveText(range);
    // return getEditedString(R, Context);
    return true;
}
bool WAEPInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    if (hasChildOfType<BinaryOperator>(stmt)) {
        std::vector<const BinaryOperator *> arguments = getChildrenFlat<BinaryOperator>(stmt);
        for (const BinaryOperator *op : arguments) {
            // int code = op->getOpcode();
            if (isArithmetic(op)) {
                const BinaryOperator *rightest = getBinaryOperatorWithRightedtRHS(op);

                // rightest->getRHS()->dumpColor();

                // const Expr* temp = rightest->IgnoreParenCasts();
                // if(rightest->getType().getDesugaredType(Context).getNonReferenceType()
                // ==
                // rightest->getLHS()->getType().getDesugaredType(Context).getNonReferenceType())
                nodeCallback(binding, rightest);
                /*else{
                    rightest->getLocStart().dump(Context.getSourceManager());
                    cerr<<endl;
                    rightest->dumpColor();
                    rightest->getType().dump();
                    rightest->getLHS()->getType().dump();
                */ /*}*/
            }
        }
    }
    return false;
}
