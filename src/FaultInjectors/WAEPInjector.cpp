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
}
// clang-format on

bool WAEPInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    SourceLocation start, end;

    start = cast<const BinaryOperator>(current.stmt)->getOperatorLoc();
    end = cast<const BinaryOperator>(current.stmt)->getRHS()->getLocEnd();

    SourceRange range(start, end);
    R.RemoveText(range);
    return true;
}
bool WAEPInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    if (hasChildOfType<BinaryOperator>(stmt)) {
        std::vector<const BinaryOperator *> arguments = getChildrenFlat<BinaryOperator>(stmt);
        for (const BinaryOperator *op : arguments) {
            if (isArithmetic(op)) {
                const BinaryOperator *rightest = getBinaryOperatorWithRightedtRHS(op);
                nodeCallback(binding, rightest);
            }
        }
    }
    return false;
}
