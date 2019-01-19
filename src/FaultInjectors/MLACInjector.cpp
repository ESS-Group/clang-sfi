#include "_all.h"

std::string MLACInjector::toString() {
    return "MLAC";
};
// clang-format off
MLACInjector::MLACInjector() { // Missing AND clause in branch condition
    //Matcher.addMatcher(binaryOperator(anyOf(hasAncestor(expr(anyOf(hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt())))),hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt()))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    Matcher.addMatcher(
        stmt(
            switchStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("switch"), createStmtHandler("switch")
    );
    Matcher.addMatcher(
        stmt(
            doStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("do"), createStmtHandler("do")
    );
    Matcher.addMatcher(
        stmt(
            whileStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("while"), createStmtHandler("while")
    );
    Matcher.addMatcher(
        stmt(
            forStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("for"), createStmtHandler("for")
    );
    Matcher.addMatcher(
        stmt(
            ifStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("if"), createStmtHandler("if")
    );
}
// clang-format on

bool MLACInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    bool left = current.left;

    SourceLocation start, end;
    if (left) {
        start = cast<BinaryOperator>(current.stmt)->getLHS()->getLocStart();
        end = cast<BinaryOperator>(current.stmt)->getRHS()->getLocStart().getLocWithOffset(-1);
    } else {
        start = cast<BinaryOperator>(current.stmt)->getOperatorLoc();
        end = cast<BinaryOperator>(current.stmt)->getRHS()->getLocEnd();
    }

    SourceRange range(start, end);
    R.RemoveText(range);
    // return getEditedString(R, Context);
    return true;
}
bool MLACInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) { // no else
    std::vector<const BinaryOperator *> binaryOperators;
    if (binding.compare("if") == 0) {
        const Expr *condition = cast<IfStmt>(stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    } else if (binding.compare("do") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    } else if (binding.compare("switch") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    } else if (binding.compare("while") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    } else if (binding.compare("for") == 0) {
        const Stmt *condition = cast<IfStmt>(stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    } else {
        return false;
    }
    for (const BinaryOperator *op : binaryOperators) {
        if (op->getOpcode() == BinaryOperatorKind::BO_LAnd) {
            const Expr *left = op->getLHS()->IgnoreImplicit();
            const Expr *right = op->getRHS()->IgnoreImplicit();
            if (!isa<BinaryOperator>(left) || cast<BinaryOperator>(left)->getOpcode() != BinaryOperatorKind::BO_LAnd) {
                nodeCallback("MLOC", op, true);
            }
            if (!isa<BinaryOperator>(right) ||
                cast<BinaryOperator>(right)->getOpcode() != BinaryOperatorKind::BO_LAnd) {
                nodeCallback("MLOC", op, false);
            }
        }
    }
    return false;
}
