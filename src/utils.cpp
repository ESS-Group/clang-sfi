template <class T>
void deleteFromList(std::vector<T>& src, std::vector<T>& toDelete){
    bool deleted = false;
    for(std::vector<const BinaryOperator*>::iterator i = src.begin();i!=src.end();deleted?i:i++){
        deleted = false;
        for(T c:toDelete){
            if(*i==c){
                i = src.erase(i);
                deleted = true;
                break;
            }
        }
    }
}
const CompoundStmt* getParentCompoundStmt(const Stmt *stmt, ASTContext &Context){
    ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
    if(!list.empty()){
        if(list[0].get<Stmt>()!=NULL){
            if(isa<CompoundStmt>(list[0].get<Stmt>())){
                const CompoundStmt* container = list[0].get<CompoundStmt>();
                return container;
            } else return NULL;
        }
    } 
    return NULL;
}
const CompoundStmt* getParentCompoundStmt(const Decl *decl, ASTContext &Context){
    ASTContext::DynTypedNodeList list = Context.getParents(*decl);

    if(!list.empty()){
        if(list[0].get<Stmt>() != NULL){
            if(isa<CompoundStmt>(list[0].get<Stmt>())){
                const CompoundStmt* container = list[0].get<CompoundStmt>();
                return container;
            } else if(isa<DeclStmt>(list[0].get<Stmt>())){
                return getParentCompoundStmt(list[0].get<Stmt>(), Context);
            } else return NULL;
        }
    } 
    return NULL;
}
template<class T>
void concatVector(std::vector<T> &dst, std::vector<T> &src){
    dst.insert(dst.end(), src.begin(), src.end());
}
bool isAssignment(const BinaryOperator* op){
    return /*isa<DeclRefExpr>(op->getLHS()) && */op->getOpcode()==BinaryOperatorKind::BO_Assign/*20*/;
}

bool isValueAssignment(const BinaryOperator* op){
    if(isAssignment(op)){
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

std::vector<const BinaryOperator*> getChildForFindInitForVar(const Stmt *parent, const VarDecl* var, bool alsoinloop = false, bool alsoinforconstruct = true){
    std::vector<const BinaryOperator*> ret;
    if(parent==NULL)
        return ret;
    if(var==NULL)
        return ret;
    if(isa<BinaryOperator>(parent)){
        if(isAssignment((const BinaryOperator*)parent)){
            if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(parent)){
                if(exp->getDecl() == var){
                    ret.push_back((const BinaryOperator*) parent);
                }
            }
        }
    } else {
        for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
            if(*i == NULL){

            }else if(isa<BinaryOperator>(*i)){
                if(isAssignment((const BinaryOperator*)*i)){
                    if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(*i)){
                        if(exp->getDecl() == var){
                            ret.push_back((const BinaryOperator*) *i);
                            break;
                        }
                    }
                }
            }else if(isa<Stmt>(*i) && *i != NULL){
                if(isa<IfStmt>(*i)){

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
                    if(isa<ForStmt>(*i)){
                        std::vector <const BinaryOperator*> temp;
                        if(alsoinforconstruct){
                            temp = getChildForFindInitForVar(((const ForStmt*)*i)->getInit(), var, alsoinloop, alsoinforconstruct);
                            if(temp.size()!=0){
                                concatVector<const BinaryOperator*>(ret,temp);
                                break;
                            }
                        }
                    }

                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(*i, var), inloop;
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        break;
                    }
                    
                } else if(isa<ForStmt>(*i)){
                        std::vector <const BinaryOperator*> temp;
                    if(alsoinforconstruct){
                        temp = getChildForFindInitForVar(((const ForStmt*)*i)->getInit(), var, alsoinloop, alsoinforconstruct);
                        if(temp.size()!=0){
                            concatVector<const BinaryOperator*>(ret,temp);
                            break;
                        }
                    }
                }
            }
        }
    }
    return ret;
}







std::vector<const BinaryOperator*> getChildForFindVarAssignment(const Stmt *parent, const VarDecl* var, bool alsoinloop = true, bool alsoinforconstruct = true, bool pinited=false){
    
    std::vector<const BinaryOperator*> ret;
    if(var == NULL)
        return ret;
    if(parent == NULL)
        return ret;
    bool inited = pinited || var->hasInit();
    if(isa<BinaryOperator>(parent)){
        if(isAssignment((const BinaryOperator*)parent)){
            if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(parent)){
                if(exp->getDecl() == var){
                    if(inited)
                        ret.push_back((const BinaryOperator*) parent);
                    else
                        inited = true;
                }
            }
        }
    } else {
        for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
            if(*i == NULL){

            }else if(isa<BinaryOperator>(*i)){
                if(isAssignment((const BinaryOperator*)*i)){
                    if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(*i)){
                        if(exp->getDecl() == var){

                            if(inited)
                                ret.push_back((const BinaryOperator*) *i);
                            else
                                inited = true;
                        }
                    }

                }
            }else if(isa<Stmt>(*i) && *i != NULL){
                if(isa<IfStmt>(*i)){

                    IfStmt *ifS = (IfStmt *)*i;

                    std::vector<const BinaryOperator*> inThen = getChildForFindVarAssignment(ifS->getThen(), var,alsoinloop, alsoinforconstruct, inited);
                    if(inThen.size()!=0){
                        concatVector<const BinaryOperator*>(ret,inThen);
                        inited = true;
                    }
                    if(const Stmt* elseS = ifS->getElse()){
                        std::vector<const BinaryOperator*> inElse = getChildForFindVarAssignment(elseS, var, alsoinloop, alsoinforconstruct, inited);
                        if(inElse.size()!=0){
                            concatVector<const BinaryOperator*>(ret,inElse);
                            inited = true;
                        }
                    }
                }else if(alsoinloop){
                    if(isa<ForStmt>(*i)){
                        std::vector<const BinaryOperator*> temp;
                        if(alsoinforconstruct){
                            temp = getChildForFindVarAssignment(((const ForStmt*)*i)->getInit(), var, alsoinloop, alsoinforconstruct, inited);
                            if(temp.size()!=0){
                                concatVector<const BinaryOperator*>(ret,temp);
                                inited = true;
                            }
                            temp = getChildForFindVarAssignment(((const ForStmt*)*i)->getInc(), var, alsoinloop, alsoinforconstruct, inited);
                            if(temp.size()!=0){
                                concatVector<const BinaryOperator*>(ret,temp);
                                inited = true;
                            }
                        }
                        temp = getChildForFindVarAssignment(((const ForStmt*)*i)->getBody(), var, alsoinloop, alsoinforconstruct, true);
                            
                        if(temp.size()!=0){
                            concatVector<const BinaryOperator*>(ret,temp);
                            inited = true;
                        }
                    }else if(isa<WhileStmt>(*i)){
                        std::vector<const BinaryOperator*> temp = getChildForFindVarAssignment(((const WhileStmt*)*i)->getBody(), var, alsoinloop, alsoinforconstruct, true);
                        if(temp.size()!=0){
                            concatVector<const BinaryOperator*>(ret,temp);
                            inited = true;
                        }
                    } else if(isa<DoStmt>(*i)){
                        std::vector<const BinaryOperator*> temp = getChildForFindVarAssignment(((const DoStmt*)*i)->getBody(), var, alsoinloop, alsoinforconstruct, true);
                        if(temp.size()!=0){
                            concatVector<const BinaryOperator*>(ret,temp);
                            inited = true;
                        }
                    } else {
                        std::vector<const BinaryOperator*> temp = getChildForFindVarAssignment(*i, var, alsoinloop, alsoinforconstruct,inited);
                        if(temp.size()!=0){
                            concatVector<const BinaryOperator*>(ret,temp);
                            inited = true;
                        }
                    }
                }else if((!isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i))){
                    std::vector<const BinaryOperator*> temp = getChildForFindVarAssignment(*i, var, alsoinloop, alsoinforconstruct,inited);
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        inited = true;
                    }
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
const T* getParentOfType(const Stmt* stmt, ASTContext &Context, int maxDepth = 3){//MaxDepth = -1 for to the root
    T* ret = NULL;
    if(stmt==NULL)
        return NULL;
    if(maxDepth!=0){
        ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
        for(auto p : list){
            if(isa<T>(p.get<Stmt>())){
                return p.get<T>();
            }else if(ret == NULL){
                return getParentOfType<T>(p.get<T>(), Context, maxDepth-1);
            }
        }
    }
    if(ret == NULL)
        return NULL;
    else
        return const_cast<const T*>(ret);
}


template<class T>
const T* getParentOfType(const Decl* decl, ASTContext &Context, int maxDepth = 3){//MaxDepth = -1 for to the root
    T* ret = NULL;
    if(maxDepth!=0){
        ASTContext::DynTypedNodeList list = Context.getParents(*decl);
        for(auto p : list){
            if(p.get<Stmt>() == NULL){
            }else if(isa<T>(p.get<Stmt>())){
                return p.get<T>();
            }else if(ret == NULL){
                return getParentOfType<T>(p.get<Stmt>(), Context, maxDepth-1);
            }
        }
    }
    
    if(ret == NULL){
        return NULL;
    }else{
        return const_cast<const T*>(ret);
    }
}


template<class T>
bool hasParentOfType(const Stmt* stmt, ASTContext &Context){
    return getParentOfType<T>(stmt, Context, -1)!=NULL;
}
template<class T>
bool hasParentOfType(const Decl* decl, ASTContext &Context){
    return getParentOfType<T>(decl, Context, -1)!=NULL;
}

const FunctionDecl* getParentFunctionDecl(const Stmt* stmt, ASTContext &Context){
    if(stmt == NULL)
        return NULL;
    const DeclStmt* ret = getParentOfType<DeclStmt>(stmt,Context, -1);
    if(ret == NULL)
        return NULL;
    else if(ret->isSingleDecl()){
        if(isa<FunctionDecl>(ret->getSingleDecl()))
            return (const FunctionDecl*) ret->getSingleDecl();
        else
            return getParentFunctionDecl(ret, Context);
    } else return NULL;
}
bool isPartOfFunction(const Stmt* stmt, ASTContext &Context){
    if(stmt == NULL)
        return false;
    const FunctionDecl* decl = getParentFunctionDecl(stmt, Context);
    return decl != NULL;
    /*const DeclStmt* ret = getParentOfType<DeclStmt>(stmt,Context, -1);
    if(ret == NULL)
        return false;
    else if(ret->isSingleDecl()){
        if(isa<FunctionDecl>(ret->getSingleDecl()))
            return true;
        else
            return isPartOfFunction(ret, Context);
    } else return false;*/
}
bool isLocal(const Stmt* stmt, ASTContext &Context){
    if(stmt == NULL)
        return false;
    else
        return isPartOfFunction(stmt, Context);
}
bool isLocal(const Decl* decl, ASTContext &Context){
    if(decl == NULL)
        return false;
    return isLocal(getParentOfType<DeclStmt>(decl, Context), Context);    
}
template<class T>
bool hasChildOfType(const Stmt* stmt){
    if(stmt==NULL)
        return false;
        for(Stmt::child_iterator i = cast_away_const(stmt->child_begin()), e = cast_away_const(stmt->child_end());i!=e;++i){
            if(isa<T>(*i))
                return true;
            else if(hasChildOfType<T>(*i))
                return true;
        }
        return false;
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
    return isParentOf(parent,stmt);
}

bool isInitializedBefore(const DeclRefExpr* ref, ASTContext &Context){
    const VarDecl* decl =  (const VarDecl*) ref->getDecl();
    if( decl->getInit() != NULL)//if declaration is initialization => every use after that is an assignment
        return true;
    else{
        const CompoundStmt* parent = getParentCompoundStmt(decl, Context);
        std::vector<const BinaryOperator*> inits = getChildForFindInitForVar(parent, decl, false);
        for(const BinaryOperator* init : inits){//else check if ref is used in initialization
            if(init->getLHS() == ref)
                return false;
        }
        return true;
    }

}


template<class T>
std::vector<const T*> getChildrenOfType(const Stmt *parent, bool first = true){
    std::vector<const BinaryOperator*> ret;
    if(parent==NULL)
        return ret;
    if(isa<T>(parent)&&first){
        ret.push_back((const T*)parent);
    }
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(isa<T>(*i)){
                ret.push_back((const T*)*i);
        }
        std::vector<const T*> children = getChildrenOfType<T>(*i, false);
        if(children.size()!=0){
            concatVector<const T*>(ret,children);
        }
    }
    return ret;
}