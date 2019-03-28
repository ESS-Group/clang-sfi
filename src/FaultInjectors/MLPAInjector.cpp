#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mlpa"

std::string SMLPAInjector::toString() {
    return "SMLPA";
};
std::string MLPAInjector::toString() {
    return "MLPA";
};

std::vector<std::vector<const Stmt *>> getStmtLists(const CompoundStmt &block, ASTContext &Context,
                                                    bool returnIsAJump = RETURNISAJUMP,
                                                    bool noDeclStmt = DONOTDELETEDECLSTMTINCONSTRAINT) {
    std::vector<std::vector<const Stmt *>> ret;
    int index = -1;
    for (Stmt::child_iterator i = cast_away_const(block.child_begin()), e = cast_away_const(block.child_end()); i != e;
         ++i) {
        if ((*i != NULL) && isa<Stmt>(*i)) {
            if (index == -1) {
                std::vector<const Stmt *> list;
                ret.push_back(list);
                index = 0;
            }
            if ((noDeclStmt || !isa<DeclStmt>(*i)) && !isa<IfStmt>(*i) && !isa<ForStmt>(*i) && !isa<WhileStmt>(*i) &&
                !isa<DoStmt>(*i) && !isa<SwitchStmt>(*i) && (!returnIsAJump || !isa<ReturnStmt>(*i)) &&
                !isa<BreakStmt>(*i) && !isa<ContinueStmt>(*i)) { // SwitchCase
                if (isa<CompoundStmt>(*i)) {
                    if (ret[index].size() > 0) {
                        std::vector<const Stmt *> list;
                        ret.push_back(list);
                        index++;
                    }

                    // TODO: maybe include the statements inside the
                    // CompoundStmt
                } else {
                    ret.at(index).push_back(*i);
                }
            } else if (ret[index].size() > 0) {
                std::vector<const Stmt *> list;
                ret.push_back(list);
                index++;
            }
        }
    }

    if (index <= 0 && (ret.size() == 0 || ret[0].size() == 0)) {
        std::vector<std::vector<const Stmt *>> ret;
        return ret;
    }

    std::vector<std::vector<const Stmt *>>::reverse_iterator rit = ret.rbegin();

    bool deleteIt = false;

    for (; rit != ret.rend(); /*++rit*/) {
        std::vector<const Stmt *> list = *rit;
        std::vector<const DeclStmt *> declstmts = getStmtsOfType<DeclStmt>(list);
        std::vector<const DeclStmt *> notPossible;

        for (const DeclStmt *declstmt : declstmts) { // calculate if declstatements in list cannot be
                                                     // removed, because its latest reference is outside
                                                     // this list

            std::vector<const DeclRefExpr *> ref;
            for (auto decl : declstmt->decls()) {
                if (!isa<VarDecl>(decl)) {
                    continue;
                }
                const DeclRefExpr *latest = getLatestRef(block, cast<VarDecl>(*decl));

                if (ref.size()) {
                    ref.clear();
                    ref.push_back(latest);
                } else {
                    ref.push_back(latest);
                }
            }

            if (ref.size()) {
                for (const DeclRefExpr *reference : ref) {
                    if (reference != NULL && list.back()->getEndLoc() < reference->getBeginLoc() &&
                        std::find(list.begin(), list.end(), declstmt) != list.end()) {
                        const DeclStmt *statement = cast<DeclStmt>(declstmt);

                        notPossible.push_back(statement);
                    }

                    break;
                }
            }
        }

        if (notPossible.size() != 0) {
            deleteIt = true;
            std::vector<std::vector<const Stmt *>> changed;
            std::vector<const Stmt *> temp(list.begin(), list.end());
            changed.push_back(temp);

            std::sort(notPossible.begin(), notPossible.end(), _comparefunc<DeclStmt>);

            int ritpos = std::distance(ret.rbegin(), rit) + changed.size();

            ret.insert(ret.end(), changed.begin(), changed.end()); // invalidates iterator
            rit = ret.rbegin();
            for (int i = 0; i < ritpos; i++, rit++)
                ;
            list = *rit;
        }

        if (deleteIt) {
            rit->clear();
            deleteIt = false;

            ++rit;
        } else {
            ++rit;
        }
    }

    // TODO: delete all empty lists??

    return ret;
}

bool isMLPAListPossible(std::vector<const Stmt *> stmtlist, const CompoundStmt &block) {
    std::vector<const DeclStmt *> declstmts = getStmtsOfType<DeclStmt>(stmtlist);
    std::vector<const DeclStmt *> notPossible;

    for (const DeclStmt *declstmt : declstmts) { // calculate if declstatements in list cannot be removed,
                                                 // because its latest reference is outside this list
        std::vector<const DeclRefExpr *> ref;
        for (auto decl : declstmt->decls()) {
            if (!isa<VarDecl>(decl)) {
                continue;
            }
            const DeclRefExpr *latest = getLatestRef(block, cast<VarDecl>(*decl));
            if (ref.size()) {
                for (const DeclRefExpr *x : ref) {
                    if (x->getBeginLoc() < latest->getBeginLoc()) {
                        ref.clear();
                        ref.push_back(latest);
                        break;
                    }
                }
            } else {
                ref.push_back(latest);
            }
        }
        for (const DeclRefExpr *reference : ref) {
            if (reference != NULL && stmtlist.back()->getEndLoc() < reference->getBeginLoc() &&
                std::find(stmtlist.begin(), stmtlist.end(), declstmt) != stmtlist.end()) {
                const DeclStmt *statement = cast<const DeclStmt>(declstmt);
                notPossible.push_back(statement);
            }
        }
    }
    return notPossible.size() == 0;
}

std::vector<std::vector<const Stmt *>> getMLPAListOfSize(std::vector<const Stmt *> stmtlist, int size,
                                                         const CompoundStmt &block) {
    std::vector<std::vector<const Stmt *>> ret;
    int listsize = stmtlist.size();
    for (int begin = 0; begin + size <= listsize; begin++) {
        std::vector<const Stmt *> list(stmtlist.begin() + begin, stmtlist.begin() + begin + size);
        if (isMLPAListPossible(list, block)) {
            ret.push_back(list);
        }
    }
    return ret;
}

// clang-format off
MLPAInjector::MLPAInjector() { // Missing small and localized part of the algorithm
    Matcher.addMatcher(
                compoundStmt(
                    allOf(
                        unless(hasParent(declStmt())),
                        unless(hasParent(switchStmt()))
                    )
                ).bind("compoundStmt"), 
                createMatchHandler("compoundStmt")
        );

}
// clang-format on

bool MLPAInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.binding.compare("compoundStmt") == 0) {
        std::vector<const Stmt *> list = current.stmtlist;
        SourceLocation start = list[0]->getBeginLoc(), end = list[0]->getEndLoc();

        for (const Stmt *stmt : list) {
            if (stmt->getBeginLoc() < start) {
                start = stmt->getBeginLoc();
            }
            if (end < stmt->getEndLoc()) {
                end = stmt->getEndLoc();
            }
        }

        SourceRange range(start, end);
        R.RemoveText(range);
        LLVM_DEBUG(dbgs() << "MLPA: Removed range for compoundStmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
    } else {
        assert(false && "Unknown binding in MLPA injector");
        std::cerr << "Unknown binding in MLPA injector" << std::endl;
    }
    return true;
}

bool MLPAInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    const CompoundStmt &compoundStmt = cast<CompoundStmt>(stmt);
    std::vector<std::vector<const Stmt *>> stmtlists = getStmtLists(compoundStmt, Context);
    for (std::vector<const Stmt *> it : stmtlists) {
        if (it.size() >= 2) {
            int size = it.size();
            if (size == compoundStmt.size()) { // because 1 statement must remain
                                               // within the compoundStmt
                size--;
            }

            if (size > MAXSTATEMENTNUMFORCONSTRAINT) { // maximum size constraint
                size = MAXSTATEMENTNUMFORCONSTRAINT;
            }

            for (; size >= 2; size--) {
                std::vector<std::vector<const Stmt *>> injectionpoints =
                    getMLPAListOfSize(it, size, compoundStmt); // create SubList of size

                for (std::vector<const Stmt *> injectionpoint : injectionpoints) {
                    nodeCallback(binding, injectionpoint);
                }
            }
        }
    }

    return false;
}

bool SMLPAInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    const CompoundStmt &compoundStmt = cast<CompoundStmt>(stmt);
    std::vector<std::vector<const Stmt *>> stmtlists = getStmtLists(compoundStmt, Context);

    for (std::vector<const Stmt *> it : stmtlists) {
        if (it.size() >= 2) {
            int size = it.size();
            if (size == compoundStmt.size()) // because 1 statement must remain
                                             // within the compoundStmt
                size--;
            if (size > 5) {
                size = 5;
            }

            for (; size >= 2; size--) {
                std::vector<std::vector<const Stmt *>> injectionpoints = getMLPAListOfSize(it, size, compoundStmt);
                for (std::vector<const Stmt *> injectionpoint : injectionpoints) {
                    nodeCallback(binding, injectionpoint);
                }
            }
        }
    }

    return false;
}
