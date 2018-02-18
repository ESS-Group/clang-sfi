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
                if(faultInjector->checkStmt(stmt, binding))//nur Statemets die durch checkStmt erlaubt sind
                    faultInjector->nodeCallback(binding, stmt);
                //cout << "StmtHandler::run5" << endl;
            }
        }
        //cout << "StmtHandler::run6" << endl;
    }
}



/*
class IfStmtHandler : public MatchFinder::MatchCallback {
    public:
        IfStmtHandler(Rewriter &Rewrite, std::string name): Rewrite(Rewrite){fileName = name;}
        virtual void run(const MatchFinder::MatchResult &Result){
            
               
            if(const IfStmt *IfS = (const IfStmt*)(Result.Nodes.getNodeAs<clang::Stmt>("ifStmt"))){
                std::string name(Rewrite.getSourceMgr().getFilename(IfS->getLocStart()).data());
                if(fileName.compare(name) == 0){
                    //cout<<Rewrite.getSourceMgr().getFilename(IfS->getLocStart()).data()<<endl;
                    //countIf++;
                    const Stmt *Then = IfS->getThen();
                    //Rewrite.InsertText(Then->getLocStart(), "// the 'if' part\n", true, true);
                    //Rewrite.InsertText(Then->getLocEnd(), "// the 'if' part - end\n", true, true);
/*
                    if(const Stmt *Else = IfS->getElse()){
                        countElse++;
                        Rewrite.RemoveText(Else->getSourceRange());
                        //Rewrite.InsertText(Else->getLocStart(), "// the 'else' part begin\n", false, true);
                        //Rewrite.InsertText(Else->getLocEnd(), "// the 'else' part end\n");

                    }*//*
                    if(const Stmt *Else = IfS->getElse()){
                        if(!countIf++){
                            SourceRange sr(IfS->getLocStart(), Else->getLocStart().getLocWithOffset(-1));//-1 Offset da sonst das erste Zeichen des Else blockes mit gelöscht wird
                            locations.push_back(sr);
                            Rewrite.RemoveText(sr);
                            
                        }
                    }
                }
            }
            /*if(const IfStmt *IfS = Result.Nodes.getNodeAs<clang::IfStmt>("ifStmt")){

                Rewrite.InsertText(Then->getLocStart(), "// the 'if' part\n", true, true);
                locations.push_back(IfS->getSourceRange());
                //Rewriter rw;
                //SourceManager *sm = &Rewrite.getSourceMgr();
                //rw.setSourceMgr(Rewrite.getSourceMgr() , lo);
                //const Stmt *Then = IfS->getThen();
                //rw.RemoveText(IfS->getSourceRange());
                //rw.InsertText(Then->getLocStart(), "// the 'if' part\n", true, true);
                //rw.getEditBuffer(Rewrite.getSourceMgr().getMainFileID()).write(llvm::outs());//an dieser Stelle durch änderungen durchiterieren (mit begin() den iterator erhalten)
                //if(IfS->getLocStart().isValid() && IfS->getLocStart().isFileID())
                //    locations.push_back(IfS->getSourceRange());
                //Rewrite.RemoveText(IfS->getThen()->getSourceRange());
                //Rewrite.getEditBuffer(Rewrite.getSourceMgr().getMainFileID()).write(llvm::outs());
            }*//*
        }
    private:
        Rewriter &Rewrite;
        std::string fileName;
};
*/