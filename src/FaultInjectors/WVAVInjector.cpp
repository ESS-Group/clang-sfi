WVAVInjector::WVAVInjector(bool alsoOverwritten){
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

std::string WVAVInjector::toString(){
    return "WVAV";
};


bool WVAVInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){
//if OVERWRITTENASSIGNMENTOPERATORISASSIGNEMENT
    if(binding.compare("overwritten") == 0){
        const CXXOperatorCallExpr* opCall = (const CXXOperatorCallExpr*)stmt;
        if(!opCall->isInfixBinaryOp())
            return false;
        /*if(!C2(stmt, Context))
            return false;*/
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(stmt,Context,3))
            if(isParentOf(forstmt->getCond(), stmt) || isParentOf(forstmt->getInc(), stmt))
                return false;
        return true;
        
        //const Expr* arg = ((const CXXOperatorCallExpr*)stmt)->getArg(0);
        //return isValue(arg->IgnoreImplicit()->IgnoreParenCasts()->IgnoreImplicit());
    }
//endif
//stmt->dumpColor();
//stmt->getLocStart().dump(Context.getSourceManager());
//cerr<<endl;
    if(const ForStmt* forstmt = getParentOfType<ForStmt>(stmt,Context,3)){
//        cerr<<"1"<<endl;
        return !isParentOf(forstmt->getCond(), stmt) && !isParentOf(forstmt->getInc(), stmt);// && C2(stmt, Context);
    }
    else return true;//return(C2(stmt, Context));
    /*if(!C2(stmt, Context)){
    stmt->getLocStart().dump(Context.getSourceManager());
    cerr<<endl;
    }
    return C2(stmt, Context);*/
}
/*
bool WVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
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
}
*/
std::string WVAVInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    Expr* val = NULL;
//if OVERWRITTENASSIGNMENTOPERATORISASSIGNEMENT
    //if(alsoOverwritten){
    if(current.binding.compare("overwritten") == 0){
        val=(Expr*)(&*((const CXXOperatorCallExpr*)current.stmt)->getArg(1));
    } else 
        val = ((const BinaryOperator *)current.stmt)->getRHS();
    //} else {
//elif
    //    val = ((const BinaryOperator *)current.stmt)->getRHS();
    //}
//endif
    SourceRange range(val->getLocStart(), val->getLocEnd());
    if(isa<CXXBoolLiteralExpr>(val)){
        bool value = ((const CXXBoolLiteralExpr *)val)->getValue();
        if(value){
            R.ReplaceText(range, "false");
        } else {
            R.ReplaceText(range, "true");
        }
    } else {
        std::string text = R.getRewrittenText(range);
        R.ReplaceText(range, text+"^0xFF");
    }

    return getEditedString(R, Context);
}



std::string OWVAVInjector::toString(){
    return "OWVAV";
};
OWVAVInjector::OWVAVInjector():WVAVInjector(true){
    
};