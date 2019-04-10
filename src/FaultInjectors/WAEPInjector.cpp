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

bool WAEPInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.binding.compare("functionCall") == 0) {
        SourceLocation start, end;

        start = cast<const BinaryOperator>(current.stmt)->getOperatorLoc();
        end = cast<const BinaryOperator>(current.stmt)->getRHS()->getEndLoc();

        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "WAEP: Removed range for functionCall"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    } else {
        assert(false && "Unknown binding in WAEP injector");
        std::cerr << "Unknown binding in WAEP injector" << std::endl;
    }
    return false;
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
