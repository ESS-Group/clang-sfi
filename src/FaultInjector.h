#ifndef FAULTINJECTOR
#define FAULTINJECTOR 1

#include <vector>
#include <algorithm>
#include "clang/ASTMatchers/ASTMatchFinder.h"
//include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/AST/AST.h"
#include "clang/Rewrite/Core/Rewriter.h"

class StmtHandler;
//include "StmtHandler.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

class FaultInjector{
    public:
        class StmtBinding{
            public:
                StmtBinding(std::string binding, const Stmt* stmt):binding(binding){
                    this->stmt = stmt;
                }
                std::string binding;
                const Stmt* stmt;
        };
        StmtHandler* createStmtHandler(std::string binding);
        //std::vector<StmtHandler> test;
        FaultInjector();
        ~FaultInjector();
        FaultInjector(const FaultInjector& that);

        void push(std::string binding, const Stmt *st);
        virtual void inject(std::vector<StmtBinding> target, ASTContext &Context);
        virtual std::string inject(StmtBinding current, ASTContext &Context) = 0;
        virtual bool checkStmt(const Stmt* stmt, std::string binding) = 0;
        void matchAST(ASTContext &Context);
        virtual std::string toString() = 0;

        std::vector<StmtBinding> locations;
        //void setSourceMgr(SourceManager &sourceManager);
        //SourceManager* getSourceMgr();
        //Rewriter Rewrite;
        void nodeCallback(std::string binding, const Stmt* stmt);
        std::string getFileName();
        void setFileName(std::string name);
        void setVerbose(bool v);
        void setDirectory(std::string directory);
    protected:
        static void dumpStmt(const Stmt* stmt, ASTContext &Context);
        static std::string stmtToString(const Stmt* stmt/*, SourceManager &sourceManager*/, const LangOptions &langOpts);
        static std::string sourceLocationToString(SourceLocation loc,const SourceManager &sourceManager);
        static std::string sourceRangeToString(SourceRange range,const SourceManager &sourceManager);
        static std::string sourceRangeToString(const Stmt *stmt,const SourceManager &sourceManager);
        static std::string rewriteBufferToString(RewriteBuffer &buffer);
        std::string getEditedString(Rewriter &rewrite, ASTContext &Context);
        void writeDown(std::string data, int i);
        void printStep(StmtBinding current, const SourceManager &sourceManager, const LangOptions &langOpts, int i=0, int size=0);//with printing statements
        void printStep(StmtBinding current, const SourceManager &sourceManager, int i=0, int size=0);//only position
        std::string fileName;
        //SourceManager *sourceMgr;
        std::vector<std::string> bindings;
        MatchFinder Matcher;//child Classes have to add Matchers!!
        void _sort();
        static bool comparefunc(StmtBinding st1, StmtBinding st2);
        bool verbose;
        std::string dir;
};
class MIFSInjector: public FaultInjector{
    public:
        MIFSInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Stmt* stmt, std::string binding) override;
};

class MIAInjector: public FaultInjector{
    public:
        MIAInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Stmt* stmt, std::string binding) override;
};

class MIEBInjector: public FaultInjector{
    public:
        MIEBInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Stmt* stmt, std::string binding) override;
};
#endif