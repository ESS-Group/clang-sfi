#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mvae"

std::string MVAEInjector::toString() {
    return "MVAE";
};
std::string OMVAEInjector::toString() {
    return "OMVAE";
};

// clang-format off
MVAEInjector::MVAEInjector(bool alsoOverwritten) { // Missing variable assignment using an expression
    this->alsoOverwritten = alsoOverwritten;
    Matcher.addMatcher(
            binaryOperator(
                    allOf(
                        hasOperatorName("="), //assignmentOperator
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
                                    
                                    //assignment to one time dereferenced local pointer
                                    ignoringParenCasts(ignoringImplicit(
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))
                                        ))
                                    )),

                                    //assignment to one time dereferenced pointer, which is member of a local object
                                    ignoringParenCasts(ignoringImplicit(
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(memberExpr(hasObjectExpression(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))))
                                        ))
                                    ))
                                )
                            )
                        ),
                        hasRHS(
                            anyOf(//assure right side is an expression
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
                        )
                    )
            ).bind("assignment"), createMatchHandler("assignment"));

        if(alsoOverwritten) {
            Matcher.addMatcher( //overwritten assignmentoperator call, rest like above
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
                            anyOf(
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
                        )
                    )
            ).bind("overwritten"), createMatchHandler("overwritten"));
        }
}
// clang-format on

bool MVAEInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    SourceLocation start = current.stmt->getBeginLoc(), end = current.stmt->getEndLoc();
    SourceRange range(start, end);
    LLVM_DEBUG(dbgs() << "MVAE: Removed range"
                      << "\n"
                      << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                      << range.getEnd().printToString(R.getSourceMgr()) << "\n");

    return R.RemoveText(range);
}

bool MVAEInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    if (binding.compare("overwritten") == 0) {
        const CXXOperatorCallExpr &opCall = cast<const CXXOperatorCallExpr>(stmt);
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

OMVAEInjector::OMVAEInjector()
    : MVAEInjector(true) {

      };
