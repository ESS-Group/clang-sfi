MLOCInjector::MLOCInjector(){
    Matcher.addMatcher(binaryOperator(anyOf(hasAncestor(expr(anyOf(hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt())))),hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt()))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
}

std::string MLOCInjector::toString(){
    return "MLOC";
};
std::string MLOCInjector::inject(StmtBinding current, ASTContext &Context, bool left){

    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceLocation start, end;
    if(left){
        start = ((const BinaryOperator *)current.stmt)->getOperatorLoc();//.getLocWithOffset(-1);//.getLocWithOffset(1);
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();
    }else {
        start = ((const BinaryOperator *)current.stmt)->getLHS()->getLocStart();
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocStart().getLocWithOffset(-1);
    }


    SourceRange range(start, end);
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MLOCInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else

    return ((const BinaryOperator *)stmt)->getOpcode() == 19;
}
void MLOCInjector::inject(std::vector<StmtBinding> target, ASTContext &Context){
    int i = 0;
    for(StmtBinding current : target){
        if(verbose)
            printStep(current, Context.getSourceManager(), Context.getLangOpts(),2*(i++),target.size()*2);
        else
            printStep(current, Context.getSourceManager(),i++,target.size()*2);
        std::string result = inject(current, Context, true);
        if(result.compare("")){
            cout<<" -Success"<<endl;
            writeDown(result, i-1);
        } else
            cerr << "-Failed"<<endl;


        if(verbose)
            printStep(current, Context.getSourceManager(), Context.getLangOpts(),2*(i++),target.size()*2);
        else
            printStep(current, Context.getSourceManager(),i++,target.size()*2);
        result = inject(current, Context, false);
        if(result.compare("")){
            cout<<" -Success"<<endl;
            writeDown(result, i-1);
        } else
            cerr << "-Failed"<<endl;
    }
}
std::string MLOCInjector::inject(StmtBinding current, ASTContext &Context){
    return "";
}