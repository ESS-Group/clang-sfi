#include "StmtHandler.h"
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include <algorithm>
#include <vector>

#include <fstream>
#include <iostream>

#include "llvm/Support/raw_ostream.h"
using namespace llvm;
// MIFS
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

constexpr int MAXSTATEMENTNUMFORCONSTRAINT = 5;
constexpr bool DONOTDELETEDECLSTMTINCONSTRAINT = false;
constexpr bool RETURNISAJUMP = true;
constexpr bool DONTCOUNTBREAKINSWITCHCASEFORBLOCKSIZE = true;

bool isaJumpStmt(const Stmt *stmt, bool returnIsAJump = true);

bool C9(const clang::Stmt::const_child_iterator &begin,
        const clang::Stmt::const_child_iterator &end,
        ASTContext *Context = NULL, bool returnIsAJump = RETURNISAJUMP,
        int maxNum = MAXSTATEMENTNUMFORCONSTRAINT,
        bool noDeclStmt = DONOTDELETEDECLSTMTINCONSTRAINT);
bool C9(const Stmt *stmt, ASTContext *Context = NULL,
        bool returnIsAJump = RETURNISAJUMP,
        int maxNum = MAXSTATEMENTNUMFORCONSTRAINT,
        bool noDeclStmt = DONOTDELETEDECLSTMTINCONSTRAINT);
bool C8(const IfStmt *ifS);

bool isaImplicit(const Stmt *stmt);

const Stmt *getParentIgnoringImplicit(const Stmt *stmt, ASTContext &Context);

const SwitchStmt *getParentSwitchStmt(const SwitchCase *sc,
                                      ASTContext &Context);
struct CaseChilds {
    std::vector<const Stmt *> stmts;
    bool endWithBreak;
};
CaseChilds getCaseChilds(const SwitchCase *sc, ASTContext &Context);
int childCount(const Stmt *stmt);
int childCount(const Stmt *stmt, ASTContext &Context);
int childCount(const Stmt *stmt);

bool C2(const Stmt *stmt, ASTContext &Context);

bool C2(const Decl *decl, ASTContext &Context);
