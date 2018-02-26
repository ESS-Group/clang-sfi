#ifndef FAULTINJECTOR
#define FAULTINJECTOR 1

#include <vector>
#include <algorithm>
#include "clang/ASTMatchers/ASTMatchFinder.h"
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
                StmtBinding(std::string binding, const Decl* decl):binding(binding){
                    this->decl = decl;
                    isStmt = false;
                }
                StmtBinding(std::string binding, const Stmt* stmt):binding(binding){
                    this->stmt = stmt;
                    isStmt = true;
                }
                const void *get(){
                    if(isStmt)
                        return stmt;
                    else
                        return decl;
                }
                bool isStmt;
                std::string binding;
                const Stmt* stmt;
                const Decl* decl;
        };
        StmtHandler* createStmtHandler(std::string binding);
        //std::vector<StmtHandler> test;
        FaultInjector();
        ~FaultInjector();
        FaultInjector(const FaultInjector& that) = delete;

        void push(std::string binding, const Stmt *st);
        void push(std::string binding, const Decl *st);
        virtual void inject(std::vector<StmtBinding> target, ASTContext &Context);
        virtual std::string inject(StmtBinding current, ASTContext &Context) = 0;
        virtual bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context);
        virtual bool checkStmt(const Decl* stmt, std::string binding, ASTContext &Context);
        void matchAST(ASTContext &Context);
        virtual std::string toString() = 0;

        std::vector<StmtBinding> locations;
        //void setSourceMgr(SourceManager &sourceManager);
        //SourceManager* getSourceMgr();
        //Rewriter Rewrite;
        void nodeCallback(std::string binding, const Stmt* stmt);
        void nodeCallback(std::string binding, const Decl* decl);
        std::string getFileName();
        void setFileName(std::string name);
        void setVerbose(bool v);
        void setDirectory(std::string directory);
    protected:
        static void dumpStmt(const Stmt* stmt, ASTContext &Context);
        static std::string stmtToString(const Stmt* stmt/*, SourceManager &sourceManager*/, const LangOptions &langOpts);
        static void dumpStmt(const Decl* decl);
        static std::string stmtToString(const Decl* decl, const LangOptions &langOpts);
        static std::string sourceLocationToString(SourceLocation loc,const SourceManager &sourceManager);
        static std::string sourceRangeToString(SourceRange range,const SourceManager &sourceManager);
        static std::string sourceRangeToString(const Stmt *stmt,const SourceManager &sourceManager);
        static std::string sourceRangeToString(const Decl *decl,const SourceManager &sourceManager);
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
        bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context) override;
};

class MIAInjector: public FaultInjector{
    public:
        MIAInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context) override;
};

class MIEBInjector: public FaultInjector{
    public:
        MIEBInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context) override;
};

class WAEPInjector: public FaultInjector{
    public:
        WAEPInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context) override;
};

class MFCInjector: public FaultInjector{
    public:
        MFCInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context) override;
};


class MLOCInjector: public FaultInjector{
    public:
        MLOCInjector();
        std::string toString() override;
        void inject(std::vector<StmtBinding> target, ASTContext &Context) override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        std::string inject(StmtBinding current, ASTContext &Context, bool left);
        bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context) override;
};

class MLACInjector: public FaultInjector{
    public:
        MLACInjector();
        std::string toString() override;
        void inject(std::vector<StmtBinding> target, ASTContext &Context) override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        std::string inject(StmtBinding current, ASTContext &Context, bool left);
        bool checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context) override;
};

class MVIVInjector: public FaultInjector{
    public:
        MVIVInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Decl* decl, std::string binding, ASTContext &Context) override;
};


class MVAVInjector: public FaultInjector{
    public:
        MVAVInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Decl* decl, std::string binding, ASTContext &Context) override;
};
class WVAVInjector: public FaultInjector{
    public:
        WVAVInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Decl* decl, std::string binding, ASTContext &Context) override;
};

class MVAEInjector: public FaultInjector{
    public:
        MVAEInjector();
        std::string toString() override;
        std::string inject(StmtBinding current, ASTContext &Context) override;
        bool checkStmt(const Decl* decl, std::string binding, ASTContext &Context) override;
};
#endif