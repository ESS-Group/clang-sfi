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
        if(arg->IgnoreImpCasts() != NULL && isa<T>(arg->IgnoreImpCasts())){
            ret.push_back((const T*)arg->IgnoreImpCasts());
        }
    }
    return ret;
}

bool isVisible(const Decl* decl, const Stmt* position, ASTContext &Context){
    if(position == NULL || decl == NULL)
        return false;
        
    const DeclStmt * declstmt =  getParentOfType<DeclStmt>(decl, Context);
    
    if(declstmt == NULL)
        return false;
    ASTContext::DynTypedNodeList list = Context.getParents(*declstmt);
    if(!list.empty()){
        const Stmt* parent = list[0].get<Stmt>();
        
        if(parent == NULL)
            return false;

        if(isParentOf(parent, position) && decl->getLocEnd()<position->getLocStart())
            return true;
    }
    return false;
}
std::string WPFVInjector::inject(StmtBinding current, ASTContext &Context){
    const DeclRefExpr* stmt = (const DeclRefExpr*)current.stmt;

    
    const VarDecl * arg = (VarDecl*)stmt->getDecl();
    const DeclContext* declContext = arg->getDeclContext();
    const FunctionDecl* fkt = (const FunctionDecl*) declContext->getNonClosureAncestor();

    bool isParam = true;

    std::vector<const VarDecl*> localVariables;
    int paramCount = fkt->getNumParams();
    for(int i = 0 ; i < paramCount ; i++){
        const VarDecl* param = fkt->getParamDecl(i);
        if(param != arg && param->getType() == arg->getType() && localVariables.size()==0){
            localVariables.push_back(param);
        }else if(param == arg)
            isParam = true;
    }
    
    if(!isParam || localVariables.size()==0){
        for(DeclContext::decl_iterator it = declContext->decls_begin(), e = declContext->decls_end() ; it!=e;++it){
            const VarDecl* vardecl = (const VarDecl *)*it;
            if(isVisible(vardecl, stmt, Context) && vardecl != arg && arg->getType() == vardecl->getType()){
                if(!isParam)
                    localVariables.clear();
                localVariables.push_back(vardecl);
                break;
            }
            
        }
    }
    std::string variableName(localVariables[0]->getName().data());
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    
    R.ReplaceText(stmt->getSourceRange(), variableName);
    return getEditedString(R, Context);
}

bool WPFVInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){
    //cout<<"WPFV"<< 1 <<endl;
    for(auto i : getArgumentsOfType<DeclRefExpr>((const CallExpr*)stmt)){
        //cout<<"WPFV"<< 1.1 <<endl;
        const VarDecl * arg = (VarDecl*)i->getDecl();
        //const DeclContext* declContext = arg->getDeclContext();
        int varcount = 0;
        //cout<<"WPFV"<< 1.2 <<endl;
        const FunctionDecl* fkt = getParentFunctionDecl(getParentOfType<DeclStmt>(arg, Context), Context);
        if(fkt != NULL){
            //cout<<"WPFV"<< 1.21 <<endl;
            //if(fkt == NULL){
                //cout<<"NULL"<<endl;
                //stmt->getLocStart().dump(Context.getSourceManager());
            //}

            int paramCount = fkt->getNumParams();
            //cout<<"WPFV"<< 1.3 <<endl;
            


            for(int i = 0 ; i < paramCount ; i++){
                const VarDecl* param = fkt->getParamDecl(i);
                if(param != arg && param->getType() == arg->getType()){
                    varcount++;
                    break;
                }
            }
            //cout<<"WPFV"<< 1.4 <<endl;
            
            for(DeclContext::decl_iterator it = fkt->decls_begin(), e = fkt->decls_end() ; it!=e;++it){
                const VarDecl* vardecl = (const VarDecl *)*it;
                if(vardecl!=arg && vardecl->getType() == arg->getType()){
                    if(isVisible(vardecl, i, Context)){
                        varcount++;
                        break;
                    }
                }
            }
            //cout<<"WPFV"<< 1.5 <<endl;
            
            if(varcount >= 1)
                nodeCallback(binding, i);
        } else { // variable is not local

        }
    }
    return false;
}