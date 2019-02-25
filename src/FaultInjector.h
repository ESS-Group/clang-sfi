#ifndef FAULTINJECTOR_H
#define FAULTINJECTOR_H

#include <sstream>

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"

class StmtHandler;
// include "StmtHandler.h"

using namespace clang;
using namespace clang::ast_matchers;

class FaultInjector {
  public:
    class StmtBinding {
      public:
        class Location {
          public:
            unsigned int line;
            unsigned int column;
            Location(unsigned int pLine, unsigned int pColumn) : line(pLine), column(pColumn){};
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
            Range(Location pBegin, Location pEnd) : begin(pBegin), end(pEnd), valid(true){};
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
        StmtBinding(std::string binding, const Decl *decl, bool left = false, bool isMacroExpansion = false)
            : binding(binding) {
            this->isMacroExpansion = isMacroExpansion;
            this->left = left;
            this->decl = decl;
            decllist.push_back(decl);
            isStmt = false;
            isList = false;
        }
        StmtBinding(std::string binding, const Stmt *stmt, bool left = false, bool isMacroExpansion = false)
            : binding(binding) {
            this->isMacroExpansion = isMacroExpansion;
            this->left = left;
            this->stmt = stmt;
            stmtlist.push_back(stmt);
            isStmt = true;
            isList = false;
        }
        StmtBinding(std::string binding, std::vector<const Decl *> list, bool left = false,
                    bool isMacroExpansion = false)
            : binding(binding), decllist(list.begin(), list.end()) {
            this->isMacroExpansion = isMacroExpansion;
            this->left = left;
            isStmt = false;
            isList = true;
        }
        StmtBinding(std::string binding, std::vector<const Stmt *> list, bool left = false,
                    bool isMacroExpansion = false)
            : binding(binding), stmtlist(list.begin(), list.end()) {
            this->isMacroExpansion = isMacroExpansion;
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
                            SourceLocation _begin = stmtlist[i]->getLocStart(), _end = stmtlist[i]->getLocEnd();
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
                            begin = decllist[0]->getLocStart();
                            end = decllist[0]->getLocEnd();
                        } else {
                            SourceLocation _begin = decllist[i]->getLocStart(), _end = decllist[i]->getLocEnd();
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
                    begin = stmt->getLocStart();
                    end = stmt->getLocEnd();
                } else {
                    begin = decl->getLocStart();
                    end = decl->getLocEnd();
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
        bool isMacroExpansion;
        // std::vector<int> additionalAttributesInt;
        // std::vector<Str> additionalAttributesStr;
        // std::vector<clang::FileID> injectedFiles;
    };
    StmtHandler *createStmtHandler(std::string binding);
    FaultInjector();
    ~FaultInjector();
    FaultInjector(const FaultInjector &that) = delete;

    static std::string getFileName(const Stmt *stmt, SourceManager &SM);
    static std::string getFileName(const Decl *decl, SourceManager &SM);

    void pushMacroDef(std::string binding, const Stmt *stmt, SourceManager &SM, bool left = false);
    void pushMacroDef(std::string binding, const Decl *decl, SourceManager &SM, bool left = false);
    void push(std::string binding, const Stmt *st, bool left = false, bool isMacroExpansion = false);
    void push(std::string binding, const Decl *st, bool left = false, bool isMacroExpansion = false);
    void push(std::string binding, std::vector<const Stmt *> list);
    void push(std::string binding, std::vector<const Decl *> list);
    virtual void inject(std::vector<StmtBinding> target, ASTContext &Context, bool isMacroDefinition = false);
    virtual void _inject(StmtBinding current, ASTContext &Context, int i = 0, bool isMacroDefinition = false);
    virtual bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) = 0;
    // default false
    virtual bool checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context);
    virtual bool checkStmt(const Decl *stmt, std::string binding, ASTContext &Context);
    // default behavior of checkMacroExpansion => checkStmt
    virtual bool checkMacroExpansion(const Stmt *stmt, std::string binding, ASTContext &Context);
    virtual bool checkMacroExpansion(const Decl *stmt, std::string binding, ASTContext &Context);
    // default false
    virtual bool checkMacroDefinition(const Stmt *stmt, std::string binding, ASTContext &Context);
    virtual bool checkMacroDefinition(const Decl *stmt, std::string binding, ASTContext &Context);
    bool isMacroDefinitionAdded(SourceLocation locStart, SourceManager &SM);
    void matchAST(ASTContext &Context);
    virtual std::string toString() = 0;

    std::vector<StmtBinding> locations;
    std::vector<StmtBinding> macroLocations;
    std::vector<SourceLocation> addedMacroPositions;
    bool matchMacroDefinition, matchMacroExpansion;
    void setMatchMacro(bool match);
    void setMatchMacro(bool matchDef, bool matchExp);
    // void setSourceMgr(SourceManager &sourceManager);
    // SourceManager* getSourceMgr();
    // Rewriter Rewrite;
    void nodeCallbackMacroDef(std::string binding, const Stmt *stmt, SourceManager &SM, bool left = false);
    void nodeCallbackMacroDef(std::string binding, const Decl *decl, SourceManager &SM, bool left = false);
    void nodeCallbackMacroExpansion(std::string binding, const Stmt *stmt, bool left = false);
    void nodeCallbackMacroExpansion(std::string binding, const Decl *decl, bool left = false);
    void nodeCallback(std::string binding, const Stmt *stmt, bool left = false);
    void nodeCallback(std::string binding, const Decl *decl, bool left = false);
    void nodeCallback(std::string binding, std::vector<const Stmt *> list);
    void nodeCallback(std::string binding, std::vector<const Decl *> list);
    std::string getFileName();
    void setFileName(std::string name);
    void setVerbose(bool v);
    void setDirectory(std::string directory);
    void setRootDir(std::string);
    void setFileList(std::vector<std::string> list);
    std::string rootDir = "";
    std::vector<std::string> fileList;

  protected:
    static void dumpStmt(const Stmt *stmt, ASTContext &Context);
    static std::string stmtToString(const Stmt *stmt /*, SourceManager &sourceManager*/, const LangOptions &langOpts);
    static void dumpStmt(const Decl *decl);
    static std::string stmtToString(const Decl *decl, const LangOptions &langOpts);
    static std::string sourceLocationToString(SourceLocation loc, const SourceManager &sourceManager);
    static std::string sourceRangeToString(SourceRange range, const SourceManager &sourceManager);
    static std::string sourceRangeToString(const Stmt *stmt, const SourceManager &sourceManager);
    static std::string sourceRangeToString(const Decl *decl, const SourceManager &sourceManager);
    static std::string rewriteBufferToString(RewriteBuffer &buffer);
    std::string getEditedString(Rewriter &rewrite, ASTContext &Context);
    void writeDown(std::string data, int i);
    void printStep(StmtBinding current, const SourceManager &sourceManager, const LangOptions &langOpts, int i = 0,
                   int size = 0); // with printing statements
    void printStep(StmtBinding current, const SourceManager &sourceManager, int i = 0, int size = 0); // only position
    std::string fileName;
    // SourceManager *sourceMgr;
    std::vector<std::string> bindings;
    MatchFinder Matcher; // child classes have to add Matchers
    void _sort();
    void _sortMacro(SourceManager &SM);
    static bool comparefunc(StmtBinding st1, StmtBinding st2);
    bool verbose;
    std::string dir;
};

#include "FaultInjectors/_all.h"

#endif
