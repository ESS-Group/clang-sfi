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
                    anyOf(
                        ignoringImplicit(ignoringParenCasts(declRefExpr())),
                        ignoringImplicit(
                                cxxConstructExpr(
                                    hasArgument(0, ignoringImplicit(declRefExpr()))
                                )   
                            )
                        
                    )
                )
                    //implicit constructorcall + implicit cast
                    
                )
            ).bind("functionCall"),
        createStmtHandler("functionCall")
    );
}

std::string WPFVInjector::toString(){
    return "WPFV";
};

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
        if(param != arg && param->getType().getNonReferenceType().getDesugaredType(Context) == arg->getType().getNonReferenceType().getDesugaredType(Context) && localVariables.size()==0){
            localVariables.push_back(param);
        }else if(param == arg)
            isParam = true;
    }
    
    if(!isParam || localVariables.size()==0){
        for(DeclContext::decl_iterator it = declContext->decls_begin(), e = declContext->decls_end() ; it!=e;++it){
            const VarDecl* vardecl = (const VarDecl *)*it;
            if(isVisible(vardecl, stmt, Context) && vardecl != arg && arg->getType().getNonReferenceType().getDesugaredType(Context) == vardecl->getType().getNonReferenceType().getDesugaredType(Context)){
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
    for(auto i : getArgumentsOfType<DeclRefExpr>((const CallExpr*)stmt)){
        const VarDecl * arg = (VarDecl*)i->getDecl();
        const DeclContext* declContext = arg->getDeclContext();
        int varcount = 0;
        const FunctionDecl* fkt = getParentFunctionDecl(getParentOfType<DeclStmt>(arg, Context), Context);
        if(fkt == NULL){
            const FunctionDecl* _fkt = (const FunctionDecl*) declContext->getNonClosureAncestor();
            if(_fkt!= NULL){

            int paramCount = _fkt->getNumParams();
            


            for(int i = 0 ; i < paramCount ; i++){
                const VarDecl* param = _fkt->getParamDecl(i);
                if(param != arg && param->getType().getNonReferenceType().getDesugaredType(Context) == arg->getType().getNonReferenceType().getDesugaredType(Context)){
                    varcount++;
                    break;
                }
            }
            
            for(DeclContext::decl_iterator it = _fkt->decls_begin(), e = _fkt->decls_end() ; it!=e;++it){
                const VarDecl* vardecl = (const VarDecl *)*it;
                if(vardecl!=arg && vardecl->getType().getNonReferenceType().getDesugaredType(Context) == arg->getType().getNonReferenceType().getDesugaredType(Context)){
                    if(isVisible(vardecl, i, Context)){
                        varcount++;
                        break;
                    }
                }
            }
            
            if(varcount >= 1){
                nodeCallback(binding, i);
            }

            }
        } else if(fkt != NULL){
            int paramCount = fkt->getNumParams();

            for(int i = 0 ; i < paramCount ; i++){
                const VarDecl* param = fkt->getParamDecl(i);
                if(param != arg && param->getType() == arg->getType()){
                    varcount++;
                    break;
                }
            }
            
            for(DeclContext::decl_iterator it = fkt->decls_begin(), e = fkt->decls_end() ; it!=e;++it){
                const VarDecl* vardecl = (const VarDecl *)*it;
                if(vardecl!=arg && vardecl->getType().getNonReferenceType().getDesugaredType(Context) == arg->getType().getNonReferenceType().getDesugaredType(Context)){
                    if(isVisible(vardecl, i, Context)){
                        varcount++;
                        break;
                    }
                }
            }
            
            if(varcount >= 1){
                nodeCallback(binding, i);
            }
        } else { // variable is not local

        }
    }
    return false;
}