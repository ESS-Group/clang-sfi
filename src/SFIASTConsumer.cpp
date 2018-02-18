#include "ASTConsumer.h"

#include <sstream>
#include <string>
#include <algorithm>

//include "StmtHandler.h"
#include "FaultInjector.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"

#include <iostream>

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;


SFIASTConsumer::SFIASTConsumer(/*Rewriter &R,*/ std::string name, std::vector<FaultInjector*> injectors ): faultInjectors(injectors), fileName(name)/*,Rewrite(R)*/{
    /*for(FaultInjector injector: injectors.begin()){

    }


    std::vector<std::string> bindings;
    for(FaultInjector binding: injectors.begin())
        bindings.push_back(binding);
    stmtHandler = new StmtHandler(R.getSourceMgr(), name, bindings, nodeCallback);
    //Rw = R;
    //RW = &R;
    Matcher.addMatcher(ifStmt().bind("ifStmt"), &HandlerForIf);*/

        //cout << "SFIASTConsumer::SFIASTConsumer" << endl;
    for(FaultInjector *injector: faultInjectors){
        //Rewriter rw;
        //rw.setSourceMgr(R.getSourceMgr(), R.getLangOpts());
        //cout<<"SET REWRITER AND SOURCEMANAGER"<<endl;
        //injector.Rewrite = rw;
        //injector.setSourceMgr(R.getSourceMgr());
        //cout<<"name:"<<name<<endl;
        injector->setFileName(name);
        //cout << injector<< endl;
        //cout << injector->getFileName() << endl;
    
    }
}
void SFIASTConsumer::HandleTranslationUnit(ASTContext &Context) /*override*/{
    for(FaultInjector *injector: faultInjectors){
        //injector.setSourceMgr(Rewrite.getSourceMgr());
        //cout << "SFIASTConsumer::HandleTranslationUnit0" << endl;
        injector->matchAST(Context);

        cout << "Found " << injector->locations.size() << " " <<  injector->toString() << " injection locations" << endl;
        //cout << "SFIASTConsumer::HandleTranslationUnit1" << endl;
        //ASTContext &Context
        //Rewriter rw;
        //rw.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
                //rw.setSourceMgr(Rewrite.getSourceMgr() , lo);
        injector->inject(injector->locations, Context);

        //cout << injector<< endl;

        //cout << "SFIASTConsumer::HandleTranslationUnit2" << endl;
    }



    //Matcher.matchAST(Context);
}
        //void nodeCallback(std::string, const Stmt*){}
        //StmtHandler *stmtHandler;
        //IfStmtHandler HandlerForIf;
        //MatchFinder Matcher;
//        Rewriter Rewrite;
//        std::string fileName;
//        std::vector<FaultInjector> faultInjectors;

