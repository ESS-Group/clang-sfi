#include "StmtHandler.h"
#include "FaultInjector.h"


#include <iostream>
using namespace std;
StmtHandler::StmtHandler(FaultInjector *pFaultInjector, std::string name, std::vector<std::string> bindings/*,void (*nodeCallback)(std::string, const Stmt*)*/): /*nodeCallback(nodeCallback),*/bindings(bindings), fileName(name){
    faultInjector = pFaultInjector;
}
void StmtHandler::run(const MatchFinder::MatchResult &Result){
    for(std::string binding:bindings){
        if(const Stmt *stmt = Result.Nodes.getNodeAs<clang::Stmt>(binding)){
            std::string name(Result.Context->getSourceManager().getFilename(stmt->getLocStart()));
            if(faultInjector->getFileName().compare(name)==0){//nur Nodes aus dem zu parsenden File beachten!!
                if(faultInjector->checkStmt(stmt, binding, *Result.Context))//nur Statemets die durch checkStmt erlaubt sind
                    faultInjector->nodeCallback(binding, stmt);
            }
        } else if(const Decl *stmt = Result.Nodes.getNodeAs<clang::Decl>(binding)){
                std::string name(Result.Context->getSourceManager().getFilename(stmt->getLocStart()));
                if(faultInjector->getFileName().compare(name)==0){//nur Nodes aus dem zu parsenden File beachten!!
                    if(faultInjector->checkStmt(stmt, binding, *Result.Context)){//nur Statemets die durch checkStmt erlaubt sind
                        faultInjector->nodeCallback(binding, stmt);
                    }
                }
        }
    }
}