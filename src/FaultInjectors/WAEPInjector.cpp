#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-waep"

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
        ).bind("functionCall"), createMatchHandler("functionCall")
    );
}
// clang-format on

bool WAEPInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    if (current.binding.compare("functionCall")) {
        SourceLocation start, end;

        start = cast<const BinaryOperator>(current.stmt)->getOperatorLoc();
        end = cast<const BinaryOperator>(current.stmt)->getRHS()->getLocEnd();

        SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
        R.RemoveText(range);
        LLVM_DEBUG(dbgs() << "WAEP: Removed range for functionCall"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
    } else {
        assert(false && "Unknown binding in WAEP injector");
        std::cerr << "Unknown binding in WAEP injector" << std::endl;
    }
    return true;
}
bool WAEPInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    if (hasChildOfType<BinaryOperator>(&stmt)) {
        std::vector<const BinaryOperator *> arguments = getChildrenFlat<BinaryOperator>(&stmt);
        for (const BinaryOperator *op : arguments) {
            assert(op != NULL);
            if (isArithmetic(*op)) {
                const BinaryOperator rightest = getBinaryOperatorWithRightedtRHS(*op);
                nodeCallback(binding, rightest);
            }
        }
    }
    return false;
}
