#include <algorithm>





std::vector<const DeclRefExpr*> getAllRefs(const Stmt *parent, const VarDecl* var){
    //
    //parent->dumpColor();
    //var->dumpColor();
    std::vector<const DeclRefExpr*> ret;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == NULL){
        }else if(isa<Stmt>(*i)){
            //
            if(isa<DeclRefExpr>(*i)){
                //i->dumpColor();
                if(((const DeclRefExpr *)*i)->getDecl()==var)
                ret.push_back((const DeclRefExpr*) *i);
            } else {
                std::vector<const DeclRefExpr*> list = getAllRefs(*i, var);
                if(list.size()!=0)
                    concatVector<const DeclRefExpr*>(ret,list);
            }
        }
    }
    //
    return ret;
}

const DeclRefExpr* getLatestRef(const Stmt *parent, const VarDecl* var){
    std::vector<const DeclRefExpr*> refs = getAllRefs(parent, var);
    //var->dumpColor();
    //cout << ">>>"<<endl;
    //parent->dumpColor();
    //
    std::vector<const DeclRefExpr*> ret;
    for(const DeclRefExpr* ref : refs){
        //ref->dumpColor();
        if(ret.size()){
            for(const DeclRefExpr* reference:ret){
                if(reference->getLocEnd()<ref->getLocEnd()){
                    ret.clear();
                    ret.push_back(ref);
                }
            }
        } else {
            ret.push_back(ref);
        }
        /*
        if(ret == NULL)
            ret = ref;
        else if(ret->getLocEnd()<ref->getLocEnd())
            ret = ref;
        */
    }

    //cout << "<<<"<<endl;

    for(const DeclRefExpr* ref : ret){
        //ref->dumpColor();
        //cout << ":)"<<endl;
        return ref;
    }
        //cout << ":("<<endl;
    return NULL;
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

    std::vector<const T*> ret;
    if(list.empty())
        return ret;
    for(const Stmt* stmt:list){
        if(stmt!=NULL && isa<T>(stmt)){
            ret.push_back((const T*)stmt);

        }
    }

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





std::vector<std::vector<const Stmt*>> getStmtLists(const CompoundStmt * block, ASTContext& Context){
    std::vector<std::vector<const Stmt*>> ret;
    int index = -1;
    for(Stmt::child_iterator i = cast_away_const(block->child_begin()), e = cast_away_const(block->child_end());i!=e;++i){
        if(*i == NULL){

            //

        }else if(isa<Stmt>(*i)){
            if(index==-1){

                std::vector<const Stmt*> list;
                ret.push_back(list);
                index = 0;

            }
            if(!isa<IfStmt>(*i) && !isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i)&&!isa<SwitchStmt>(*i) && !isa<ReturnStmt>(*i)&& !isa<BreakStmt>(*i)&& !isa<ContinueStmt>(*i)){
                if(isa<CompoundStmt>(*i)){
                    if(ret[index].size()>0){
                        std::vector<const Stmt*> list;
                        ret.push_back(list);
                        index++;
                    }

                    //TODO: maybe include the statements inside the CompoundStmt
                } else {

                    ret.at(index).push_back(*i);

                }
            } else if(ret[index].size()>0){

                std::vector<const Stmt*> list;
                ret.push_back(list);
                index++;

                
            }
        }
    }


    if(index <= 0 && (ret.size()==0 || ret[0].size()==0)){
        std::vector<std::vector<const Stmt*>> ret;
        return ret;
    }




    //for(auto list:ret){
    std::vector<std::vector<const Stmt*>>::reverse_iterator rit = ret.rbegin();


    //std::vector<const Stmt*>::iterator toDeleteIt;
    //std::vector<const Stmt*>::iterator* toDelete = NULL;
    bool deleteIt = false;

    for(;rit!=ret.rend();/*++rit*/){


        std::vector<const Stmt*> list = *rit;

        std::vector<const DeclStmt*> declstmts = getStmtsOfType<DeclStmt>(list);


        std::vector<const DeclStmt*> notPossible;

        for(const DeclStmt* declstmt:declstmts){

            std::vector<const DeclRefExpr*> ref;
            for(auto decl:declstmt->decls()){

               const DeclRefExpr* latest = getLatestRef(block, (const VarDecl*)decl);

                if(ref.size()){
                    ref.clear();
                    ref.push_back(latest);
                } else {
                    ref.push_back(latest);
                }

            }

            if(ref.size()/*ref!=NULL*/){

                for(const DeclRefExpr* reference:ref){

                    if(reference!=NULL && list.back()->getLocEnd()<reference->getLocStart() && std::find(list.begin(), list.end(), declstmt)!=list.end()){

                        const DeclStmt* statement = (const DeclStmt*)declstmt;//declStmt;

                        notPossible.push_back(statement);
                    }

                    break;
                }

            }

        }

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
            /*
            bool deletedFirst = false;
                cout<<"<<3";
            for(const DeclStmt* stmt:notPossible){

                ///////////////////////////////////////////////////////////////////////////////stmt->dumpColor();
                //bool deletechanged = false;
                cout<<"hohoho1"<<endl;
                for(std::vector<std::vector<const Stmt*>>::iterator it=changed.begin();it!=changed.end();++it){
                    //auto changedPosition = std::find(changed.begin(), changed.end(), *it);
                    cout<<"hohoho1.5"<<endl;
                    auto position = std::find(it->begin(), it->end(),stmt);//std::find((*it).rbegin(), (*it).rend(), stmt);
                    cout<<"hohoho2"<<endl;
                    int pos = std::distance(it->begin(), position);//(*it).size() - std::distance((*it).rbegin(), position);
                    cout<<"hohoho3"<<endl;
                    /*if(position!=it->end()){

                        if(*position == NULL){

                        }else{

                        }

                    }*//*
                    if(position!=it->end()){
                        cout<<"hohoho4"<<endl;
                        std::vector<const Stmt*> first(it->begin(), it->begin()+pos),//((*it).begin(), (*it).begin()+pos),
                                    second(it->begin()+pos+1, it->end());//((*it).begin()+pos+1, (*it).end());
                        cout<<"hohoho5"<<endl;
                        if(first.size()!=0){
                            changed.push_back(first);

                        }cout<<"hohoho6"<<endl;
                        if(second.size()!=0){
                            changed.push_back(second);

                        }cout<<"hohoho7"<<endl;
                        deleteIt = true;

                        std::vector<std::vector<const Stmt*>>::iterator iter;
                        int pos = std::distance(changed.begin(), it);
                        
                        cout << pos << endl;
                        std::vector<std::vector<const Stmt*>> changedNew(changed.begin(), it);
                        cout << "huhu"<<endl;
                        if(it != changed.end())
                            changedNew.insert(changedNew.end(), it+1, changed.end());
                        changed = changedNew;
                        cout << "huhu2"<<endl;
                        //it->clear();
                        //deletechanged = true;

                        //toDeleteIt = position;
                        //toDelete = &toDeleteIt;
                        break;

                        //UNBEDINGT IM NÄCHSTEN SCHRITT DER ÄUSSERSTEN SCHLEIFE EINTRAG LÖSCHEN
                    }
                    cout<<"huhu3"<<endl;
                }
                    cout<<"huhu4"<<endl;
                /*if(changed.size()>0&&!deletedFirst){
                    changed = std::vector<std::vector<const Stmt*>>(changed.begin()+1, changed.end());
                    deletedFirst = true;
                }*//*
            }
                    cout<<"huhu5"<<endl;
                    */

                //cout<<"<<4";




            int ritpos = std::distance(ret.rbegin(), rit)+changed.size();

            ret.insert(ret.end(), changed.begin(),changed.end());//invalidates iterator
            rit = ret.rbegin();
            for(int i = 0 ; i<ritpos;i++,rit++);
            list = *rit;
            
            //cout<<"huhu6"<<endl;




        } 
        //cout<<"huhu7"<<endl;

                //cout<<"<<3";
        //get
        if(deleteIt){
            /*auto it = ret.erase(--rit.base());
            rit=std::vector<std::vector<const Stmt*>>::reverse_iterator(it);//ret.erase(rit.base());
            deleteIt = false;*/



            //cout<<"huhu71"<<endl;
            rit->clear();

            //cout<<"huhu72"<<endl;
            deleteIt = false; 

            ++rit;
        } else{


            ++rit;
        }
        //cout<<"huhu8"<<endl;
    }








    //TODO: delete all empty lists??
    //ret;


    return ret;

}



bool isMLPAListPossible(std::vector<const Stmt*> stmtlist, const CompoundStmt* block){
    std::vector<const DeclStmt*> declstmts = getStmtsOfType<DeclStmt>(stmtlist);

    std::vector<const DeclStmt*> notPossible;

    for(const DeclStmt* declstmt:declstmts){
        std::vector<const DeclRefExpr*> ref;
        for(auto decl:declstmt->decls()){
            const DeclRefExpr* latest = getLatestRef(block, (const VarDecl*)decl);
            if(ref.size()){
                for(const DeclRefExpr* x:ref){
                    if(x->getLocStart()<latest->getLocStart()){
                        ref.clear();
                        ref.push_back(latest);
                        break;
                    }
                }
            } else {
                ref.push_back(latest);
            }
            /*if(ref==NULL)
                ref = latest;
            else if(ref->getLocStart()<latest->getLocStart())
                ref = latest;
            */
        }
        for(const DeclRefExpr* reference:ref){
            if(reference!=NULL && stmtlist.back()->getLocEnd()<reference->getLocStart() && std::find(stmtlist.begin(), stmtlist.end(), declstmt)!=stmtlist.end()){
                const DeclStmt* statement = (const DeclStmt*)declstmt;//declStmt;
                notPossible.push_back(statement);
            }
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
                    allOf(
                        unless(hasParent(declStmt())),
                        unless(hasParent(switchStmt()))
                    )
                ).bind("compoundStmt"), 
                createStmtHandler("compoundStmt")
        );

}

std::string MLPAInjector::toString(){
    return "MLPA";
};




std::string MLPAInjector::inject(StmtBinding current, ASTContext &Context){
    

    std::vector<const Stmt*> list = current.stmtlist;

    SourceLocation begin=list[0]->getLocStart(), end=list[0]->getLocEnd();

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

    const CompoundStmt * compoundStmt = (const CompoundStmt *)stmt;

    std::vector<std::vector<const Stmt*>> stmtlists = getStmtLists(compoundStmt, Context);
    //int s = stmt->size();
    /*
    if(compoundStmt->size()>2){
        stmt->dumpColor();
        cerr << "=================" << endl;
        for(std::vector<const Stmt*> it:stmtlists){
            
                        cerr<<"--sublist:"<<it.size()<<endl;
                        /*
                        for(const Stmt * stmt:it){
                            stmt->dumpColor();
                        }
                        cerr<<"--endlist"<<endl;
                        *//*
        }
    }
    return false;
    */
    for(std::vector<const Stmt*> it:stmtlists){

        if(it.size()>=2){
            //if(isMLPAListPossible(it,compoundStmt))
            //nodeCallback(binding, it);continue;

            int size = it.size();
            if(size == compoundStmt->size())//because 1 statement must remain within the compoundStmt
                size--;
            /*if(verbose){
                cout<<"--- new List ---"<<size<<endl;
            }*/
            if(size>5)
                size=5;
            
            for(;size>=2;size--){

                //if(verbose)cout<<"size:"<<size<<endl;

                std::vector<std::vector<const Stmt*>> injectionpoints = getMLPAListOfSize(it, size, compoundStmt);

                //if(verbose)cout << "found"<<injectionpoints.size()<<endl;

                for(std::vector<const Stmt*> injectionpoint: injectionpoints){
                    if(verbose){
                        //cout<<"--sublist:"<<injectionpoint.size()<<endl;
                        for(const Stmt * stmt:injectionpoint){
                            stmt->dumpColor();
                        }
                    }
                    nodeCallback(binding, injectionpoint);
                }

            }

        }

    }

    return false;
}