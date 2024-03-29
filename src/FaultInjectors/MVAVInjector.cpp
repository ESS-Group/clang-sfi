#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mvav"

std::string MVAVInjector::toString() {
    return "MVAV";
};
std::string OMVAVInjector::toString() {
    return "OMVAV";
};

// clang-format off
MVAVInjector::MVAVInjector(bool alsoOverwritten) { // Missing variable assignment using a value
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
                                    
                                    ignoringParenCasts(ignoringImplicit(//assignment to one time dereferenced local pointer
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))
                                        ))
                                    )),
                                    
                                    ignoringParenCasts(ignoringImplicit(//assignment to one time dereferenced pointer, which is member of a local object
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
            ).bind("assignment"), createMatchHandler("assignment"));

            if(alsoOverwritten) {
            Matcher.addMatcher( // overwritten assignmentoperator call, rest like above
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

bool MVAVInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    SourceLocation start = current.stmt->getBeginLoc(), end = current.stmt->getEndLoc();
    SourceRange range(start, end);
    LLVM_DEBUG(dbgs() << "MVAV: Removed range"
                      << "\n"
                      << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                      << range.getEnd().printToString(R.getSourceMgr()) << "\n");

    return R.RemoveText(range);
}

bool MVAVInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    if (binding.compare("overwritten") == 0) {
        const CXXOperatorCallExpr &opCall = cast<CXXOperatorCallExpr>(stmt);
        if (!opCall.isInfixBinaryOp()) {
            return false;
        }
    }
    if (const ForStmt *forstmt = getParentOfType<ForStmt>(&stmt, Context, 3)) {
        return !isParentOfOrSelf(forstmt->getCond(), stmt, Context) && !isParentOfOrSelf(forstmt->getInc(), stmt, Context) && !isParentOfOrSelf(forstmt->getInit(), stmt, Context) && C2(stmt, Context);
    } else {
        return C2(stmt, Context);
    }
}

OMVAVInjector::OMVAVInjector()
    : MVAVInjector(true) {

      };
