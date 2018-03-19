WVAVInjector::WVAVInjector(){
    Matcher.addMatcher(
            varDecl(
                    hasAncestor(compoundStmt())
            ).bind("varDecl"), createStmtHandler("varDecl")); 

}

std::string WVAVInjector::toString(){
    return "WVAV";
};

std::string WVAVInjector::inject(StmtBinding current, ASTContext &Context){
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());

    Expr* val = ((const BinaryOperator *)current.stmt)->getRHS();
    SourceRange range(val->getLocStart(), val->getLocEnd());
    if(isa<CXXBoolLiteralExpr>(val)){
        bool value = ((const CXXBoolLiteralExpr *)val)->getValue();
        if(value){
            R.ReplaceText(range, "false");
        } else {
            R.ReplaceText(range, "true");
        }
    } else {
        std::string text = R.getRewrittenText(range);
        R.ReplaceText(range, text+"^0xFF");
    }

    return getEditedString(R, Context);
}


bool WVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){
    if(binding.compare("varDecl") == 0 && isa<VarDecl>(decl)){
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        for(const BinaryOperator* op:list){
            if(
                isValueAssignment(op) && 
                isInitializedBefore((const DeclRefExpr*)((op)->getLHS()), Context)
            ){
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){
                    if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){
                    }else{
                        nodeCallback(binding, op);
                    }
                } else {
                    nodeCallback(binding, op);
                }
            }
        }

        return false;
    }
    return false;
}