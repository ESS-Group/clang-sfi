#ifndef FAULTINJECTOR_H
#define FAULTINJECTOR_H

#include <sstream>

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Frontend/CompilerInstance.h"

#include "GenericRewriter.h"

class MatchHandler;

using namespace clang;
using namespace clang::ast_matchers;

class FaultInjector : public MatchFinder::MatchCallback {
  public:
    class StmtBinding {
      public:
        class Location {
          public:
            unsigned int line;
            unsigned int column;
            Location(unsigned int pLine, unsigned int pColumn) : line(pLine), column(pColumn) {};
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
            Range(Location pBegin, Location pEnd) : begin(pBegin), end(pEnd), valid(true) {};
            Range() : valid(false){};
            bool isValid() {
                return valid;
            };
            std::string toString() {
                if (isValid()) {
                    std::stringstream ss;
                    ss << begin.toString() << " - " << end.toString() << std::endl;
                    return ss.str();
                } else {
                    return "INVALID";
                }
            }

          private:
            bool valid;
        };
        StmtBinding(std::string binding, const Decl &decl, bool left = false)
            : binding(binding) {
            this->left = left;
            this->decl = &decl;
            decllist.push_back(&decl);
            isStmt = false;
            isList = false;
        }
        StmtBinding(std::string binding, const Stmt &stmt, bool left = false)
            : binding(binding) {
            this->left = left;
            this->stmt = &stmt;
            stmtlist.push_back(&stmt);
            isStmt = true;
            isList = false;
        }
        StmtBinding(std::string binding, std::vector<const Decl *> list, bool left = false)
            : binding(binding), decllist(list.begin(), list.end()) {
            this->left = left;
            isStmt = false;
            isList = true;
        }
        StmtBinding(std::string binding, std::vector<const Stmt *> list, bool left = false)
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
                            begin = stmtlist[0]->getBeginLoc();
                            end = stmtlist[0]->getEndLoc();
                        } else {
                            SourceLocation _begin = stmtlist[i]->getBeginLoc(), _end = stmtlist[i]->getEndLoc();
                            if (end < _end) {
                                end = _end;
                            }
                            if (_begin < begin) {
                                begin = _begin;
                            }
                        }
                    }
                } else {
                    for (int i = 0; i < decllist.size(); i++) {
                        if (i == 0) {
                            begin = decllist[0]->getBeginLoc();
                            end = decllist[0]->getEndLoc();
                        } else {
                            SourceLocation _begin = decllist[i]->getBeginLoc(), _end = decllist[i]->getEndLoc();
                            if (end < _end) {
                                end = _end;
                            }
                            if (_begin < begin) {
                                begin = _begin;
                            }
                        }
                    }
                }
            } else {
                if (isStmt) {
                    begin = stmt->getBeginLoc();
                    end = stmt->getEndLoc();
                } else {
                    begin = decl->getBeginLoc();
                    end = decl->getEndLoc();
                }
            }
            if (begin.isValid() && end.isValid()) {
                FullSourceLoc fBegin = Context.getFullLoc(begin), fEnd = Context.getFullLoc(end);
                location = Range(Location(fBegin.getLineNumber(), fBegin.getColumnNumber()),
                                 Location(fEnd.getLineNumber(), fEnd.getColumnNumber()));
            }
        }
        const void *get() {
            if (isList) {
                if (isStmt) {
                    return &stmtlist;
                } else {
                    return &decllist;
                }
            } else {
                if (isStmt) {
                    return stmt;
                } else {
                    return decl;
                }
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
    FaultInjector *createMatchHandler(std::string binding);
    FaultInjector();
    ~FaultInjector();
    FaultInjector(const FaultInjector &that) = delete;

    virtual void run(const MatchFinder::MatchResult &Result);
    template <typename SD>
    void run_stmt_or_decl(const MatchFinder::MatchResult &Result, SourceManager &SM, std::string binding, SD &stmtOrDecl);

    template<class T>
    void push(std::string binding, const T &stmtOrDecl, bool left = false);
    template<class T>
    void push(std::string binding, std::vector<const T *> list);
    /// Perform injections for a list of StmtBindings.
    virtual void inject(std::vector<StmtBinding> target, ASTContext &Context);
    /// Perform an injection for a specific StmtBinding in the provided Rewriter.
    /// \return True if the rewriting was successful.
    virtual bool inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) = 0;
    /// Generate a patch for a single StmtBinding.
    virtual void generatePatchFile(StmtBinding current, ASTContext &Context, GenericRewriter &R, int i = 0);
    // default false
    virtual bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context);
    virtual bool checkStmt(const Decl &stmt, std::string binding, ASTContext &Context);
    void matchAST(ASTContext &Context);
    virtual std::string toString() = 0;

    std::vector<StmtBinding> locations;
    bool matchMacroDefinition, matchMacroExpansion;
    void setMatchMacro(bool match);
    void setMatchMacro(bool matchDef, bool matchExp);
    void nodeCallback(std::string binding, const Stmt &stmt, bool left = false);
    void nodeCallback(std::string binding, const Decl &decl, bool left = false);
    void nodeCallback(std::string binding, std::vector<const Stmt *> list);
    void nodeCallback(std::string binding, std::vector<const Decl *> list);

    void setCI(CompilerInstance *CI);
    void setVerbose(bool v);
    void setDirectory(std::string directory);
    void setFileName(std::string name);
    void setRootDir(std::string);
    void setFileList(std::vector<std::string> list);

  protected:
    static void dumpStmt(const Stmt *stmt, ASTContext &Context);
    static std::string stmtToString(const Stmt *stmt, const LangOptions &langOpts);
    static void dumpStmt(const Decl *decl);
    static std::string stmtToString(const Decl *decl, const LangOptions &langOpts);
    static std::string sourceLocationToString(SourceLocation loc, const SourceManager &sourceManager);
    static std::string sourceRangeToString(SourceRange range, const SourceManager &sourceManager);
    static std::string sourceRangeToString(const Stmt *stmt, const SourceManager &sourceManager);
    static std::string sourceRangeToString(const Decl *decl, const SourceManager &sourceManager);
    static std::string rewriteBufferToString(RewriteBuffer &buffer);
    std::string getEditedString(Rewriter &rewrite, ASTContext &Context);
    void printStep(StmtBinding current, const SourceManager &sourceManager, const LangOptions &langOpts, int i = 0,
                   int size = 0); // with printing statements
    void printStep(StmtBinding current, const SourceManager &sourceManager, int i = 0, int size = 0); // only position
    MatchFinder Matcher; // child classes have to add Matchers
    void _sort();
    static bool comparefunc(StmtBinding st1, StmtBinding st2);

    bool verbose;
    std::string dir;
    CompilerInstance *CI;
    std::string fileName;
    std::string rootDir = "";
    std::vector<std::string> fileList;
    std::vector<std::string> bindings;
};

#include "FaultInjectors/_all.h"

#endif
