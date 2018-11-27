#include "_all.h"

std::string MVAEInjector::toString() {
    return "MVAE";
};
std::string OMVAEInjector::toString() {
    return "OMVAE";
};

// clang-format off
MVAEInjector::MVAEInjector(bool alsoOverwritten) { // Missing variable assignment using an expression
    this->alsoOverwritten = alsoOverwritten;
    /*Matcher.addMatcher(
            varDecl(
                    hasAncestor(compoundStmt())
            ).bind("varDecl"), createStmtHandler("varDecl")); */
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
                                    
                                    //assignment to one time dereferred local pointer
                                    ignoringParenCasts(ignoringImplicit(
                                            unaryOperator(allOf(
                                                hasOperatorName("*"),
                                                hasUnaryOperand(ignoringParenCasts(ignoringImplicit(declRefExpr(to(varDecl(hasDeclContext(functionDecl())))))))
                                        ))
                                    )),

                                    //assignment to one time dereferred pointer, which is member of a local object
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
                            anyOf(//assure right side is an expressions
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
                    //hasAncestor(compoundStmt())
            ).bind("assignment"), createStmtHandler("assignment"));

//if OVERWRITTENASSIGNMENTOPERATORISASSIGNEMENT
        if(alsoOverwritten) {
            Matcher.addMatcher(//overwritten assignmentoperator call, rest like above
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
            ).bind("overwritten"), createStmtHandler("overwritten"));
        }
//endif
}
// clang-format on

std::string MVAEInjector::inject(StmtBinding current, ASTContext &Context) {
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());

    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);

    return getEditedString(R, Context);
}

bool MVAEInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    // if OVERWRITTENASSIGNMENTOPERATORISASSIGNEMENT
    if (binding.compare("overwritten") == 0) {
        const CXXOperatorCallExpr *opCall = (const CXXOperatorCallExpr *)stmt;
        if (!opCall->isInfixBinaryOp()) {
            return false;
        }
        if (!C2(stmt, Context)) {
            return false;
        }
        if (const ForStmt *forstmt = getParentOfType<ForStmt>(stmt, Context, 3)) {
            if (isParentOf(forstmt->getCond(), stmt) || isParentOf(forstmt->getInc(), stmt)) {
                return false;
            }
        }
        return true;
        // const Expr* arg = ((const CXXOperatorCallExpr*)stmt)->getArg(0);
        // return !isValue(arg->IgnoreImplicit()->IgnoreParenCasts()->IgnoreImplicit());
    }
    // endif
    if (const ForStmt *forstmt = getParentOfType<ForStmt>(stmt, Context, 3)) {
        return !isParentOf(forstmt->getCond(), stmt) && !isParentOf(forstmt->getInc(), stmt) && C2(stmt, Context);
    } else {
        return (C2(stmt, Context));
    }
    /*
    if(!C2(stmt, Context)) {
        stmt->getLocStart().dump(Context.getSourceManager());
        cerr<<endl;
    }
    return C2(stmt, Context);
    */
}
/*
bool MVAEInjector::checkStmt(const Decl* decl, std::string binding, ASTContext
&Context){
    return true;
        std::vector<const BinaryOperator*> list =
getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const
VarDecl*)decl, true, false, true);
        for(const BinaryOperator* op:list){

            if(isExprAssignment(op)
                //&&(isInitializedBefore((const DeclRefExpr*)((op)->getLHS()),
Context))
            ){
                if(const ForStmt* forstmt =
getParentOfType<ForStmt>(decl,Context,3)){
                    if(!isParentOf(forstmt->getCond(), decl, Context) &&
!isParentOf(forstmt->getInc(), decl,Context) && C2(op, Context)){
                        nodeCallback(binding, op);
                    }
                } else if(C2(op, Context)){
                    nodeCallback(binding, op);
                }
            }
        }

        return false;
}
*/

OMVAEInjector::OMVAEInjector()
    : MVAEInjector(true){

      };
