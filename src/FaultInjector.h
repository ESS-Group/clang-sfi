#ifndef FAULTINJECTOR
#define FAULTINJECTOR 1
#define MAXSTATEMENTNUMFORCONSTRAINT 5
#define DONOTDELETEDECLSTMTINCONSTRAINT false
#define RETURNISAJUMP true
#define DONTCOUNTBREAKINSWITCHCASEFORBLOCKSIZE true

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <algorithm>
#include <sstream>
#include <vector>

class StmtHandler;
// include "StmtHandler.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

class FaultInjector {
  public:
    class StmtBinding {
      public:
        class Location {
          public:
            unsigned int line;
            unsigned int column;
            Location(unsigned int pLine, unsigned int pColumn)
                : line(pLine), column(pColumn){};
            Location(){};
            std::string toString() {
                std::stringstream ss;
                ss << line << ":" << column;
                return ss.str();
            }
        };
        class Range {
          public:
            Location begin;
            Location end;
            Range(Location pBegin, Location pEnd)
                : begin(pBegin), end(pEnd), valid(true){};
            Range() : valid(false){};
            bool isValid() { return valid; };
            std::string toString() {
                if (isValid()) {
                    std::stringstream ss;
                    ss << begin.toString() << " - " << end.toString() << endl;
                    return ss.str();
                } else
                    return "INVALID";
            }

          private:
            bool valid;
        };
        StmtBinding(std::string binding, const Decl *decl, bool left = false)
            : binding(binding) {
            this->left = left;
            this->decl = decl;
            decllist.push_back(decl);
            isStmt = false;
            isList = false;
        }
        StmtBinding(std::string binding, const Stmt *stmt, bool left = false)
            : binding(binding) {
            this->left = left;
            this->stmt = stmt;
            stmtlist.push_back(stmt);
            isStmt = true;
            isList = false;
        }
        StmtBinding(std::string binding, std::vector<const Decl *> list,
                    bool left = false)
            : binding(binding), decllist(list.begin(), list.end()) {
            this->left = left;
            isStmt = false;
            isList = true;
        }
        StmtBinding(std::string binding, std::vector<const Stmt *> list,
                    bool left = false)
            : binding(binding), stmtlist(list.begin(), list.end()) {
            this->left = left;
            isStmt = true;
            isList = true;
        }

        void calculateRange(ASTContext &Context) {
            SourceLocation begin, end;
            if (isList) {
                if (isStmt) {
                    for (int i = 0; i < stmtlist.size(); i++) {
                        if (i == 0) {
                            begin = stmtlist[0]->getLocStart();
                            end = stmtlist[0]->getLocEnd();
                        } else {
                            SourceLocation _begin = stmtlist[i]->getLocStart(),
                                           _end = stmtlist[i]->getLocEnd();
                            if (end < _end)
                                end = _end;
                            if (_begin < begin)
                                begin = _begin;
                        }
                    }
                } else {
                    for (int i = 0; i < decllist.size(); i++) {
                        if (i == 0) {
                            begin = decllist[0]->getLocStart();
                            end = decllist[0]->getLocEnd();
                        } else {
                            SourceLocation _begin = decllist[i]->getLocStart(),
                                           _end = decllist[i]->getLocEnd();
                            if (end < _end)
                                end = _end;
                            if (_begin < begin)
                                begin = _begin;
                        }
                    }
                }
            } else {

                if (isStmt) {
                    begin = stmt->getLocStart();
                    end = stmt->getLocEnd();
                } else {
                    begin = decl->getLocStart();
                    end = decl->getLocEnd();
                }
            }
            if (begin.isValid() && end.isValid()) {
                FullSourceLoc fBegin = Context.getFullLoc(begin),
                              fEnd = Context.getFullLoc(end);
                location = Range(
                    Location(fBegin.getLineNumber(), fBegin.getColumnNumber()),
                    Location(fEnd.getLineNumber(), fEnd.getColumnNumber()));
            }
        }
        const void *get() {
            if (isList) {
                if (isStmt)
                    return &stmtlist;
                else
                    return &decllist;
            } else {
                if (isStmt)
                    return stmt;
                else
                    return decl;
            }
        }
        bool isStmt;
        bool isList;
        std::string binding;
        const Stmt *stmt;
        const Decl *decl;
        std::vector<const Stmt *> stmtlist;
        std::vector<const Decl *> decllist;
        Range location;
        bool left;
    };
    StmtHandler *createStmtHandler(std::string binding);
    // std::vector<StmtHandler> test;
    FaultInjector();
    ~FaultInjector();
    FaultInjector(const FaultInjector &that) = delete;

    void push(std::string binding, const Stmt *st, bool left = false);
    void push(std::string binding, const Decl *st, bool left = false);
    void push(std::string binding, std::vector<const Stmt *> list);
    void push(std::string binding, std::vector<const Decl *> list);
    virtual void inject(std::vector<StmtBinding> target, ASTContext &Context);
    virtual std::string inject(StmtBinding current, ASTContext &Context) = 0;
    virtual bool checkStmt(const Stmt *stmt, std::string binding,
                           ASTContext &Context);
    virtual bool checkStmt(const Decl *stmt, std::string binding,
                           ASTContext &Context);
    void matchAST(ASTContext &Context);
    virtual std::string toString() = 0;

    std::vector<StmtBinding> locations;
    // void setSourceMgr(SourceManager &sourceManager);
    // SourceManager* getSourceMgr();
    // Rewriter Rewrite;
    void nodeCallback(std::string binding, const Stmt *stmt, bool left = false);
    void nodeCallback(std::string binding, const Decl *decl, bool left = false);
    void nodeCallback(std::string binding, std::vector<const Stmt *> list);
    void nodeCallback(std::string binding, std::vector<const Decl *> list);
    std::string getFileName();
    void setFileName(std::string name);
    void setVerbose(bool v);
    void setDirectory(std::string directory);

  protected:
    static void dumpStmt(const Stmt *stmt, ASTContext &Context);
    static std::string
    stmtToString(const Stmt *stmt /*, SourceManager &sourceManager*/,
                 const LangOptions &langOpts);
    static void dumpStmt(const Decl *decl);
    static std::string stmtToString(const Decl *decl,
                                    const LangOptions &langOpts);
    static std::string
    sourceLocationToString(SourceLocation loc,
                           const SourceManager &sourceManager);
    static std::string sourceRangeToString(SourceRange range,
                                           const SourceManager &sourceManager);
    static std::string sourceRangeToString(const Stmt *stmt,
                                           const SourceManager &sourceManager);
    static std::string sourceRangeToString(const Decl *decl,
                                           const SourceManager &sourceManager);
    static std::string rewriteBufferToString(RewriteBuffer &buffer);
    std::string getEditedString(Rewriter &rewrite, ASTContext &Context);
    void writeDown(std::string data, int i);
    void printStep(StmtBinding current, const SourceManager &sourceManager,
                   const LangOptions &langOpts, int i = 0,
                   int size = 0); // with printing statements
    void printStep(StmtBinding current, const SourceManager &sourceManager,
                   int i = 0, int size = 0); // only position
    std::string fileName;
    // SourceManager *sourceMgr;
    std::vector<std::string> bindings;
    MatchFinder Matcher; // child Classes have to add Matchers!!
    void _sort();
    static bool comparefunc(StmtBinding st1, StmtBinding st2);
    bool verbose;
    std::string dir;
};
class MIFSInjector : public FaultInjector {
  public:
    MIFSInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class SMIFSInjector : public MIFSInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MIAInjector : public FaultInjector {
  public:
    MIAInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class SMIAInjector : public MIAInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MIEBInjector : public FaultInjector {
  public:
    MIEBInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class SMIEBInjector : public MIEBInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class WAEPInjector : public FaultInjector {
  public:
    WAEPInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class WPFVInjector : public FaultInjector {
  public:
    WPFVInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MFCInjector : public FaultInjector {
  public:
    MFCInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MLOCInjector : public FaultInjector {
  public:
    MLOCInjector();
    std::string toString() override;
    // void inject(std::vector<StmtBinding> target, ASTContext &Context)
    // override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    // std::string inject(StmtBinding current, ASTContext &Context, bool left);
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MLACInjector : public FaultInjector {
  public:
    MLACInjector();
    std::string toString() override;
    // void inject(std::vector<StmtBinding> target, ASTContext &Context)
    // override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    // std::string inject(StmtBinding current, ASTContext &Context, bool left);
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MVIVInjector : public FaultInjector {
  public:
    MVIVInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Decl *decl, std::string binding,
                   ASTContext &Context) override;
};

class MVAVInjector : public FaultInjector {
  public:
    MVAVInjector(bool alsoOverwritten = false);
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    // bool checkStmt(const Decl* decl, std::string binding, ASTContext
    // &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};
class OMVAVInjector : public MVAVInjector {
  public:
    OMVAVInjector();
    std::string toString() override;
};
class WVAVInjector : public FaultInjector {
  public:
    WVAVInjector(bool alsoOverwritten = false);
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    // bool checkStmt(const Decl* decl, std::string binding, ASTContext
    // &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};

class OWVAVInjector : public WVAVInjector {
  public:
    OWVAVInjector();
    std::string toString() override;
};
class MVAEInjector : public FaultInjector {
  public:
    MVAEInjector(bool alsoOverwritten = false);
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    // bool checkStmt(const Decl* decl, std::string binding, ASTContext
    // &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};

class OMVAEInjector : public MVAEInjector {
  public:
    OMVAEInjector();
    std::string toString() override;
};
class MLPAInjector : public FaultInjector {
  public:
    MLPAInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class SMLPAInjector : public MLPAInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MIESInjector : public FaultInjector {
  public:
    MIESInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

class MRSInjector : public FaultInjector {
  public:
    MRSInjector();
    std::string toString() override;
    std::string inject(StmtBinding current, ASTContext &Context) override;
    bool checkStmt(const Stmt *stmt, std::string binding,
                   ASTContext &Context) override;
};

#endif