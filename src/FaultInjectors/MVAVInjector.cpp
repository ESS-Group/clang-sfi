MVAVInjector::MVAVInjector(bool alsoOverwritten){
    this->alsoOverwritten = alsoOverwritten;
    Matcher.addMatcher(
            binaryOperator(
                    allOf(
                        hasOperatorName("="),
                        hasLHS(
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
                        hasRHS(
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
            if(alsoOverwritten){
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
            ).bind("overwritten"), createStmtHandler("overwritten"));
        }
//endif
}

std::string MVAVInjector::toString(){
    return "MVAV";
};




std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    
    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);
    
    return getEditedString(R, Context);
}

bool MVAVInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){

//if OVERWRITTENASSIGNMENTOPERATORISASSIGNEMENT
    
    if(binding.compare("overwritten") == 0){
        const CXXOperatorCallExpr* opCall = (const CXXOperatorCallExpr*)stmt;
        if(!opCall->isInfixBinaryOp())
            return false;
        if(!C2(stmt, Context))
            return false;
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(stmt,Context,3))
            if(isParentOf(forstmt->getCond(), stmt) || isParentOf(forstmt->getInc(), stmt))
                return false;
        return true;
        
        //const Expr* arg = ((const CXXOperatorCallExpr*)stmt)->getArg(0);
        //return isValue(arg->IgnoreImplicit()->IgnoreParenCasts()->IgnoreImplicit());
    }
//endif
    if(const ForStmt* forstmt = getParentOfType<ForStmt>(stmt,Context,3))
        return !isParentOf(forstmt->getCond(), stmt) && !isParentOf(forstmt->getInc(), stmt) && C2(stmt, Context);
    else return(C2(stmt, Context));
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
            if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3/*5*//*)){
                if(!isParentOf(forstmt->getCond(), decl, Context) && !isParentOf(forstmt->getInc(), decl,Context)){ // not part of for construct!!!
                    nodeCallback(binding, op);
                }
            } else {
                nodeCallback(binding, op);
            }
        }
    }

    return false;
}*/

OMVAVInjector::OMVAVInjector():MVAVInjector(true){
    
};
std::string OMVAVInjector::toString(){
    return "OMVAV";
};