MVAVInjector::MVAVInjector(){
    /*
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    Matcher.addMatcher(stmt(allOf(binaryOperator(hasOperatorName("=")),
    unless(anyOf(hasDescendant(callExpr()),allOf(hasDescendant(binaryOperator()), unless(hasDescendant(binaryOperator(hasOperatorName("="))))),hasDescendant(unaryOperator()),hasDescendant(cxxNewExpr()),hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt()))))).bind("variable"), createStmtHandler("variable"));
    //hasDescendant(unaryOperator())
    //allOf(hasDescendant(binaryOperator()), unless(hasDescendant(binaryOperator(hasOperatorName("=")))))
    //hasDescendant(callExpr()),hasDescendant(cxxNewExpr()),hasDescendant(binaryOperator()),hasDescendant(unaryOperator())
    Matcher.addMatcher(varDecl(allOf(hasInitializer(unless(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator()))),
                        unless(anyOf(hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt()))))).bind("variable1"), createStmtHandler("variable1"));

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
    */
    /*Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(anyOf(
                callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
                hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
            )),
            hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));
        /**//*Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(anyOf(
                callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator()//,
                //hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
            )
            ),
            //unless(hasParent(forStmt())),
            hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));
/**/
        Matcher.addMatcher(
        varDecl(//allOf(
                hasInitializer(
                    expr(unless(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),unless(hasAncestor(compoundStmt())))))
                    // allOf(
                    /*unless(anyOf(
                        callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),unless(hasAncestor(compoundStmt()))//,
                        //hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
                    )
                    //),
                    //unless(hasParent(forStmt())),
                    
                    )*/
                )//,
                //hasAncestor(compoundStmt())
            ).bind("variable"), createStmtHandler("variable"));
/**/



    Matcher.addMatcher(
        //callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
            varDecl(
                    //allOf(
                    //unless(hasInitializer(expr())),
                    hasAncestor(compoundStmt())
                    //,unless(varDecl(hasInitializer(expr())))
                    //)
            ).bind("notInitialized"), createStmtHandler("notInitialized")); // in this case get next assignement

}

std::string MVAVInjector::toString(){
    return "MVAV";
};




std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    //return "";
    //((const VarDecl*)current.decl)->getAnyInitializer()->dump(Context.getSourceManager());
    //((const VarDecl*)current.decl)->setInit(NULL);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if(current.isStmt){
        //const Stmt *temp = (const Stmt*)current.stmt;
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
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
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
            
            if(isValueAssignment(op)){
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
        //cout << "MVAV TYPE2 - 1"<<i<<endl;
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

/*
std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    return "";
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
    cout<<"decl - "<<stmtToString(decl, Context.getLangOpts())<<endl;
    
    return false;
}*/
/*
bool MVAVInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    
    const BinaryOperator* op = (const BinaryOperator*)stmt;
    //if(op->getOpcode()==20){//=
    cout<<stmtToString(op, Context.getLangOpts())<<endl;
    //}
    return false;
}
*/
