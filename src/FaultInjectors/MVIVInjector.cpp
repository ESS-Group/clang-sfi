MVIVInjector::MVIVInjector(){//Missing if construct plus statements
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    //anyOf(hasParent(anyOf(forStmt(), whileStmt(),doStmt()))
    Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(anyOf(
                callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
                hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
            )),
            hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));

    Matcher.addMatcher(
        //callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
            varDecl(
                    allOf(
                    unless(anyOf(
                        hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
                    )),
                    hasAncestor(compoundStmt()),
                    unless(varDecl(hasInitializer(expr())))
                    )
            ).bind("notInitialized"), createStmtHandler("notInitialized")); // in this case get next assignement

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
}

std::string MVIVInjector::toString(){
    return "MVIV";
};
std::string MVIVInjector::inject(StmtBinding current, ASTContext &Context){

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
    /*
    return "";
    
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
    */
}



/*
std::vector<const BinaryOperator*> getInitializationOfUninitializedVar(const VarDecl* var, ASTContext &Context){
    std::vector<const BinaryOperator*> ret;
    const CompoundStmt *scope = getParentCompoundStmt(var, Context);

}
*/
//int i = 0;
bool MVIVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
    if(binding.compare("notInitialized") == 0 && isa<VarDecl>(decl) /*&&i++ == 0*/){
        //if(decl != NULL)
        //cout << "DECL: " << stmtToString (decl, Context.getLangOpts())<<endl;
        //getParentCompoundStmt(decl, Context)->dump(Context.getSourceManager());
        std::vector<const BinaryOperator*> list = getChildForFindInitForVar(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, false);
        //cout << "SIZE: "<<list.size()<<endl;
        for(const BinaryOperator* op:list){
            
            if(isValueAssignment(op) && C2(op, Context)){
                //op->dump(Context.getSourceManager());
                nodeCallback(binding, op);
            }
            //cout<<"-----------"<<endl;
        }

        return false;
        //cout<<stmtToString(decl, Context.getLangOpts())<<endl;

        //cout<<"------------"<<(((const VarDecl*)decl)->hasInit()?1:0)<<endl;
        ((const VarDecl*)decl)->getInit()->dump(Context.getSourceManager());
        //decl->getDeclContext()->dumpDeclContext();
    }else
        return C2(decl, Context);
    //cout<<stmtToString(decl, Context.getLangOpts())<<endl;
    
    //return false;
}