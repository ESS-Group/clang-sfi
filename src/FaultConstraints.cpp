
bool C9(const clang::Stmt::const_child_iterator &begin, const clang::Stmt::const_child_iterator &end){
    StmtIterator it = cast_away_const(begin);
    int num = 0;
    while(it!=cast_away_const(end)){
        if(isa<ForStmt>(*it) || isa<WhileStmt>(*it) || isa<DoStmt>(*it))//schleife
            return false;
        num++;
        it++;
    }
    if(num<=5)
        return true;
    else
        return false;
}
bool C9(const Stmt *stmt){
    return C9(stmt->child_begin(), stmt->child_end());   
}
bool C8(const IfStmt *ifS){
    if(const Stmt* Else = ifS->getElse())
        return false;
    else
        return true;
}