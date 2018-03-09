#include <algorithm>
bool ___verbose = false;






std::vector<const DeclRefExpr*> getAllRefs(const Stmt *parent, const VarDecl* var){
    //if(___verbose)cout<<"getAllRefs"<<endl;
    //parent->dumpColor();
    //var->dumpColor();
    std::vector<const DeclRefExpr*> ret;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == NULL){
        }else if(isa<Stmt>(*i)){
            //if(___verbose)cout<<"stmt"<<endl;
            if(isa<DeclRefExpr>(*i)){
                //i->dumpColor();
                ret.push_back((const DeclRefExpr*) *i);
            } else {
                std::vector<const DeclRefExpr*> list = getAllRefs(*i, var);
                if(list.size()!=0)
                    concatVector<const DeclRefExpr*>(ret,list);
            }
        }
    }
    //if(___verbose)cout<<"getAllRefs - ende"<<endl;
    return ret;
}

const DeclRefExpr* getLatestRef(const Stmt *parent, const VarDecl* var){
    std::vector<const DeclRefExpr*> refs = getAllRefs(parent, var);
    //var->dumpColor();
    //parent->dumpColor();
    //if(___verbose)cout <<"ALLREFS"<<refs.size()<<endl;
    const DeclRefExpr* ret = NULL;
    for(const DeclRefExpr* ref : refs){
        if(ret == NULL)
            ret = ref;
        else if(ret->getLocEnd()<ref->getLocEnd())
            ret = ref;
    }
    return ret;
}




template<class T>
bool hasStmtOfType(std::vector<const Stmt *> list){
    for(const Stmt* stmt:list){
        if(isa<T>(stmt)){
            return true;
        }
    }
    return false;
}
template<class T>
std::vector<const T*> getStmtsOfType(std::vector<const Stmt *> &list){
    if(___verbose)cout<<"huhu"<<endl;
    std::vector<const T*> ret;
    if(list.empty())
        return ret;
    for(const Stmt* stmt:list){
        if(stmt!=NULL && isa<T>(stmt)){
            ret.push_back((const T*)stmt);
            if(___verbose)stmt->dumpColor();
        }
    }
    if(___verbose)cout<<"-huhu"<<endl;
    return ret;
}




/*
void FaultInjector::_sort(){
    std::sort(
        locations.begin(),
        locations.end(),
        comparefunc
    );
}

*/
template <class T>
bool _comparefunc(const T* st1, const T* st2){
    return st1->getLocStart()<st2->getLocStart();//l2<l1;
}





std::vector<std::vector<const Stmt*>> getStmtLists(const CompoundStmt * block){
    std::vector<std::vector<const Stmt*>> ret;
    int index = -1;
    for(Stmt::child_iterator i = cast_away_const(block->child_begin()), e = cast_away_const(block->child_end());i!=e;++i){
        if(*i == NULL){

            //if(___verbose)cout << "getChildForFindInitForVar >> Found NULL" << endl;
            if(___verbose)cout << "NULLSTMT in block"<<endl;
        }else if(isa<Stmt>(*i)){
            if(index==-1){
                if(___verbose)cout<<"begin"<<endl;
                std::vector<const Stmt*> list;
                ret.push_back(list);
                index = 0;
                if(___verbose)cout<<"end"<<endl;
            }
            if(!isa<IfStmt>(*i) && !isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i)){
                if(isa<CompoundStmt>(*i)){
                    if(ret[index].size()>0){
                        std::vector<const Stmt*> list;
                        ret.push_back(list);
                        index++;
                    }
                    if(___verbose)cout << "CompoundSTMT in block"<<endl;
                    //TODO: maybe include the statements inside the CompoundStmt
                } else {
                    if(___verbose)cout << "begin STMT in block"<<endl;
                    ret.at(index).push_back(*i);
                    if(___verbose)cout << "end STMT in block"<<endl;
                }
            } else if(ret[index].size()>0){
                if(___verbose)cout << "begin IF or Loop in block"<<endl;
                std::vector<const Stmt*> list;
                ret.push_back(list);
                index++;
                if(___verbose)cout << "end IF or Loop in block"<<endl;
                
            }
        }
    }
    if(___verbose)cout <<"1"<<endl;
    if(___verbose)cout<<"hell"<<" "<<"jeah"<<" "<<"1"<<endl;
    if(index <= 0 && (ret.size()==0 || ret[0].size()==0)){
        std::vector<std::vector<const Stmt*>> ret;
        return ret;
    }

    if(___verbose)cout<<"hell"<<" "<<"jeah"<<" "<<"2"<<endl;
    if(___verbose)cout<<"blocks"<<ret[0].size()<<endl;
    if(___verbose)cout <<"2"<<endl;
    //for(auto list:ret){
    std::vector<std::vector<const Stmt*>>::reverse_iterator rit = ret.rbegin();
    if(___verbose)cout<<"hell"<<" "<<"jeah"<<" "<<"3"<<endl;
    if(___verbose)cout <<"3"<<endl;
    //std::vector<const Stmt*>::iterator toDeleteIt;
    //std::vector<const Stmt*>::iterator* toDelete = NULL;
    bool deleteIt = false;
    if(___verbose)cout<<"hell"<<" "<<"jeah"<<" "<<"4"<<endl;
    for(;rit!=ret.rend();/*++rit*/){

        if(___verbose)cout <<"3.1"<<endl;
        //if(toDelete != NULL){
            /*
        if(deleteIt){
            //int pos = std::distance(ret.begin(),rit)+1;
            int rpos=std::distance(ret.rbegin(), rit);
            int pos = ret.size()-rpos+1;
            
            if(___verbose)cout<<pos<<"POSSS"<<ret.size()<<endl;
            std::erase(ret.begin()+pos);
            deleteIt = false;
            rit = ret.rbegin();
            for(int i = 0 ; i <rpos ; i++)
                rit++;
            //std::erase(toDelete);
            //toDelete = NULL;
        }*/

        if(___verbose)cout <<"3.2"<<endl;
        std::vector<const Stmt*> list = *rit;
        if(___verbose)cout<<"-----------------------------------RIT"<<ret.size()<<"begin"<<endl;
        if(___verbose)cout<<rit->size()<<endl<<list.size()<<endl;
        if(___verbose)
            for(auto item:list){
                item->dumpColor();
            };
        if(___verbose)cout <<"3.3"<<endl;
        std::vector<const DeclStmt*> declstmts = getStmtsOfType<DeclStmt>(list);

        if(___verbose)cout <<"3.4"<<endl;

        std::vector<const DeclStmt*> notPossible;

        for(const DeclStmt* declstmt:declstmts){
            if(___verbose)cout<<"declstmts"<<endl;
            if(___verbose)declstmt->dumpColor();
            if(___verbose)cout <<"3.4.1"<<endl;
            const DeclRefExpr* ref = NULL;
            for(auto decl:declstmt->decls()){
                if(___verbose)cout <<"3.4.1.1"<<endl;
                const DeclRefExpr* latest = getLatestRef(block, (const VarDecl*)decl);
                if(___verbose)if(latest!=NULL)latest->dumpColor();
                if(___verbose)cout <<"3.4.1.2"<<endl;
                if(ref==NULL)
                    ref = latest;
                else if(ref->getLocStart()<latest->getLocStart())
                    ref = latest;

                if(___verbose)cout <<"3.4.1.3"<<endl;
            }
            if(ref!=NULL){
                if(___verbose)cout << "REFFFF"<<endl;
                if(ref==NULL){
                    if(___verbose)cout<<"NULL"<<endl;
                }else{
                    if(___verbose)ref->dumpColor();
                }
                if(___verbose)cout <<"3.4.2"<<endl;
                //if(ref!=NULL)ref->getLocStart().dump();
                if(___verbose)cout <<list.size()<<endl;
                if(___verbose)list.back()->dumpColor();
                if(___verbose)ref->dumpColor();
                if(___verbose)declstmt->dumpColor();
                if(list.back()->getLocEnd()<ref->getLocStart())
                    if(___verbose)cout<<"IS AFTER"<<endl;
                if(std::find(list.begin(), list.end(), declstmt)!=list.end())
                    if(___verbose)cout<<"Found DECL"<<endl;
                if(ref!=NULL && list.back()->getLocEnd()<ref->getLocStart() && std::find(list.begin(), list.end(), declstmt)!=list.end()){
                    const DeclStmt* statement = (const DeclStmt*)declstmt;//declStmt;
                    notPossible.push_back(statement);
                }
                if(___verbose)cout<<"notPossiblesize: "<<notPossible.size()<<endl;
                //wenn letzte ref nicht mehr in list dann liste verkleinern sodass ref nicht mehr drin ist, evtl liste in 2 splitten, da ja vorher und nachher stmts drin sein können

                if(___verbose)cout <<"3.4.3"<<endl;
            }
        }

        if(___verbose)cout <<"3.5"<<endl;
        if(notPossible.size()!=0){
            deleteIt = true;
            std::vector<std::vector<const Stmt*>> changed;
            std::vector<const Stmt*> temp(list.begin(), list.end());
            changed.push_back(temp);
            std::sort(
                notPossible.begin(),
                notPossible.end(),
                _comparefunc<DeclStmt>
            );
            bool deletedFirst = false;
            for(const DeclStmt* stmt:notPossible){
                if(___verbose)cout<<"stmt"<<endl;
                if(___verbose)stmt->dumpColor();
                for(std::vector<std::vector<const Stmt*>>::reverse_iterator it=changed.rbegin();it!=changed.rend();++it){
                    if(___verbose)cout<<"pos"<<endl;
                    for(const Stmt* st:*it)
                        if(___verbose)st->dumpColor();
                    auto position = std::find(it->begin(), it->end(),stmt);//std::find((*it).rbegin(), (*it).rend(), stmt);
                    int pos = std::distance(it->begin(), position);//(*it).size() - std::distance((*it).rbegin(), position);
                    if(position!=it->end()){
                        if(___verbose)cout<<"pre"<<" dump"<<endl;
                        if(*position == NULL){
                            if(___verbose)cout <<"NULL"<<endl;
                        }else{
                            if(___verbose)(*position)->dumpColor();
                        }
                        if(___verbose)cout<<"post"<<" dump"<<endl;
                    }
                    if(position!=it->end()){
                        std::vector<const Stmt*> first(it->begin(), it->begin()+pos),//((*it).begin(), (*it).begin()+pos),
                                    second(it->begin()+pos+1, it->end());//((*it).begin()+pos+1, (*it).end());
                        if(first.size()!=0){
                            changed.push_back(first);
                            if(___verbose)cout<<"first"<<endl;
                            if(___verbose)
                                for(auto i:first)
                                    i->dumpColor();
                        }
                        if(second.size()!=0){
                            changed.push_back(second);
                            if(___verbose)cout<<"sekundu"<<endl;
                            if(___verbose)
                                for(auto i:second)
                                    i->dumpColor();
                        }
                        deleteIt = true;
                        //toDeleteIt = position;
                        //toDelete = &toDeleteIt;
                        break;

                        //UNBEDINGT IM NÄCHSTEN SCHRITT DER ÄUSSERSTEN SCHLEIFE EINTRAG LÖSCHEN
                    }
                    
                }
                if(changed.size()>0&&!deletedFirst){
                    changed = std::vector<std::vector<const Stmt*>>(changed.begin()+1, changed.end());
                    deletedFirst = true;
                }
            }

            if(___verbose)cout <<"3.5.1"<<endl;
            if(___verbose)cout<<"------------------------changed-size"<<changed.size()<<endl;
            if(___verbose)cout<<"--------------------1LISTE"<<list.size()<<endl;
            if(___verbose)cout<<ret.size()<<endl;
            int ritpos = std::distance(ret.rbegin(), rit)+changed.size();

            ret.insert(ret.end(), changed.begin(),changed.end());//invalidates iterator
            rit = ret.rbegin();
            for(int i = 0 ; i<ritpos;i++,rit++);
            list = *rit;
            if(___verbose)cout<<"--------------------1.1LISTE"<<list.size()<<endl;

            if(___verbose)cout<<ret.size()<<endl;

            if(___verbose)cout <<"3.5.2"<<endl;
        }
        if(___verbose)cout<<"--------------------2LISTE"<<list.size()<<endl;
        //get
        if(deleteIt){
            /*auto it = ret.erase(--rit.base());
            rit=std::vector<std::vector<const Stmt*>>::reverse_iterator(it);//ret.erase(rit.base());
            deleteIt = false;*/
            if(___verbose)cout<<"--------------------3LISTE"<<list.size()<<endl;
            if(___verbose)cout<<"-----------------------------------RIT"<<ret.size()<<"end - delete"<<endl;
            if(___verbose)cout<<rit->size()<<endl<<list.size()<<endl;
            if(___verbose)cout<<list.size()<<"toDelete"<<rit->size()<<endl;
            rit->clear();
            deleteIt = false; 
            if(___verbose)cout<<"deleted"<<rit->size()<<endl;
            ++rit;
        } else{
            if(___verbose)cout<<"-----------------------------------RIT"<<ret.size()<<"end"<<endl;
        if(___verbose)cout<<rit->size()<<endl<<list.size()<<endl;
            ++rit;
        }
    }
    if(___verbose)cout<<"hell"<<" "<<"jeah"<<" "<<"4"<<endl;
    if(___verbose)cout <<"4"<<endl;


    //TODO: delete all empty lists??
    //ret;
    if(___verbose)cout<<"hell"<<" "<<"jeah"<<" "<<"5"<<endl;

    return ret;

}



bool isMLPAListPossible(std::vector<const Stmt*> stmtlist, const CompoundStmt* block){
    std::vector<const DeclStmt*> declstmts = getStmtsOfType<DeclStmt>(stmtlist);

    std::vector<const DeclStmt*> notPossible;

    for(const DeclStmt* declstmt:declstmts){
        const DeclRefExpr* ref = NULL;
        for(auto decl:declstmt->decls()){
            const DeclRefExpr* latest = getLatestRef(block, (const VarDecl*)decl);
            if(ref==NULL)
                ref = latest;
            else if(ref->getLocStart()<latest->getLocStart())
                ref = latest;
        }
        if(ref!=NULL && stmtlist.back()->getLocEnd()<ref->getLocStart() && std::find(stmtlist.begin(), stmtlist.end(), declstmt)!=stmtlist.end()){
            const DeclStmt* statement = (const DeclStmt*)declstmt;//declStmt;
            notPossible.push_back(statement);
        }
    }
    return notPossible.size()==0;
}

std::vector<std::vector<const Stmt*>> getMLPAListOfSize(std::vector<const Stmt*> stmtlist, int size, const CompoundStmt* block){
    std::vector<std::vector<const Stmt*>> ret;
    int listsize=stmtlist.size();
    for(int begin=0;begin+size<=listsize;begin++){
        std::vector<const Stmt*> list(stmtlist.begin()+begin, stmtlist.begin()+begin+size);
        //cout<<"list:"<<list.size()<<endl;
        if(isMLPAListPossible(list, block))
            ret.push_back(list);
    }
    return ret;
}










MLPAInjector::MLPAInjector(){
    Matcher.addMatcher(
                compoundStmt(
                    unless(hasParent(declStmt()))
                ).bind("compoundStmt"), 
                createStmtHandler("compoundStmt")
        );

}

std::string MLPAInjector::toString(){
    return "MLPA";
};




std::string MLPAInjector::inject(StmtBinding current, ASTContext &Context){
    if(___verbose)cout<<"aljkgaeoigh"<<endl;
    if(___verbose)cout<<(current.isList?"list":"0")<<(current.isStmt?"stmt":"decl")<<endl;
    std::vector<const Stmt*> list = current.stmtlist;
    if(___verbose)cout<<"huhu1"<<endl;
    SourceLocation begin=list[0]->getLocStart(), end=list[0]->getLocEnd();
    if(___verbose)cout<<"huhu2"<<endl;
    for(const Stmt *stmt:list){
        if(stmt->getLocStart() < begin)
            begin = stmt->getLocStart();
        if(end < stmt->getLocEnd())
            end = stmt->getLocEnd();
    }
    
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(begin, end);
    R.RemoveText(range);
    return getEditedString(R, Context);
/*    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if(current.isStmt){
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        VarDecl temp (*((const VarDecl*)current.decl));
        temp.setInit(NULL);
        const VarDecl* tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());

        
        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
        R.ReplaceText(range, withoutInit);
    }

    return getEditedString(R, Context);*/
}


bool MLPAInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){
    if(___verbose)cout<<"check - 1"<<endl;
    const CompoundStmt * compoundStmt = (const CompoundStmt *)stmt;
    //stmt->dumpColor();
    if(___verbose)cout<<"check - 2"<<endl;
    std::vector<std::vector<const Stmt*>> stmtlists = getStmtLists(compoundStmt);
    if(___verbose)cout<<"check - 3"<<endl;

    for(std::vector<const Stmt*> it:stmtlists){
        if(___verbose)cout<<"1"<<endl;
        if(it.size()>=2){
            if(___verbose)cout<<"2"<<endl;
            int size = it.size();
            //TODO: if whole block then "-1" so that at least 1 statement will remain!!!!!
            if(verbose){
                cout<<"--- new List ---"<<size<<endl;
                /*for(const Stmt * stmt:it){
                            stmt->dumpColor();
                }*/
            }
            if(___verbose)cout<<"3"<<endl;
            if(size>10)
                size=10;
            //cout<<"fullsize:"<<size<<endl;
            for(;size>=1;size--){
                if(___verbose)cout<<"3.1"<<endl;
                if(verbose)cout<<"size:"<<size<<endl;
                std::vector<std::vector<const Stmt*>> injectionpoints = getMLPAListOfSize(it, size, compoundStmt);
                if(___verbose)cout<<"3.2"<<endl;
                if(verbose)cout << "found"<<injectionpoints.size()<<endl;
                if(___verbose)cout<<"3.3"<<endl;
                for(std::vector<const Stmt*> injectionpoint: injectionpoints){
                    if(verbose){
                        cout<<"--sublist:"<<injectionpoint.size()<<endl;
                        for(const Stmt * stmt:injectionpoint){
                            stmt->dumpColor();
                        }
                    }
                    nodeCallback(binding, injectionpoint);
                }
                if(___verbose)cout<<"3.4"<<endl;
            }
            if(___verbose)cout<<"4"<<endl;
        }
        if(___verbose)cout<<"5"<<endl;
    }
    if(___verbose)cout<<"6"<<endl;
    return false;
}