MVAVInjector::MVAVInjector(){
/*
        Matcher.addMatcher(
        varDecl(
                hasInitializer(
                    expr(unless(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),unless(hasAncestor(compoundStmt())))))
                )
            ).bind("variable"), createStmtHandler("variable"));
*/

    Matcher.addMatcher(
            varDecl(
                    hasAncestor(compoundStmt())
            ).bind("notInitialized"), createStmtHandler("notInitialized")); 

}

std::string MVAVInjector::toString(){
    return "MVAV";
};




std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if(current.isStmt){
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        VarDecl temp (*((const VarDecl*)current.decl));
        temp.setInit(NULL);
        const VarDecl* tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());
        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
        R.ReplaceText(range, withoutInit);
    }

    return getEditedString(R, Context);
}


bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
    if(binding.compare("notInitialized") == 0 && isa<VarDecl>(decl)){
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        for(const BinaryOperator* op:list){
            if(isValueAssignment(op)){
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
                    if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){
                    } else if(C2(op, Context)){
                        nodeCallback(binding, op);
                    }
                } else if(C2(op, Context)){
                    nodeCallback(binding, op);
                }
            }
        }

        return false;
    }else{
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
            if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){
            } else {
               return C2(decl, Context); 
            }
        } else { 
            return C2(decl, Context);
        }
    }
    return false;
}