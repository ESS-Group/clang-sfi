#include "_all.h"

std::string SMLPAInjector::toString() {
    return "SMLPA";
};
std::string MLPAInjector::toString() {
    return "MLPA";
};

std::vector<std::vector<const Stmt *>> getStmtLists(const CompoundStmt *block, ASTContext &Context,
                                                    bool returnIsAJump = RETURNISAJUMP,
                                                    bool noDeclStmt = DONOTDELETEDECLSTMTINCONSTRAINT) {
    std::vector<std::vector<const Stmt *>> ret;
    int index = -1;
    for (Stmt::child_iterator i = cast_away_const(block->child_begin()), e = cast_away_const(block->child_end());
         i != e; ++i) {
        if (*i == NULL) {
        } else if (isa<Stmt>(*i)) {
            /*if(Context.getFullLoc(i->getLocStart()).getLineNumber()==965){
                i->getLocStart().dump(Context.getSourceManager());
                cerr<<endl;
                i->dumpColor();
            }*/
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
                const DeclRefExpr *latest = getLatestRef(block, (const VarDecl *)decl);

                if (ref.size()) {
                    ref.clear();
                    ref.push_back(latest);
                } else {
                    ref.push_back(latest);
                }
            }

            if (ref.size() /*ref!=NULL*/) {
                for (const DeclRefExpr *reference : ref) {
                    if (reference != NULL && list.back()->getLocEnd() < reference->getLocStart() &&
                        std::find(list.begin(), list.end(), declstmt) != list.end()) {
                        const DeclStmt *statement = (const DeclStmt *)declstmt; // declStmt;

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
    // ret;

    return ret;
}

bool isMLPAListPossible(std::vector<const Stmt *> stmtlist, const CompoundStmt *block) {
    std::vector<const DeclStmt *> declstmts = getStmtsOfType<DeclStmt>(stmtlist);
    std::vector<const DeclStmt *> notPossible;

    for (const DeclStmt *declstmt : declstmts) { // calculate if declstatements in list cannot be removed,
                                                 // because its latest reference is outside this list
        std::vector<const DeclRefExpr *> ref;
        for (auto decl : declstmt->decls()) {
            const DeclRefExpr *latest = getLatestRef(block, (const VarDecl *)decl);
            if (ref.size()) {
                for (const DeclRefExpr *x : ref) {
                    if (x->getLocStart() < latest->getLocStart()) {
                        ref.clear();
                        ref.push_back(latest);
                        break;
                    }
                }
            } else {
                ref.push_back(latest);
            }
            /*if(ref==NULL)
                ref = latest;
            else if(ref->getLocStart()<latest->getLocStart())
                ref = latest;
            */
        }
        for (const DeclRefExpr *reference : ref) {
            if (reference != NULL && stmtlist.back()->getLocEnd() < reference->getLocStart() &&
                std::find(stmtlist.begin(), stmtlist.end(), declstmt) != stmtlist.end()) {
                const DeclStmt *statement = (const DeclStmt *)declstmt; // declStmt;
                notPossible.push_back(statement);
            }
        }
    }
    return notPossible.size() == 0;
}

std::vector<std::vector<const Stmt *>> getMLPAListOfSize(std::vector<const Stmt *> stmtlist, int size,
                                                         const CompoundStmt *block) {
    std::vector<std::vector<const Stmt *>> ret;
    int listsize = stmtlist.size();
    for (int begin = 0; begin + size <= listsize; begin++) {
        std::vector<const Stmt *> list(stmtlist.begin() + begin, stmtlist.begin() + begin + size);
        // cout<<"list:"<<list.size()<<endl;
        if (isMLPAListPossible(list, block))
            ret.push_back(list);
    }
    return ret;
}

// clang-format off
MLPAInjector::MLPAInjector() {
    Matcher.addMatcher(
                compoundStmt(
                    allOf(
                        unless(hasParent(declStmt())),
                        unless(hasParent(switchStmt()))
                    )
                ).bind("compoundStmt"), 
                createStmtHandler("compoundStmt")
        );

}
// clang-format on

std::string MLPAInjector::inject(StmtBinding current, ASTContext &Context) {
    std::vector<const Stmt *> list = current.stmtlist;
    SourceLocation begin = list[0]->getLocStart(), end = list[0]->getLocEnd();

    for (const Stmt *stmt : list) {
        if (stmt->getLocStart() < begin) {
            begin = stmt->getLocStart();
        }
        if (end < stmt->getLocEnd()) {
            end = stmt->getLocEnd();
        }
    }

    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(begin, end);
    R.RemoveText(range);
    return getEditedString(R, Context);
    /*    Rewriter R;
        R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
        if(current.isStmt){
            SourceRange range(current.stmt->getLocStart(),
       current.stmt->getLocEnd());
            R.RemoveText(range);
        } else {
            VarDecl temp (*((const VarDecl*)current.decl));
            temp.setInit(NULL);
            const VarDecl* tempP = &temp;
            std::string withoutInit = stmtToString(tempP,
       Context.getLangOpts());


            SourceRange range(current.decl->getLocStart(),
       current.decl->getLocEnd());
            R.ReplaceText(range, withoutInit);
        }

        return getEditedString(R, Context);*/
}

bool MLPAInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const CompoundStmt *compoundStmt = cast<CompoundStmt>(stmt);
    std::vector<std::vector<const Stmt *>> stmtlists = getStmtLists(compoundStmt, Context);
    // int s = stmt->size();
    /*
    if(compoundStmt->size()>2){
        stmt->dumpColor();
        cerr << "=================" << endl;
        for(std::vector<const Stmt*> it:stmtlists){

                        cerr<<"--sublist:"<<it.size()<<endl;
                        */
    /*
                        for(const Stmt * stmt:it){
                            stmt->dumpColor();
                        }
                        cerr<<"--endlist"<<endl;
                        */ /*
}
}
return false;
*/
    for (std::vector<const Stmt *> it : stmtlists) {
        if (it.size() >= 2) {
            // if(isMLPAListPossible(it,compoundStmt))
            // nodeCallback(binding, it);continue;

            int size = it.size();
            if (size == compoundStmt->size()) { // because 1 statement must remain
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

bool SMLPAInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    const CompoundStmt *compoundStmt = cast<CompoundStmt>(stmt);
    std::vector<std::vector<const Stmt *>> stmtlists = getStmtLists(compoundStmt, Context);

    for (std::vector<const Stmt *> it : stmtlists) {
        if (it.size() >= 2) {
            int size = it.size();
            if (size == compoundStmt->size()) // because 1 statement must remain
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
