WAEPInjector::WAEPInjector(){
    Matcher.addMatcher(
        callExpr(
            allOf(
                unless(anyOf(
                    cudaKernelCallExpr(),
                    cxxOperatorCallExpr(),
                    userDefinedLiteral()
                )),
                hasDescendant(expr(anyOf(
                    binaryOperator(),
                    hasDescendant(binaryOperator())
                )))
            )
        ).bind("functionCall"),
        createStmtHandler("functionCall")
    );
    //Matcher.addMatcher(callExpr(unless(anyOf(hasParent(varDecl(isDefinition())), hasParent(returnStmt()), cxxOperatorCallExpr(), hasParent(callExpr()), hasParent(expr())))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    //Matcher.addMatcher(callExpr().bind("FunctionCall"), createStmtHandler("FunctionCall"));
}

std::string WAEPInjector::toString(){
    return "WAEP";
};
bool isArithmetic(const BinaryOperator* op){
    int code = op->getOpcode();
    return code == BinaryOperatorKind::BO_Mul||
        code == BinaryOperatorKind::BO_Div ||
        code == BinaryOperatorKind::BO_Rem ||
        code == BinaryOperatorKind::BO_Add ||
        code == BinaryOperatorKind::BO_Sub ||
        code == BinaryOperatorKind::BO_Shl ||
        code == BinaryOperatorKind::BO_Shr ||
        code == BinaryOperatorKind::BO_And ||
        code == BinaryOperatorKind::BO_Or ||
        code == BinaryOperatorKind::BO_Xor;
}
const BinaryOperator* getBinaryOperatorWithRightedtRHS(const BinaryOperator* op){
    if(isa<BinaryOperator>(op->getRHS()) && isArithmetic(op)){
        return getBinaryOperatorWithRightedtRHS((const BinaryOperator*)op->getRHS());
    } else
        return op;
}
template<class T>
std::vector<const T*> getChildrenFlat(const Stmt *parent){
    std::vector<const T*> ret;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i != NULL){
            //if(isa<T>(*i)){
            if(isa<Expr>(*i)){
                const Expr* expr = ((const T *) *i)->IgnoreImplicit()->IgnoreParenCasts();
                if(isa<T>(expr))
                    ret.push_back((const T*)expr);
                //ret.push_back(((const T *) *i)->IgnoreImplicit()->IgnoreParenCasts());
            } else if(isa<T>(*i)){
                ret.push_back(((const T *) *i));
            }
           // }
           /* else {
                if(const T* ret = getFirstChild<T>(*i))
                    return ret; 
            }*/
        }
    }
    return ret;
}
std::string WAEPInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceLocation start, end;

    start = ((const BinaryOperator *)current.stmt)->getOperatorLoc();//.getLocWithOffset(-1);//.getLocWithOffset(1);
    //TODO:MLAC und MLOC checken!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();



    SourceRange range(start, end);
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool WAEPInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    if(hasChildOfType<BinaryOperator>(stmt)){
        std::vector<const BinaryOperator*> arguments = getChildrenFlat<BinaryOperator>(stmt);
        for(const BinaryOperator* op : arguments){
            //int code = op->getOpcode();
            if(isArithmetic(op)){
            const BinaryOperator* rightest = getBinaryOperatorWithRightedtRHS(op);

            //rightest->getRHS()->dumpColor();

            //const Expr* temp = rightest->IgnoreParenCasts();
            //if(rightest->getType().getDesugaredType(Context).getNonReferenceType() == rightest->getLHS()->getType().getDesugaredType(Context).getNonReferenceType())
                nodeCallback(binding, rightest);
            /*else{
                rightest->getLocStart().dump(Context.getSourceManager());
                cerr<<endl;
                rightest->dumpColor();
                rightest->getType().dump();
                rightest->getLHS()->getType().dump();
            /*}*/
            }
        }
    }
    return false;
}