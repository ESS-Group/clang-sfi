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
template<class T>
std::vector<const T*> getArgumentsOfType(const CallExpr* call){
    std::vector<const T*> ret;
    const Expr*const * args = call->getArgs();
    for(int i = 0 ; i < call->getNumArgs() ; i++){
        const Expr* arg = args[i];
        //cout << 1<<endl;
        if(arg->IgnoreImpCasts() != NULL && isa<T>(arg->IgnoreImpCasts())){
            ret.push_back((const T*)arg->IgnoreImpCasts());
        }
        //cout << 2<<endl;
    }
    return ret;
}

bool isVisible(const Decl* decl, const Stmt* position, ASTContext &Context){
    if(position == NULL || decl == NULL)
        return false;
    //cout << 1<<endl;
    const DeclStmt * declstmt =  getParentOfType<DeclStmt>(decl, Context);
    //cout << 2<<endl;
    if(declstmt == NULL)
        return false;
    ASTContext::DynTypedNodeList list = Context.getParents(*declstmt);
    if(!list.empty()){
        const Stmt* parent = list[0].get<Stmt>();
        /*cout<<"--"<<endl;
        parent->dumpColor();
        cout<<"++"<<endl;*/
        if(parent == NULL)
            return false;
        /*parent->dumpColor();
        if(isParentOf(parent, position))
            cout << "testitest"<<endl;
        if(decl->getLocEnd()<position->getLocStart())
            cout << "huhu"<<endl;*/
        if(isParentOf(parent, position) && decl->getLocEnd()<position->getLocStart())
            return true;
    }
    return false;
}
std::string WPFVInjector::inject(StmtBinding current, ASTContext &Context){
    //cout << "INJECCTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"<<endl;
    const DeclRefExpr* stmt = (const DeclRefExpr*)current.stmt;

    

    const DeclContext* declContext = stmt->getDecl()->getDeclContext();
    //int varcount = 0;
    const FunctionDecl* fkt = (const FunctionDecl*) declContext->getNonClosureAncestor();



    std::vector<const VarDecl*> localVariables;
    int paramCount = fkt->getNumParams();
    for(int i = 0 ; i < paramCount ; i++){
        const VarDecl* param = fkt->getParamDecl(i);
        if(param != stmt->getDecl())
            localVariables.push_back(param);
    }
    
    for(DeclContext::decl_iterator it = declContext->decls_begin(), e = declContext->decls_end() ; it!=e;++it){
        const VarDecl* vardecl = (const VarDecl *)*it;
        if(isVisible(vardecl, stmt, Context) && vardecl != stmt->getDecl()){
            //vardecl->dumpColor();
            localVariables.push_back(vardecl);
        }
        
    }
    ///cout << "Lokale sichtbare andere Variablen: "<<localVariables.size()<<endl;
    std::string variableName(localVariables[0]->getName().data());
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    
    R.ReplaceText(stmt->getSourceRange(), variableName);
    return getEditedString(R, Context);
    /*Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceLocation start, end;

    start = ((const BinaryOperator *)current.stmt)->getOperatorLoc();//.getLocWithOffset(-1);//.getLocWithOffset(1);
    //TODO:MLAC und MLOC checken!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();



    SourceRange range(start, end);
    R.RemoveText(range);*/
    //return "";
}
//int i = 0;
bool WPFVInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    //stmt->dumpColor();
    //if(i++>0)
    //    return false;
    for(auto i : getArgumentsOfType<DeclRefExpr>((const CallExpr*)stmt)){
        /*cout<<"---"<<endl;
        i->dumpColor();
        cout<<"+++"<<endl;*/
        //cout << (i->getDecl()->getDeclContext()->isFunctionOrMethod()?"1":"0")<<endl;
        
        const DeclContext* declContext = i->getDecl()->getDeclContext();
        int varcount = 0;

        const FunctionDecl* fkt = (const FunctionDecl*) declContext->getNonClosureAncestor();
        int paramCount = fkt->getNumParams();
        varcount += paramCount;
        /*for(FunctionDecl::param_iterator it = cast_away_const(fkt->param_begin()), e = cast_away_const(fkt->param_end()) ; it!=e;++it){
            varcount++;
        }*/
        for(DeclContext::decl_iterator it = declContext->decls_begin(), e = declContext->decls_end() ; it!=e;++it){
            //cout << 1<<endl;
            const VarDecl* vardecl = (const VarDecl *)*it;
            //cout << 2<<endl;
            if(isVisible(vardecl, i, Context))
                varcount++;//it->dumpColor();
            //cout << 3<<endl;
            
        }
        //cout << 2<<endl;
        if(varcount >= 2)///////////////////////////////////////////////////////maybe >=1 !!!!!  because i do not add the variable that is replaced
            nodeCallback(binding, i);
        //i->getDecl()->getDeclContext()->dumpDeclContext();
    }
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