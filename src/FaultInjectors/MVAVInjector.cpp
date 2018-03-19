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
            ).bind("varDecl"), createStmtHandler("varDecl")); 

}

std::string MVAVInjector::toString(){
    return "MVAV";
};




std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    //if(current.isStmt){
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    /*} else {
        VarDecl temp (*((const VarDecl*)current.decl));
        temp.setInit(NULL);
        const VarDecl* tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());
        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
        R.ReplaceText(range, withoutInit);
    }*/

    return getEditedString(R, Context);
}
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
    if(binding.compare("varDecl") == 0 && isa<VarDecl>(decl)){
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        for(const BinaryOperator* op:list){
            if(isValueAssignment(op) && 
            (isInitializedBefore((const DeclRefExpr*)((op)->getLHS()), Context) 
//            ||
//            isInitializationInLoop((const DeclRefExpr*)((op)->getLHS())
            )
            ){
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
                    if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){
                    //} else if(C2(op, Context)){
                    //} else if(getParentOfType<CompoundStmt>(decl,Context,3) != NULL){//local variable
                    }else{
                        nodeCallback(binding, op);
                    }
                //} else if(C2(op, Context)){
                } else {
                //commented to include assignements inside a for construct
                    nodeCallback(binding, op);
                }
            }
        }

        return false;
    }/*else{
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
            if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){
            } else {
               return C2(decl, Context); 
            }
        } else { 
            return C2(decl, Context);
        }
    }*/
    return false;
}
/*
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
    if(binding.compare("varDecl") == 0 && isa<VarDecl>(decl)){
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        //for(auto i:list){
        //    i->dumpColor();
        //}
        //cout<<"---"<<endl;
        //std::vector<const BinaryOperator*> initList = getChildForFindInitForVar(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        //for(auto i:initList){
        //    i->dumpColor();
        //}
        //cout<<"---"<<endl;
        
        //cout<<"SIZE:"<<list.size()<<endl;
        //deleteFromList<const BinaryOperator*>(list, initList);
        //cout<<"SIZE:"<<list.size()<<endl;
        for(const BinaryOperator* op:list){
            if(isValueAssignment(op)&& 
            (isInitializedBefore((const DeclRefExpr*)((op)->getLHS()), Context) )){
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
                    if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){

                //cout<<"nein isset net!!:O"<<endl;
                    } else if(C2(op, Context)){
                        nodeCallback(binding, op);
                    }
                } else if(C2(op, Context)){
                //commented to include assignements inside a for construct
                    nodeCallback(binding, op);
                }
            } else {
                //cout<<"nein isset net!!"<<endl;
            }
        }

        return false;
    }/*else{
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
            if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){
            } else {
               return C2(decl, Context); 
            }
        } else { 
            return C2(decl, Context);
        }
    }*//*
    return false;
}
*/