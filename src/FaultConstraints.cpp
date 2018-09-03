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
bool isaJumpStmt(const Stmt *stmt, bool returnIsAJump = true) {
    if (stmt == NULL)
        return false;
    else if (isa<ForStmt>(stmt) || isa<WhileStmt>(stmt) ||
             isa<DoStmt>(stmt)) // loops
        return true;
    else if (isa<IfStmt>(stmt) || isa<SwitchStmt>(stmt)) // conditional
        return true;
    else if (isa<ContinueStmt>(stmt) || isa<BreakStmt>(stmt) ||
             isa<CXXThrowExpr>(stmt) || isa<CXXTryStmt>(stmt) ||
             isa<GotoStmt>(stmt))
        return true;
    else if (returnIsAJump && isa<ReturnStmt>(stmt))
        return true;
    return false;
}

bool C9(const clang::Stmt::const_child_iterator &begin,
        const clang::Stmt::const_child_iterator &end,
        ASTContext *Context = NULL, bool returnIsAJump = RETURNISAJUMP,
        int maxNum = MAXSTATEMENTNUMFORCONSTRAINT,
        bool noDeclStmt = DONOTDELETEDECLSTMTINCONSTRAINT) {
    StmtIterator it = cast_away_const(begin);
    int num = 0;
    while (it != cast_away_const(end)) {
        if (isaJumpStmt(*it, returnIsAJump) ||
            (noDeclStmt && *it != NULL && isa<DeclStmt>(*it))) { // other jumps
            return false;
        }
        num++;
        it++;
    }
    if (num <= /*5*/ maxNum)
        return true;
    else
        return false;
}
bool C9(const Stmt *stmt, ASTContext *Context = NULL,
        bool returnIsAJump = RETURNISAJUMP,
        int maxNum = MAXSTATEMENTNUMFORCONSTRAINT,
        bool noDeclStmt = DONOTDELETEDECLSTMTINCONSTRAINT) {

    if (isa<CompoundStmt>(stmt))
        return C9(stmt->child_begin(), stmt->child_end(), Context,
                  returnIsAJump, maxNum, noDeclStmt);
    else {
        return stmt == NULL || (!isaJumpStmt(stmt, returnIsAJump) &&
                                !(noDeclStmt && isa<DeclStmt>(stmt)));
    }
}
bool C8(const IfStmt *ifS) {
    if (const Stmt *Else = ifS->getElse())
        return false;
    else
        return true;
}

bool isaImplicit(const Stmt *stmt) {
    return isa<ExprWithCleanups>(stmt) || isa<MaterializeTemporaryExpr>(stmt) ||
           isa<CXXBindTemporaryExpr>(stmt) || isa<ImplicitCastExpr>(stmt);
}

const Stmt *getParentIgnoringImplicit(const Stmt *stmt, ASTContext &Context) {
    ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
    if (!list.empty()) {
        if (list[0].get<Stmt>() != NULL) {

            const Stmt *temp = list[0].get<Stmt>();
            if (isaImplicit(temp)) {
                return getParentIgnoringImplicit(temp, Context);
            } else {
                return temp;
            }
        } else
            return NULL;
    } else
        return NULL;
}

const SwitchStmt *getParentSwitchStmt(const SwitchCase *sc,
                                      ASTContext &Context) {
    if (sc == NULL)
        return NULL;
    const Stmt *parent = getParentIgnoringImplicit(sc, Context);
    if (parent == NULL) {
        return NULL;
    } else if (isa<SwitchStmt>(parent)) {
        return (const SwitchStmt *)parent;
    } else { // if(isa<SwitchCase>(parent)){
        return getParentSwitchStmt((const SwitchCase *)parent, Context);
    }
}
struct CaseChilds {
    std::vector<const Stmt *> stmts;
    bool endWithBreak;
};
CaseChilds getCaseChilds(const SwitchCase *sc, ASTContext &Context) {
    CaseChilds ret;
    ret.endWithBreak = false;
    const Stmt *begin = sc;

    const SwitchStmt *switchStmt = getParentSwitchStmt(sc, Context);
    if (switchStmt != NULL) {
        const Stmt *body = switchStmt->getBody();
        if (body == NULL)
            return ret;
        else if (isa<CompoundStmt>(body)) {
            bool started = false;
            for (Stmt::child_iterator i = cast_away_const(body->child_begin()),
                                      e = cast_away_const(body->child_end());
                 i != e; ++i) {
                if (*i == NULL) {
                    break;
                } else if (isa<SwitchCase>(*i) && *i == begin) {
                    started = true;
                    const Stmt *stmt = ((const SwitchCase *)*i)->getSubStmt();
                    if (stmt == NULL) {
                        break;
                    } else if (isa<BreakStmt>(stmt)) {
                        ret.stmts.push_back(stmt);
                        ret.endWithBreak = true;
                        break;
                    } else if (isa<SwitchCase>(stmt)) {

                        break;
                    } else {
                        ret.stmts.push_back(stmt);
                    }
                } else if (isa<SwitchCase>(*i) && started) {
                    break;
                } else if (isa<SwitchCase>(*i)) {
                    SwitchCase *tcase = (SwitchCase *)&(*(*i));
                    while (tcase != NULL && tcase != begin &&
                           tcase->getSubStmt() != NULL &&
                           isa<SwitchCase>(tcase->getSubStmt()))
                        tcase = (SwitchCase *)tcase->getSubStmt();
                    if (tcase == begin) {
                        started = true;
                        const Stmt *stmt =
                            ((const SwitchCase *)&(*tcase))->getSubStmt();
                        if (stmt == NULL) {
                            break;
                        } else if (isa<BreakStmt>(stmt)) {
                            ret.stmts.push_back(stmt);
                            ret.endWithBreak = true;
                            break;
                        } else if (isa<SwitchCase>(stmt)) {

                            break;
                        } else {

                            ret.stmts.push_back(stmt);
                        }
                    }

                } else if (started) {
                    ret.stmts.push_back(*i);
                    if (isa<BreakStmt>(*i)) {
                        ret.endWithBreak = true;
                        break;
                    }
                }
            }
            return ret;
        } else if (isa<SwitchCase>(body)) {
            const Stmt *stmt = ((const SwitchCase *)body)->getSubStmt();
            if (stmt != NULL && !isa<SwitchCase>(stmt)) // otherwise case 1:case
                                                        // 2:break; would end in
                                                        // "case 2" being seen
                                                        // as child
                ret.stmts.push_back(stmt);
        }
    }
    return ret;
}
int childCount(const Stmt *stmt);
int childCount(const Stmt *stmt, ASTContext &Context) {

    int count = 0;
    if (isa<SwitchCase>(stmt)) {
        CaseChilds childs = getCaseChilds((const SwitchCase *)stmt, Context);
        int count = childs.stmts.size();
#if DONTCOUNTBREAKINSWITCHCASEFORBLOCKSIZE
        if (childs.endWithBreak)
            count--;
#endif
        return count;
    } else
        return childCount(stmt);
    return count;
}
int childCount(const Stmt *stmt) {
    int count = 0;
    const clang::Stmt::const_child_iterator &begin = stmt->child_begin();
    const clang::Stmt::const_child_iterator &end = stmt->child_end();
    StmtIterator it = cast_away_const(begin);
    while (it != cast_away_const(end)) {
        if (*it != NULL)
            count++;
        it++;
    }
    return count;
}

bool C2(const Stmt *stmt, ASTContext &Context) {
    const Stmt *parent = getParentIgnoringImplicit(stmt, Context);
    if (parent != NULL) {
        if (isa<CompoundStmt>(parent)) {
            const CompoundStmt *container = (const CompoundStmt *)parent;
            return container->size() > 1;
        } else if (isa<SwitchCase>(parent)) {
            const SwitchCase *container = (const SwitchCase *)parent;
            return childCount(container, Context) > 1;
        } else
            return false;
    }
    return false;
}

bool C2(const Decl *decl, ASTContext &Context) {
    ASTContext::DynTypedNodeList list = Context.getParents(*decl);
    while (!list.empty() && list[0].get<Stmt>() != NULL &&
           isaImplicit(list[0].get<Stmt>()))
        list = Context.getParents(*list[0].get<Stmt>());
    if (!list.empty()) {
        if (list[0].get<Stmt>() == NULL) {
            return false;
        }
        if (isa<CompoundStmt>(list[0].get<Stmt>())) {
            const CompoundStmt *container = list[0].get<CompoundStmt>();
            return childCount(container, Context) > 1;
        } else if (isa<DeclStmt>(list[0].get<Stmt>())) {
            return C2(list[0].get<Stmt>(), Context);
        }
        return false;
    }
    return false;
}