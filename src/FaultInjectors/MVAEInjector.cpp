MVAEInjector::MVAEInjector(){
        Matcher.addMatcher(
        varDecl(
                hasInitializer(
                    allOf(
                        expr(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator())),
                        hasAncestor(compoundStmt())
                    )
                )
            ).bind("variable"), createStmtHandler("variable"));




    Matcher.addMatcher(
            varDecl(
                    hasAncestor(compoundStmt())
            ).bind("notInitialized"), createStmtHandler("notInitialized"));

}

std::string MVAEInjector::toString(){
    return "MVAE";
};




std::string MVAEInjector::inject(StmtBinding current, ASTContext &Context){

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


//int i = 0;
bool MVAEInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
//cout << "oha"<<++i<<endl;
//i++;
//decl -> dumpColor();
//cout << i<<endl;
    if(binding.compare("notInitialized") == 0 && isa<VarDecl>(decl) /*&& false /**//*&&i++ == 0*/){
        /*if(decl != NULL)
            cout << "DECL: " << stmtToString (decl, Context.getLangOpts())<<endl;
        else
            cout << "ERROR"<<endl;*/
        //getParentCompoundStmt(decl, Context)->dump(Context.getSourceManager());
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        //cout << "SIZE: "<<list.size()<<endl;
        for(const BinaryOperator* op:list){
            
            if(isExprAssignment(op)){
                //op->dump(Context.getSourceManager());
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){

            //cout<<"-----------4"<<endl;
                    if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){

            //cout<<"-----------5"<<endl;
                    } else if(C2(op, Context)){

            //cout<<"-----------6"<<endl;
                        nodeCallback(binding, op);
                    }
                } else if(C2(op, Context)){

            //cout<<"-----------7"<<endl;
                    nodeCallback(binding, op);
                }
                
            }
            //cout<<"-----------10"<<endl;
        }

        return false;
        /*cout<<stmtToString(decl, Context.getLangOpts())<<endl;

        cout<<"------------"<<(((const VarDecl*)decl)->hasInit()?1:0)<<endl;
        ((const VarDecl*)decl)->getInit()->dump(Context.getSourceManager());*/
        //decl->getDeclContext()->dumpDeclContext();
    }else{
        //return false;
        //cout << "MVAE TYPE2 - 1"<<endl;
        //decl->dumpColor();
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){

            //cout << "MVAV TYPE2 - 2"<<i << endl;
            if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){

            } else {

            //cout << "MVAV TYPE2 - 3"<<i << endl;
               return C2(decl, Context); 
            }

            //cout << "MVAV TYPE2 - 4"<<i << endl;
        } else { 

            //cout << "MVAV TYPE2 - 5"<<i << endl;
            return C2(decl, Context);
        }
    }
    //cout<<stmtToString(decl, Context.getLangOpts())<<endl;
    
    return false;
}