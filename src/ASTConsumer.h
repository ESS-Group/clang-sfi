#include <sstream>
#include <string>
#include <algorithm>

//include "StmtHandler.h"
class FaultInjector;
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

class SFIASTConsumer: public ASTConsumer{
    public:
        SFIASTConsumer(/*Rewriter &R,*/ std::string name, std::vector<FaultInjector*> injectors );
        void HandleTranslationUnit(ASTContext &Context) override;
    private:
        //void nodeCallback(std::string, const Stmt*){}
        //StmtHandler *stmtHandler;
        //IfStmtHandler HandlerForIf;
        //MatchFinder Matcher;
        //Rewriter Rewrite;
        std::string fileName;
        std::vector<FaultInjector*> faultInjectors;
};


class SFIAction : public SyntaxOnlyAction{
    public:
        std::vector<FaultInjector*> injectors;
        SFIAction(std::vector<FaultInjector*> injs);
        void EndSourceFileAction() override;
        std::unique_ptr<ASTConsumer> CreateASTConsumer( CompilerInstance &CI,
                                                        StringRef file) override;
    //private:
        //Rewriter Rewrite;
};