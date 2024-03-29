#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mfc"

std::string MFCInjector::toString() {
    return "MFC";
};

// clang-format off
MFCInjector::MFCInjector() { // Missing function call
    Matcher.addMatcher(
        callExpr(
            ignoringImplicit(
                unless(
                    anyOf( //not used in variable declaration, return statement, function call, operator call
                        hasAncestor(varDecl(isDefinition())),
                        hasParent(returnStmt()),
                        hasParent(callExpr()),
                        hasParent(binaryOperator()),
                        hasParent(unaryOperator()),
                        cxxOperatorCallExpr()
                    )
                )
            )
        ).bind("FunctionCall"), createMatchHandler("FunctionCall"));
	Matcher.addMatcher(
		binaryOperator(
			allOf(
				hasOperatorName(","),
				hasParent(
					parenExpr(
						unless(
							hasParent(
								binaryOperator(
									hasOperatorName(",")
								)
							)
						)
					)
				),
				hasDescendant(
					callExpr()
				)
			)
		).bind("CommaOperator"), createMatchHandler("CommaOperator"));
}
// clang-format on

bool MFCInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.binding.compare("FunctionCall") == 0) {
        SourceLocation start = current.stmt->getBeginLoc(), end = current.stmt->getEndLoc();
        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "MFC: Removed range for FunctionCall"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    } else if (current.binding.compare("CommaOperator") == 0) {
        SourceLocation start, end;
        if (current.left) {
            start = cast<const BinaryOperator>(current.stmt)->getLHS()->getBeginLoc();
            end = cast<const BinaryOperator>(current.stmt)->getRHS()->getBeginLoc().getLocWithOffset(-1);
        } else {
            start = cast<const BinaryOperator>(current.stmt)->getOperatorLoc();
            end = cast<const BinaryOperator>(current.stmt)->getRHS()->getEndLoc();
        }
        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "MFC: Removed range for CommaOperator"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    } else {
        assert(false && "Unkown binding in MFC injector");
        std::cerr << "Unknown binding in MFC injector" << std::endl;
    }

    return false;
}
std::vector<const Stmt *> getFunctionCallExprListInCommaOp(const BinaryOperator *bo, bool checkRight = false,
                                                           bool neverCheckRight = false) {
    if (neverCheckRight) {
        checkRight = false;
    }
    const Expr *lhs = cast<const BinaryOperator>(bo)
                          ->getLHS()
                          ->IgnoreParenCasts()
                          ->IgnoreImplicit()
                          ->IgnoreParenCasts()
                          ->IgnoreImplicit();
    const Expr *rhs = cast<const BinaryOperator>(bo)
                          ->getRHS()
                          ->IgnoreParenCasts()
                          ->IgnoreImplicit()
                          ->IgnoreParenCasts()
                          ->IgnoreImplicit();
    std::vector<const Stmt *> ret;
    if (isa<CallExpr>(lhs) && !isa<CXXOperatorCallExpr>(lhs)) {
        ret.push_back(lhs);
    } else if (isa<BinaryOperator>(lhs) && cast<const BinaryOperator>(lhs)->getOpcode() == BO_Comma) {
        std::vector<const Stmt *> temp = getFunctionCallExprListInCommaOp(cast<const BinaryOperator>(lhs));
        if (temp.size() != 0) {
            for (const Stmt *tmp : temp) {
                ret.push_back(tmp);
            }
        }
    }
    if (isa<BinaryOperator>(rhs) && cast<const BinaryOperator>(rhs)->getOpcode() == BO_Comma) {
        std::vector<const Stmt *> temp = getFunctionCallExprListInCommaOp(cast<const BinaryOperator>(rhs), true);
        if (temp.size() != 0) {
            for (const Stmt *tmp : temp) {
                ret.push_back(tmp);
            }
        }
    } else if (!checkRight && isa<CallExpr>(rhs)) { // otherwise return of callExpr could be used
        ret.push_back(rhs);
    }

    return ret;
}
bool MFCInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    if (binding.compare("FunctionCall") == 0) {
        return C2(stmt, Context);
    } else if (binding.compare("CommaOperator") == 0) {
        const Stmt *parent = getParentIgnoringParenCasts(stmt, Context);

        std::vector<const Stmt *> stmts = getFunctionCallExprListInCommaOp(cast<const BinaryOperator>(&stmt), true,
                                                                           parent != NULL && !isa<CallExpr>(parent));
        if (stmts.size() != 0) {
            for (const Stmt *stmt : stmts) {
                const BinaryOperator *op = getParentOfType<BinaryOperator>(stmt, Context);
                if (op) {
                    bool left = isParentOfOrSelf(op->getLHS(), *stmt, Context);
                    if (left || isParentOfOrSelf(op->getRHS(), *stmt, Context)) {
                        assert(op != NULL);
                        nodeCallback("CommaOperator", *op, left);
                    }
                }
            }
        }
    }
    return false;
}
