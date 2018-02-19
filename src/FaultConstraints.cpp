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
    int count = 0;
    const clang::Stmt::const_child_iterator &begin = stmt->child_begin();
    const clang::Stmt::const_child_iterator &end = stmt->child_end();
    StmtIterator it = cast_away_const(begin);
    while(it!=cast_away_const(end)){
        //if(isa<ForStmt>(*it) || isa<WhileStmt>(*it) || isa<DoStmt>(*it))//schleife
        //    return false;
        count++;
        it++;
    }
    return count;
}

bool C2(const Stmt * stmt, ASTContext &Context){
    ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
    //cout << list.size() << " Parents";
    if(!list.empty()){
        if(isa<CompoundStmt>(list[0].get<Stmt>())){
            const CompoundStmt* container = list[0].get<CompoundStmt>();
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
    ASTContext::DynTypedNodeList list = Context.getParents(*decl);
    //cout << list.size() << " Parents";
    //cout<<"----------------------------"<<endl;
        for(auto p : list){
            //p.get<Stmt>()->dump(Context.getSourceManager());
        }
    if(!list.empty()){
        if(isa<CompoundStmt>(list[0].get<Stmt>())){
            const CompoundStmt* container = list[0].get<CompoundStmt>();
            return childCount(container)>1;
        } else if(isa<DeclStmt>(list[0].get<Stmt>())){
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