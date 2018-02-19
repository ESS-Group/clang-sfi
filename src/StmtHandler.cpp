#include "StmtHandler.h"
#include "FaultInjector.h"


#include <iostream>
using namespace std;
StmtHandler::StmtHandler(FaultInjector *pFaultInjector, std::string name, std::vector<std::string> bindings/*,void (*nodeCallback)(std::string, const Stmt*)*/): /*nodeCallback(nodeCallback),*/bindings(bindings), fileName(name){
    //sourceManager = pFaultInjector->getSourceMgr();
    faultInjector = pFaultInjector;
}
void StmtHandler::run(const MatchFinder::MatchResult &Result){
//    cout << "StmtHandler::run0" << endl;
    for(std::string binding:bindings){
//        cout << "StmtHandler::run1" << endl;
        //if(const Stmt *stmt = Result.Nodes.getNodeAs<clang::Stmt>(binding.c_str())){
        //if(const Stmt *stmt = Result.Nodes.getNodeAs<clang::Stmt>("ifStmt")){
        if(const Stmt *stmt = Result.Nodes.getNodeAs<clang::Stmt>(binding)){

            //cout << "StmtHandler::run2" << endl;
            //stmt->getLocStart();
            //Result.Context->getSourceManager().dump();
            //cout << "StmtHandler::run2.5" << endl;
            //stmt->getLocStart().printToString(faultInjector->Rewrite.getSourceMgr());
            //segmentation fault
            //stmt->getLocStart().dump(faultInjector->Rewrite.getSourceMgr());
            //segmentation fault
            std::string name(Result.Context->getSourceManager().getFilename(stmt->getLocStart()));
    //cout << "StmtHandler::run2.75" << endl;
            
            //std::string name(faultInjector->getFileName());
            //cout << "StmtHandler::run3" << endl;
            //sourceManager->getFilename(stmt->getLocStart()).data()) == 0
            //cout << faultInjector->getFileName() << " | " << name << endl;
            if(faultInjector->getFileName().compare(name)==0){//nur Nodes aus dem zu parsenden File beachten!!
                //cout << "StmtHandler::run4" << endl;
                if(faultInjector->checkStmt(stmt, binding, *Result.Context))//nur Statemets die durch checkStmt erlaubt sind
                    faultInjector->nodeCallback(binding, stmt);
                //cout << "StmtHandler::run5" << endl;
            }
        } else if(const Decl *stmt = Result.Nodes.getNodeAs<clang::Decl>(binding)){
            if(const Decl *stmt = Result.Nodes.getNodeAs<clang::Decl>(binding)){

            std::string name(Result.Context->getSourceManager().getFilename(stmt->getLocStart()));
            if(faultInjector->getFileName().compare(name)==0){//nur Nodes aus dem zu parsenden File beachten!!
                if(faultInjector->checkStmt(stmt, binding, *Result.Context))//nur Statemets die durch checkStmt erlaubt sind
                    faultInjector->nodeCallback(binding, stmt);
                //cout<<"testitest"<<endl;
                }
            }
        }
    }
}