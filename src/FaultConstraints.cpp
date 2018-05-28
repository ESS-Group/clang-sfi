#include <vector>
#include <algorithm>
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/AST/AST.h"
#include "StmtHandler.h"

#include <iostream>
#include <fstream>

#include "llvm/Support/raw_ostream.h"
using namespace llvm;
//MIFS
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

bool C9(const clang::Stmt::const_child_iterator &begin, const clang::Stmt::const_child_iterator &end){
    StmtIterator it = cast_away_const(begin);
    int num = 0;
    while(it!=cast_away_const(end)){
        if(isa<ForStmt>(*it) || isa<WhileStmt>(*it) || isa<DoStmt>(*it))//schleife
            return false;
        else if(isa<IfStmt>(*it) || isa<SwitchStmt>(*it))//other jumps
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

int childCount(const Stmt* stmt){
    //stmt->dumpColor();
    int count = 0;
    //cout<<"childCount1"<<endl;
    const clang::Stmt::const_child_iterator &begin = stmt->child_begin();
    const clang::Stmt::const_child_iterator &end = stmt->child_end();
    //cout<<"childCount2"<<endl;
    StmtIterator it = cast_away_const(begin);
    //cout<<"childCount3"<<endl;
    while(it!=cast_away_const(end)){
        //cout<<"childCount4"<<endl;
        //if(isa<ForStmt>(*it) || isa<WhileStmt>(*it) || isa<DoStmt>(*it))//schleife
        //    return false;
        if(*it != NULL)
            count++;
        it++;
    }
    //cout<<"childCount5"<<endl;
    return count;
}

bool isaImplicit(const Stmt* stmt){
    return isa<ExprWithCleanups>(stmt) || isa<MaterializeTemporaryExpr>(stmt) || isa<CXXBindTemporaryExpr>(stmt) || isa<ImplicitCastExpr>(stmt);
}
//template<class T>
ASTContext::DynTypedNodeList getParentsIgnoringImplicit(const Stmt* stmt, ASTContext &Context){
    //cerr<<"getParentsIgnoringImplicit"<<endl;
    ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
    if(!list.empty()){
        if(list[0].get<Stmt>()!=NULL){
            
            const Stmt* temp = list[0].get<Stmt>();
            if(isaImplicit(temp)){
                //cerr<<"isaImplicit"<<endl;
                //temp->dumpColor();
                return getParentsIgnoringImplicit(temp, Context);
            }else
                return list;
        } else
            return list;
    } else
        return list;
}

bool C2(const Stmt * stmt, ASTContext &Context){
    //cout<<"C2Stmt1"<<endl;
    ASTContext::DynTypedNodeList list = getParentsIgnoringImplicit(stmt, Context);
    //stmt->IgnoreImplicit()->dumpColor();
    //cout<<"C2Stmt2"<<endl;
    //cerr <<endl << list.size() << " Parents"<<endl;
    if(!list.empty()){
        //cout<<"C2Stmt3"<<endl;
        //if(list[0].get<Stmt>()!=NULL&& !isa<CompoundStmt>(list[0].get<Stmt>())){
            //list[0].get<Stmt>()->IgnoreImplicit()->dumpColor();
            //cerr<<"--"<<endl;
        //}
        if(list[0].get<Stmt>()!=NULL && isa<CompoundStmt>(list[0].get<Stmt>())){
            //cout<<"C2Stmt4.1.1"<<endl;
            const CompoundStmt* container = (const CompoundStmt*) list[0].get<Stmt>();
            //if(container != NULL)
            //    container->dumpColor();
            //else
            //    cerr<<"NULL"<<endl;
            //cout<<"C2Stmt4.1.2"<<endl;
            return container->size()>1;
        } else if(list[0].get<Stmt>()!=NULL && isa<SwitchCase>(list[0].get<Stmt>())){
            const SwitchCase* container = (const SwitchCase*) list[0].get<Stmt>();
            return childCount(container)>1;
        } else return false;
        /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    } //else
        //return false;
        //cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    //cout<<"#######################"<<endl;
    return false;
}


bool C2(const Decl *decl, ASTContext &Context){
    //cout<<"C2Decl1"<<endl;
    ASTContext::DynTypedNodeList list = Context.getParents(*decl);
    //cout << list.size() << " Parents";
    //cout<<"----------------------------"<<endl;
        //for(auto p : list){
        //    cout<<"C2Decl2"<<endl;
            //p.get<Stmt>()->dump(Context.getSourceManager());
        //}
    if(!list.empty()){
        //cout<<"C2Decl3"<<endl;
        if(list[0].get<Stmt>() == NULL){
            //decl->dumpColor();
            return false;
        }if(isa<CompoundStmt>(list[0].get<Stmt>())){
            //cout<<"C2Decl4.1.1"<<endl;
            const CompoundStmt* container = list[0].get<CompoundStmt>();
            //cout<<"C2Decl4.1.2"<<endl;
            return childCount(container)>1;
        } else if(isa<DeclStmt>(list[0].get<Stmt>())){
            //cout<<"C2Decl4.2"<<endl;
            return C2(list[0].get<Stmt>(), Context);
        } return false;
        /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    } //else
        //return false;
        //cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    //cout<<"#######################"<<endl;
    return false;
}
/*

bool C1(const Stmt *stmt){
    return true;
}
*/