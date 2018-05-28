/*
void MLACInjector::inject(std::vector<StmtBinding> target, ASTContext &Context){
    int i = 0;
    for(StmtBinding current : target){
        std::string result = "";
        if(i%2==0){
            if(verbose)
                printStep(current, Context.getSourceManager(), Context.getLangOpts(),i++,target.size()/2);
            else
                printStep(current, Context.getSourceManager(),i++,target.size()/2);
            result = inject(current, Context, true);
            if(result.compare("")){
                cout<<"-Success (1/2)"<<endl;
                writeDown(result, i-1);
            } else
                cerr << "-Failed (1/2)"<<endl;
        } else {
            i++;
            result = inject(current, Context, false);
            if(result.compare("")){
                cout<<"-Success (2/2)"<<endl;
                writeDown(result, i-1);
            } else
                cerr << "-Failed (2/2)"<<endl;
        }
    }
}
*/
/*
std::string MLACInjector::inject(StmtBinding current, ASTContext &Context){
    return "";
}
*/
MLACInjector::MLACInjector(){
    //Matcher.addMatcher(binaryOperator(anyOf(hasAncestor(expr(anyOf(hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt())))),hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt()))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    Matcher.addMatcher(
        stmt(
            switchStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("switch"), createStmtHandler("switch")
    );
    Matcher.addMatcher(
        stmt(
            doStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("do"), createStmtHandler("do")
    );
    Matcher.addMatcher(
        stmt(
            whileStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("while"), createStmtHandler("while")
    );
    Matcher.addMatcher(
        stmt(
            forStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("for"), createStmtHandler("for")
    );
    Matcher.addMatcher(
        stmt(
            ifStmt(hasCondition(anyOf(binaryOperator(), hasDescendant(binaryOperator()))))
        ).bind("if"), createStmtHandler("if")
    );
}

std::string MLACInjector::toString(){
    return "MLAC";
};
std::string MLACInjector::inject(StmtBinding current, ASTContext &Context){
    bool left = current.left;
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceLocation start, end;
    if(left){
        start = ((const BinaryOperator *)current.stmt)->getLHS()->getLocStart();
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocStart().getLocWithOffset(-1);
    }else {
        start = ((const BinaryOperator *)current.stmt)->getOperatorLoc();
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();
    }


    SourceRange range(start, end);
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MLACInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    std::vector<const BinaryOperator*> binaryOperators;
    if(binding.compare("if")==0){
        const Expr* condition = ((const IfStmt*)stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    } else if(binding.compare("do")==0){
        const Stmt* condition = ((const DoStmt*)stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    }  else if(binding.compare("switch")==0){
        const Stmt* condition = ((const SwitchStmt*)stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    }  else if(binding.compare("while")==0){
        const Stmt* condition = ((const WhileStmt*)stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    }  else if(binding.compare("for")==0){
        const Stmt* condition = ((const ForStmt*)stmt)->getCond();
        binaryOperators = getChildrenOfType<BinaryOperator>(condition);
    } else 
        return false;
    for(const BinaryOperator* op : binaryOperators){
        if(op->getOpcode()==BinaryOperatorKind::BO_LAnd){
            const Expr* left = op->getLHS()->IgnoreImplicit();
            const Expr* right = op->getRHS()->IgnoreImplicit();
            if(!isa<BinaryOperator>(left) || ((const BinaryOperator*)left)->getOpcode()!=BinaryOperatorKind::BO_LAnd)
                nodeCallback("MLOC",op, true);
            if(!isa<BinaryOperator>(right) || ((const BinaryOperator*)right)->getOpcode()!=BinaryOperatorKind::BO_LAnd)
                nodeCallback("MLOC",op, false);
        }
    }
    return false;
}