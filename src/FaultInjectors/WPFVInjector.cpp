WPFVInjector::WPFVInjector(){
    Matcher.addMatcher(
        callExpr(
            allOf(
                unless(anyOf(
                    cudaKernelCallExpr(),
                    cxxOperatorCallExpr(),
                    userDefinedLiteral()
                )),
                hasAnyArgument(
                    anyOf(declRefExpr(),
                    implicitCastExpr(hasSourceExpression(declRefExpr()))
                    )
                    
                )
            )
        ).bind("functionCall"),
        createStmtHandler("functionCall")
    );
    //Matcher.addMatcher(callExpr(unless(anyOf(hasParent(varDecl(isDefinition())), hasParent(returnStmt()), cxxOperatorCallExpr(), hasParent(callExpr()), hasParent(expr())))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    //Matcher.addMatcher(callExpr().bind("FunctionCall"), createStmtHandler("FunctionCall"));
}

std::string WPFVInjector::toString(){
    return "WPFV";
};
/*
const BinaryOperator* getBinaryOperatorWithRightedtRHS(const BinaryOperator* op){
    if(isa<BinaryOperator>(op->getRHS())){
        return getBinaryOperatorWithRightedtRHS((const BinaryOperator*)op->getRHS());
    } else
        return op;
}
template<class T>
std::vector<const T*> getChildrenFlat(const Stmt *parent){
    std::vector<const T*> ret;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i != NULL){
            if(isa<T>(*i)){
               ret.push_back((const T *) *i);
            }/* else {
                if(const T* ret = getFirstChild<T>(*i))
                    return ret; 
            }*//*
        }
    }
    return ret;
}
*/
std::string WPFVInjector::inject(StmtBinding current, ASTContext &Context){
    /*Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceLocation start, end;

    start = ((const BinaryOperator *)current.stmt)->getOperatorLoc();//.getLocWithOffset(-1);//.getLocWithOffset(1);
    //TODO:MLAC und MLOC checken!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();



    SourceRange range(start, end);
    R.RemoveText(range);
    return getEditedString(R, Context);*/
}
bool WPFVInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    stmt->dumpColor();
    return false;
    /*if(hasChildOfType<BinaryOperator>(stmt)){
        std::vector<const BinaryOperator*> arguments = getChildrenFlat<BinaryOperator>(stmt);
        for(const BinaryOperator* op : arguments){
            const BinaryOperator* rightest = getBinaryOperatorWithRightedtRHS(op);

            //rightest->getRHS()->dumpColor();
            nodeCallback(binding, rightest);
        }
    }
    return false;*/
}