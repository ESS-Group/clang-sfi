#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-wvav"

std::string OWVAVInjector::toString() {
    return "OWVAV";
};
std::string WVAVInjector::toString() {
    return "WVAV";
};

// clang-format off
WVAVInjector::WVAVInjector(bool alsoOverwritten) { // Wrong value assigned to variable
    this->alsoOverwritten = alsoOverwritten;
    Matcher.addMatcher(
            binaryOperator(
                    allOf(
                        hasOperatorName("="),//assignment operator
                        hasLHS(//left side of assignment
                            allOf(
                                unless(hasDescendant(callExpr())),//no functioncall on left side
                                anyOf(
                                    declRefExpr(to(varDecl(hasDeclContext(functionDecl())))),//assignment to local variable
                                    memberExpr(hasObjectExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))),//assignment to member of local object
                                    arraySubscriptExpr(hasBase(
                                        implicitCastExpr(hasSourceExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl()))))))//assignment to array element of local array
                                    )),
                                    arraySubscriptExpr(hasBase(
                                        ignoringParenCasts(ignoringImplicit(
                                            memberExpr(hasObjectExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl()))))))//assignment to array element of member array of local object
                                        ))    
                                    )),
                                    
                                    ignoringParenCasts(ignoringImplicit(//assignment to one time dereferred local pointer
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))
                                        ))
                                    )),
                                    
                                    ignoringParenCasts(ignoringImplicit(//assignment to one time dereferred pointer, which is member of a local object
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(memberExpr(hasObjectExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))))
                                        ))
                                    ))
                                )
                            )
                        ),
                        hasRHS(//assure right side not to be an expression
                            unless(anyOf(
                                ignoringParenCasts(ignoringImplicit(callExpr())),
                                ignoringParenCasts(ignoringImplicit(cxxNewExpr())),
                                ignoringParenCasts(ignoringImplicit(binaryOperator())),
                                ignoringParenCasts(ignoringImplicit(unaryOperator())),
                                ignoringParenCasts(ignoringImplicit(cxxConstructExpr())),
                                ignoringParenCasts(ignoringImplicit(declRefExpr())),
                                ignoringParenCasts(ignoringImplicit(memberExpr())),
                                ignoringParenCasts(ignoringImplicit(conditionalOperator())),
                                ignoringParenCasts(ignoringImplicit(binaryConditionalOperator()))
                            )
                        ))
                    )
                    // hasAncestor(compoundStmt())
            ).bind("assignment"), createMatchHandler("assignment"));

        if(alsoOverwritten) { // overwritten assignmentoperator call, rest like above
            Matcher.addMatcher(
                cxxOperatorCallExpr(allOf(
                    hasOverloadedOperatorName("="),
                    argumentCountIs(2),
                    hasArgument(0,
                            allOf(
                                unless(hasDescendant(callExpr())),
                                anyOf(
                                    declRefExpr(to(varDecl(hasDeclContext(functionDecl())))),
                                    memberExpr(hasObjectExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))),
                                    arraySubscriptExpr(hasBase(
                                        implicitCastExpr(hasSourceExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl()))))))
                                    )),
                                    arraySubscriptExpr(hasBase(
                                        ignoringParenCasts(ignoringImplicit(
                                            memberExpr(hasObjectExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl()))))))
                                        ))    
                                    )),
                                    
                                    ignoringParenCasts(ignoringImplicit(
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))
                                        ))
                                    )),

                                    ignoringParenCasts(ignoringImplicit(
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(memberExpr(hasObjectExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))))
                                        ))
                                    ))
                                )
                            )
                        ),
                        hasArgument(1,
                            unless(anyOf(
                                ignoringParenCasts(ignoringImplicit(callExpr())),
                                ignoringParenCasts(ignoringImplicit(cxxNewExpr())),
                                ignoringParenCasts(ignoringImplicit(binaryOperator())),
                                ignoringParenCasts(ignoringImplicit(unaryOperator())),
                                ignoringParenCasts(ignoringImplicit(cxxConstructExpr())),
                                ignoringParenCasts(ignoringImplicit(declRefExpr())),
                                ignoringParenCasts(ignoringImplicit(memberExpr())),
                                ignoringParenCasts(ignoringImplicit(conditionalOperator())),
                                ignoringParenCasts(ignoringImplicit(binaryConditionalOperator()))
                            )
                        ))
                    )
            ).bind("overwritten"), createMatchHandler("overwritten"));
        }
}
// clang-format on

bool WVAVInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    if (binding.compare("overwritten") == 0) {
        const CXXOperatorCallExpr &opCall = cast<CXXOperatorCallExpr>(stmt);
        if (!opCall.isInfixBinaryOp()) {
            return false;
        }
        if (const ForStmt *forstmt = getParentOfType<ForStmt>(&stmt, Context, 3)) {
            assert(forstmt->getCond() != NULL);
            assert(forstmt->getInc() != NULL);
            if (isParentOf(forstmt->getCond(), stmt) || isParentOf(forstmt->getInc(), stmt)) {
                return false;
            }
        }
        return true;
    }
    if (const ForStmt *forstmt = getParentOfType<ForStmt>(&stmt, Context, 3)) {
        assert(forstmt->getCond() != NULL);
        assert(forstmt->getInc() != NULL);
        return !isParentOf(forstmt->getCond(), stmt) && !isParentOf(forstmt->getInc(), stmt);
    } else {
        return true;
    }
}

bool WVAVInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    const Expr *val = NULL;
    clang::QualType type;
    if (current.binding.compare("overwritten") == 0) {
        auto firstArg = cast<CXXOperatorCallExpr>(current.stmt)->getArg(1);
        val = cast<Expr>(firstArg);
        type = val->getType();
    } else {
        val = cast<BinaryOperator>(current.stmt)->getRHS();
        type = cast<BinaryOperator>(current.stmt)->getLHS()->getType();
    }

    SourceLocation start = val->getBeginLoc(), end = val->getEndLoc();
    SourceRange range(start, end);
    if (isa<CXXBoolLiteralExpr>(val)) {
        bool value = cast<CXXBoolLiteralExpr>(val)->getValue();
        if (value) {
            LLVM_DEBUG(dbgs() << "WVAV: Replaced range for varDecl"
                              << "\n"
                              << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                              << range.getEnd().printToString(R.getSourceMgr()) << " with "
                              << "false"
                              << "\n");
            return R.ReplaceText(range, "false");
        } else {
            LLVM_DEBUG(dbgs() << "WVAV: Replaced range for varDecl"
                              << "\n"
                              << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                              << range.getEnd().printToString(R.getSourceMgr()) << " with "
                              << "true"
                              << "\n");
            return R.ReplaceText(range, "true");
        }
    } else {
        std::string text = R.getRewrittenText(range);
        LLVM_DEBUG(dbgs() << "WVAV: Replaced range for varDecl"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << " with "
                          << "(" + type.getAsString() + ")" + text + "^0xFF"
                          << "\n");
        return R.ReplaceText(range, "(" + type.getAsString() + ")" + text + "^0xFF");
    }

    return false;
}

OWVAVInjector::OWVAVInjector()
    : WVAVInjector(true) {

      };
