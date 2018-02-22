MIEBInjector::MIEBInjector(){//Missing if construct plus statements
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
}

std::string MIEBInjector::toString(){
    return "MIEB";
};
std::string MIEBInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    /*
    const auto& parents = Context.getParents(*ifS);
    //IfStmt &ifStmt = *ifS;
    const clang::Stmt::const_child_iterator &it = ifS->getThen()->child_begin();
    StmtIterator st = cast_away_const(it);
    //auto begin = _range.begin();
    auto i = *it;
    int z = 0;
    while(st!=cast_away_const(ifS->getThen()->child_end())){
        if(!isa<NullStmt>(*st)){
            if(isa<CompoundStmt>(*st))
                cout<<"-----------------------compound"<<endl;
        z++;
        st->dump(Context.getSourceManager());
        cout<<"---------------------------childend"<<endl;
        st++;
        }
    }
    cout << z << "children"<<endl;
    */
    /*cout<<"#######################"<<endl;
    if(!parents.empty()){
        cout<<"----------------------------"<<endl;
        for(auto p : parents){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }
    } else
        cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    cout<<"#######################"<<endl;*/
    //ifS->IgnoreImplicit()->dump(Context.getSourceManager());
    
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocStart().getLocWithOffset(-1));
    //SourceRange sr(IfS->getLocStart(), Else->getLocStart().getLocWithOffset(-1));//-1 Offset da sonst das erste Zeichen des Else blockes mit gelöscht wird
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIEBInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    //cout<<"check Node"<<endl;
    
    //if(
        const IfStmt* ifS = (IfStmt *)(stmt);
    //){
        if(!C9(ifS->getThen()))
            return false;
        return !C8(ifS);
        /*if(const Stmt* Else = ifS->getElse())
            return true;
        else
            return false;*/
    //} else return false;
}