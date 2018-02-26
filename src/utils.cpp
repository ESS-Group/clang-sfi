const CompoundStmt* getParentCompoundStmt(const Stmt *stmt, ASTContext &Context){
    ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
    //cout << list.size() << " Parents";
    /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    if(!list.empty()){
        if(list[0].get<Stmt>()!=NULL){
            if(isa<CompoundStmt>(list[0].get<Stmt>())){
                const CompoundStmt* container = list[0].get<CompoundStmt>();
                return container;
            } else return NULL;
        }
        /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    } //else
        //return false;
        //cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    //cout<<"#######################"<<endl;
    return NULL;
}
const CompoundStmt* getParentCompoundStmt(const Decl *decl, ASTContext &Context){
    ASTContext::DynTypedNodeList list = Context.getParents(*decl);
    //cout << list.size() << " Parents";
    //cout<<"----------------------------"<<endl;
      //  for(auto p : list){
        //    p.get<Stmt>()->dump(Context.getSourceManager());
        //}
    if(!list.empty()){
        if(list[0].get<Stmt>() != NULL){
            if(isa<CompoundStmt>(list[0].get<Stmt>())){
                const CompoundStmt* container = list[0].get<CompoundStmt>();
                return container;
            } else if(isa<DeclStmt>(list[0].get<Stmt>())){
                return getParentCompoundStmt(list[0].get<Stmt>(), Context);
            } else return NULL;
        }
        /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    } //else
        //return false;
        //cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    //cout<<"#######################"<<endl;
    return NULL;
}
template<class T>
void concatVector(std::vector<T> &dst, std::vector<T> &src){
    dst.insert(dst.end(), src.begin(), src.end());
}
bool isAssignment(const BinaryOperator* op){
    return op->getOpcode()==20;
}
//isValueDeclaration
bool isValueAssignment(const BinaryOperator* op){
    if(isAssignment(op)){
        //Stmt::child_iterator i = cast_away_const(op->child_begin());
        const Stmt *stmt = op->getRHS();
        if(stmt!=NULL && (isa<IntegerLiteral>(stmt) || isa<CXXBoolLiteralExpr>(stmt) || isa<CharacterLiteral>(stmt) || isa<FloatingLiteral>(stmt) || isa<clang::StringLiteral>(stmt) ))
            return true;
        else
            return false;
    }else
        return false;
}

bool isExprAssignment(const BinaryOperator* op){
    return isAssignment(op) && !isValueAssignment(op);
}

template<class T>
const T* getFirstChild(const Stmt *parent){
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i != NULL){
            if(isa<T>(*i)){
                return (const T *) *i;
            } else {
                if(const T* ret = getFirstChild<T>(*i))
                    return ret; 
            }
        }
    }
    return NULL;
}
/*
std::string stmtToString(const Decl* decl, const LangOptions &langOpts){
    std::string statement;
    raw_string_ostream stream(statement);
    decl->print(stream, PrintingPolicy(langOpts));
    stream.flush();
    return statement;
}*/

std::vector<const BinaryOperator*> getChildForFindInitForVar(const Stmt *parent, const VarDecl* var, bool alsoinloop = false){
    std::vector<const BinaryOperator*> ret;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == NULL){

            //cout << "getChildForFindInitForVar >> Found NULL" << endl;
        }else if(isa<BinaryOperator>(*i)){
            ////cout << "getChildForFindInitForVar >> Found BinaryOperator" << endl;
            if(isAssignment((const BinaryOperator*)*i)){
                ////cout << "getChildForFindInitForVar >> Found BinaryOperator >> isValueDeclaration" << endl;
                //i->dumpColor();
                if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(*i)){
                    ////exp->dumpColor();
                    ////var->dumpColor();
                    //cout << var->getName().data()<<endl;
                    if(exp->getDecl() == var){
                        ret.push_back((const BinaryOperator*) *i);
                        break;
                    }
                }
            }
        }else if(isa<Stmt>(*i) && *i != NULL){
            if(isa<IfStmt>(*i)){

                //cout << "getChildForFindInitForVar >> Found IfStmt" << endl;
                IfStmt *ifS = (IfStmt *)*i;

                std::vector<const BinaryOperator*> inThen = getChildForFindInitForVar(ifS->getThen(), var);
                if(inThen.size()!=0){
                    concatVector<const BinaryOperator*>(ret,inThen);
                }
                if(const Stmt* elseS = ifS->getElse()){
                    std::vector<const BinaryOperator*> inElse = getChildForFindInitForVar(elseS, var);
                    if(inElse.size()!=0){
                        concatVector<const BinaryOperator*>(ret,inElse);
                        if(inThen.size()!=0)
                            break;//initialization in both
                    }
                }
            }else if(alsoinloop || (!isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i))){

                //cout << "getChildForFindInitForVar >> Found Other" << endl;
                std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(*i, var), inloop;
                if(temp.size()!=0){
                    concatVector<const BinaryOperator*>(ret,temp);
                    break;
                }
                
            }
        }
    }
    return ret;
}







std::vector<const BinaryOperator*> getChildForFindVarAssignment(const Stmt *parent, const VarDecl* var, bool alsoinloop = true){
    std::vector<const BinaryOperator*> ret;
    if(parent == NULL)
        return ret;
    //parent->dumpColor();
    //cout<<"huhu"<<endl;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == NULL){

            //cout << "getChildForFindInitForVar >> Found NULL" << endl;
        }else if(isa<BinaryOperator>(*i)){
            //cout << "getChildForFindInitForVar >> Found BinaryOperator" << endl;
            if(isAssignment((const BinaryOperator*)*i)){
                //cout << "getChildForFindInitForVar >> Found BinaryOperator >> isValueDeclaration" << endl;
                //i->dumpColor();
                //cout << "test1"<<endl;
                if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(*i)){
                    //exp->dumpColor();
                    //var->dumpColor();
                    //cout << var->getName().data()<<endl;

                    //cout << "test2"<<endl;
                    if(exp->getDecl() == var){

                        //cout << "test3"<<endl;
                        ret.push_back((const BinaryOperator*) *i);
                        //break;
                    }
                }

                //cout << "test4"<<endl;
            }
        }else if(isa<Stmt>(*i) && *i != NULL){
            if(isa<IfStmt>(*i)){

                //cout << "getChildForFindInitForVar >> Found IfStmt" << endl;
                IfStmt *ifS = (IfStmt *)*i;

                std::vector<const BinaryOperator*> inThen = getChildForFindInitForVar(ifS->getThen(), var,alsoinloop);
                if(inThen.size()!=0){
                    concatVector<const BinaryOperator*>(ret,inThen);
                }
                if(const Stmt* elseS = ifS->getElse()){
                    std::vector<const BinaryOperator*> inElse = getChildForFindInitForVar(elseS, var, alsoinloop);
                    if(inElse.size()!=0){
                        concatVector<const BinaryOperator*>(ret,inElse);
                        //if(inThen.size()!=0)
                            //break;//initialization in both
                    }
                }
            }else if(alsoinloop){
                if(isa<ForStmt>(*i)){

                    //cout <<"Search in for"<<endl;    
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(((const ForStmt*)*i)->getBody(), var, alsoinloop);
                    //cout <<temp.size()<<endl;
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        /*for(auto a : temp){
                            a->dumpColor();
                        }*/
                        //break;
                    }
                }else if(isa<WhileStmt>(*i)){
                    //cout <<"Search in while"<<endl;
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(((const WhileStmt*)*i)->getBody(), var, alsoinloop);
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        //break;
                    }
                } else if(isa<DoStmt>(*i)){
                    //cout <<"Search in do"<<endl;
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(((const DoStmt*)*i)->getBody(), var, alsoinloop);
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        //break;
                    }
                } else {
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(*i, var, alsoinloop);
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        //break;
                    }
                }
                //cout << "getChildForFindInitForVar >> Found Other" << endl;
                
            }else if((!isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i))){
                std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(*i, var, alsoinloop);
                if(temp.size()!=0){
                    concatVector<const BinaryOperator*>(ret,temp);
                    //break;
                }
            }
        }
    }
    return ret;
}
/*
std::vector<const BinaryOperation*> findInitForVar(const Stmt* parent, const VarDecl* var, bool alsoinloop=false){
    std::vector<const BinaryOperator*> ret;
    if(alsoinloop || (!isa<ForStmt>(parent) && !isa<WhileStmt>(parent) && !isa<DoStmt>(parent))){
        for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
            Stmt *stmt = *i;
            if(stmt != NULL){
                if(isa<BinaryOperator>(stmt)){
                    if(((const BinaryOperation *)stmt)->getOpcode()==20&&(const DeclRefExpr* declE =getChildForFindInitForVar(stmt, var, alsoinloop)));
                        ret.push_back(stmt);
                        break;
                    }
                }
            }
        }
    }
    return ret;
}
*/

template<class T>
const T* getParentOfType(const Stmt* stmt, ASTContext &Context, int maxDepth = 1){//MaxDepth = -1 for to the root
    T* ret;

    //cout<<"+line1"<<endl;
    if(stmt==NULL)
        return NULL;
    if(maxDepth!=0){
        //cout<<"+line2"<<endl;
        ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
        //cout<<"+line3"<<endl;
        for(auto p : list){
            //cout<<"+line4"<<endl;
            if(isa<T>(p.get<Stmt>())){
                //cout<<"+line5.1.1"<<endl;
                return p.get<T>();
                //cout<<"+line5.1.2"<<endl;
            }else if(ret == NULL){
                //cout<<"+line5.2.1"<<endl;
                ret = (T*)(&(*getParentOfType<T>(p.get<T>(), Context, maxDepth-1)));
                //cout<<"+line5.2.2"<<endl;
            }
        }
    }
    //cout<<"+line6"<<endl;
    if(ret == NULL)
        return NULL;
    else
        return const_cast<const T*>(ret);
}

template<class T>
bool hasChildOfType(const Stmt* stmt){
    if(stmt==NULL)
        return false;
    //if(maxDepth!=0){
        for(Stmt::child_iterator i = cast_away_const(stmt->child_begin()), e = cast_away_const(stmt->child_end());i!=e;++i){
            if(isa<T>(*i))
                return true;
            else if(hasChildOfType<T>(*i))
                return true;
        }
        return false;
    //}
    //cout<<"+line6"<<endl;
    /*if(ret == NULL)
        return NULL;
    else
        return const_cast<const T*>(ret);*/
}
/*
template<class T>
std::vector<const T*> getInnerstChildOfType(const Stmt* stmt, ASTContext &Context, int maxDepth = 1){//MaxDepth = -1 for to the root
    //T* ret;
    std::vector<const T*> ret;
    //cout<<"+line1"<<endl;
    if(stmt==NULL)
        return NULL;
    if(maxDepth!=0){
        //cout<<"+line2"<<endl;
        ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
        //cout<<"+line3"<<endl;
        for(auto p : list){
            //cout<<"+line4"<<endl;
            if(isa<T>(p.get<Stmt>())){
                //cout<<"+line5.1.1"<<endl;
                return p.get<T>();
                //cout<<"+line5.1.2"<<endl;
            }else if(ret == NULL){
                //cout<<"+line5.2.1"<<endl;
                ret = (T*)(&(*getParentOfType<T>(p.get<T>(), Context, maxDepth-1)));
                //cout<<"+line5.2.2"<<endl;
            }
        }
    }
    //cout<<"+line6"<<endl;
    if(ret == NULL)
        return NULL;
    else
        return const_cast<const T*>(ret);
}
*/
//getParentOfType<ForStmt>(decl,Context,3)
template<class T>
const T* getParentOfType(const Decl* decl, ASTContext &Context, int maxDepth = 1){//MaxDepth = -1 for to the root
    T* ret = NULL;
    //cout<<"-line1"<<endl;
    if(maxDepth!=0){
        ASTContext::DynTypedNodeList list = Context.getParents(*decl);

        //cout<<"-line2"<<endl;
        for(auto p : list){
            //cout<<"-line3"<<endl;
            if(isa<T>(p.get<Stmt>())){

            //cout<<"-line3.1.1"<<endl;
                return p.get<T>();
            //cout<<"-line3.1.2"<<endl;
            }else if(ret == NULL){
                //cout<<"-line3.2.1"<<endl;
                ret = (T*)(&(*getParentOfType<T>(p.get<T>(), Context, maxDepth-1)));
                //cout<<"-line3.2.2"<<endl;
            }
            //cout<<"-line4"<<endl;
        }
    }
    //cout <<"-line5"<<endl;
    
    if(ret == NULL){
        //cout <<"-line6.1"<<endl;
        return NULL;
    }else{
        //cout <<"-line6.2"<<endl;
        return const_cast<const T*>(ret);
    }
}




bool isParentOf(const Stmt* parent, const Stmt* stmt){
    if(parent == NULL)
        return false;
        
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == stmt)
            return true;
        if(isParentOf(*i, stmt))
            return true;
    }
    return false;
}
bool isParentOf(const Stmt* parent, const Decl* decl, ASTContext &Context){
    if(parent == NULL)
        return false;
        
    const DeclStmt* stmt = getParentOfType<DeclStmt>(decl,Context,3);
    if(stmt==NULL)
        return false;
    else
        return isParentOf(parent,getParentOfType<DeclStmt>(decl,Context,3));
    //bool ret = false;
    /*
    ASTContext::DynTypedNodeList list = Context.getParents(decl);
    if(list.empty() || list[0].get<Stmt>() == NULL)
        return false;
    return isParentOf(parent,(const Stmt *)list[0].get<Stmt>());
    */
}