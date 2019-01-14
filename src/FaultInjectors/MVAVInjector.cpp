#include "_all.h"

std::string MVAVInjector::toString() {
    return "MVAV";
};
std::string OMVAVInjector::toString() {
    return "OMVAV";
};

// clang-format off
MVAVInjector::MVAVInjector(bool alsoOverwritten) {
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
            ).bind("overwritten"), createStmtHandler("overwritten"));
        }
//endif
}
// clang-format on

std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context) {
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());

    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);

    return getEditedString(R, Context);
}

bool MVAVInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    // if OVERWRITTENASSIGNMENTOPERATORISASSIGNEMENT

    if (binding.compare("overwritten") == 0) {
        const CXXOperatorCallExpr *opCall = cast<CXXOperatorCallExpr>(stmt);
        if (!opCall->isInfixBinaryOp()) {
            return false;
        }
        if (!C2(stmt, Context)) {
            return false;
        }
        if (const ForStmt *forstmt = getParentOfType<ForStmt>(stmt, Context, 3))
            if (isParentOf(forstmt->getCond(), stmt) || isParentOf(forstmt->getInc(), stmt)) {
                return false;
            }
        return true;

        // const Expr* arg = ((const CXXOperatorCallExpr*)stmt)->getArg(0);
        // return
        // isValue(arg->IgnoreImplicit()->IgnoreParenCasts()->IgnoreImplicit());
    }
    // endif
    if (const ForStmt *forstmt = getParentOfType<ForStmt>(stmt, Context, 3)) {
        return !isParentOf(forstmt->getCond(), stmt) && !isParentOf(forstmt->getInc(), stmt) && C2(stmt, Context);
    } else {
        return (C2(stmt, Context));
    }
    /*if(!C2(stmt, Context)){
    stmt->getLocStart().dump(Context.getSourceManager());
    cerr<<endl;
    }
    return C2(stmt, Context);*/
}
/*
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
    std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl,
                                            true, //also search in loops
                                            false, //do not search in for constructs
                                            true); //do not check initialization => use every assignment
    for(const BinaryOperator* op:list){
        if(
            isValueAssignment(op)
            //&& isInitializedBefore((const DeclRefExpr*)((op)->getLHS()), Context)
        ){
            if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3*/ /*5*/ /*)){
if(!isParentOf(forstmt->getCond(), decl, Context) &&
!isParentOf(forstmt->getInc(), decl,Context)){ // not part of for construct!!!
nodeCallback(binding, op);
}
} else {
nodeCallback(binding, op);
}
}
}

return false;
}*/

OMVAVInjector::OMVAVInjector()
    : MVAVInjector(true) {

      };
