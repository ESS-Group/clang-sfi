#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mloc"

std::string MLOCInjector::toString() {
    return "MLOC";
};

// clang-format off
MLOCInjector::MLOCInjector() { // Missing OR clause in branch condition
    Matcher.addMatcher(
        stmt(
            switchStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("switch"), createMatchHandler("switch")
    );
    Matcher.addMatcher(
        stmt(
            doStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("do"), createMatchHandler("do")
    );
    Matcher.addMatcher(
        stmt(
            whileStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("while"), createMatchHandler("while")
    );
    Matcher.addMatcher(
        stmt(
            forStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("for"), createMatchHandler("for")
    );
    Matcher.addMatcher(
        stmt(
            ifStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("if"), createMatchHandler("if")
    );
}
// clang-format on

bool MLOCInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    bool left = current.left;

    SourceLocation start, end;
    if (left) {
        start = cast<BinaryOperator>(current.stmt)->getLHS()->getBeginLoc();
        end = cast<BinaryOperator>(current.stmt)->getRHS()->getBeginLoc().getLocWithOffset(-1);
    } else {
        start = cast<BinaryOperator>(current.stmt)->getOperatorLoc();
        end = cast<BinaryOperator>(current.stmt)->getRHS()->getEndLoc();
    }

    SourceRange range(start, end);
    R.RemoveText(range);
    LLVM_DEBUG(dbgs() << "MLOC: Removed range"
                      << "\n"
                      << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                      << range.getEnd().printToString(R.getSourceMgr()) << "\n");
    return true;
}

bool MLOCInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) { // no else
    std::vector<const BinaryOperator *> binaryOperators;
    if (binding.compare("if") == 0) {
        const Expr *condition = cast<IfStmt>(stmt).getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(*condition);
    } else if (binding.compare("do") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt).getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(*condition);
    } else if (binding.compare("switch") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt).getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(*condition);
    } else if (binding.compare("while") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt).getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(*condition);
    } else if (binding.compare("for") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt).getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(*condition);
    } else {
        assert(false && "Unknown binding in MLOC injector");
        std::cerr << "Unknown binding in MLOC injector" << std::endl;
    }
    for (const BinaryOperator *op : binaryOperators) {
        if (op->getOpcode() == BinaryOperatorKind::BO_LOr) {
            assert(op != NULL);
            const Expr *left = op->getLHS()->IgnoreImplicit();
            const Expr *right = op->getRHS()->IgnoreImplicit();
            if (!isa<BinaryOperator>(left) || cast<BinaryOperator>(left)->getOpcode() != BinaryOperatorKind::BO_LOr) {
                nodeCallback("MLOC", *op, true);
            }
            if (!isa<BinaryOperator>(right) || cast<BinaryOperator>(right)->getOpcode() != BinaryOperatorKind::BO_LOr) {
                nodeCallback("MLOC", *op, false);
            }
        }
    }
    return false;
}
