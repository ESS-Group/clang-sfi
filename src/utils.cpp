#include "utils.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceLocation.h"

using namespace clang::ast_matchers;

template <class T>
const T *getParentOfType(const Stmt *stmt, ASTContext &Context) {
    ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
    if (!list.empty()) {
        if (list[0].get<Stmt>() != NULL) {
            if (isa<T>(list[0].get<Stmt>())) {
                const T *ret = list[0].get<T>();
                return ret;
            } else {
                return NULL;
            }
        }
    }
    return NULL;
}

template<class T>
const CompoundStmt *getParentCompoundStmt(const T *stmtOrDecl, ASTContext &Context) {
    ASTContext::DynTypedNodeList list = Context.getParents(*stmtOrDecl);

    if (!list.empty()) {
        if (list[0].get<Stmt>() != NULL) {
            if (isa<CompoundStmt>(list[0].get<Stmt>())) {
                const CompoundStmt *container = list[0].get<CompoundStmt>();
                return container;
            } else if (isa<DeclStmt>(list[0].get<Stmt>())) {
                return getParentCompoundStmt(list[0].get<Stmt>(), Context);
            }
        }
    }
    return NULL;
}

bool isIncDecUO(const UnaryOperator *op) {
    if (op == NULL) {
        return false;
    }

    switch (op->getOpcode()) {
    case UO_PostInc:
    case UO_PostDec:
    case UO_PreInc:
    case UO_PreDec:
        return true;
    default:
        return false;
    }
}

bool isAssignment(const BinaryOperator *op, bool anyAssign) {
    if (op == NULL) {
        return false;
    }

    bool ret = op->getOpcode() == BinaryOperatorKind::BO_Assign;
    if (!ret && anyAssign) {
        switch (op->getOpcode()) {
        case BO_MulAssign:
        case BO_DivAssign:
        case BO_RemAssign:
        case BO_AddAssign:
        case BO_SubAssign:
        case BO_ShlAssign:
        case BO_ShrAssign:
        case BO_AndAssign:
        case BO_XorAssign:
        case BO_OrAssign:
            return true;
        default:
            return false;
        }
    }
    return ret;
}

const Stmt *IgnoreCast(const Stmt *stmt, bool ignoreImplicit) {
    if (stmt == NULL) {
        return NULL;
    }

    if (ignoreImplicit) {
        const Stmt *temp = stmt->IgnoreImplicit();
        if (auto castExpr = dyn_cast_or_null<CastExpr>(temp)) {
            if (const Stmt *_stmt = IgnoreCast(castExpr->getSubExpr())) {
                return _stmt->IgnoreImplicit();
            } else {
                return NULL;
            }
        } else {
            return stmt;
        }
    } else if (auto castExpr = dyn_cast<CastExpr>(stmt)) {
        if (const Stmt *_stmt = IgnoreCast(castExpr->getSubExpr())) {
            return _stmt;
        } else {
            return NULL;
        }
    } else {
        return stmt;
    }
}

bool isAssignmentOrFC(const Stmt *stmt) {
    if (stmt == NULL) {
        return false;
    }

    const Stmt *_stmt = IgnoreCast(stmt, true);
    if (_stmt == NULL) {
        return false;
    } else if (isa<CallExpr>(_stmt) || isa<CXXMemberCallExpr>(_stmt)) {
        return true;
    } else if (isa<BinaryOperator>(_stmt) && isAssignment(cast<BinaryOperator>(_stmt), true)) {
        return true;
    } else if (isa<UnaryOperator>(_stmt) && isIncDecUO(cast<UnaryOperator>(_stmt))) {
        return true;
    }
    return false;
}

bool isValue(const Stmt *stmt) {
    return stmt != NULL && (isa<IntegerLiteral>(stmt) || isa<CXXBoolLiteralExpr>(stmt) || isa<CharacterLiteral>(stmt) ||
                            isa<FloatingLiteral>(stmt) || isa<clang::StringLiteral>(stmt));
}

bool isValueAssignment(const BinaryOperator *op) {
    if (isAssignment(op)) {
        const Stmt *stmt = op->getRHS();
        if (stmt != NULL && isValue(stmt)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool isExprAssignment(const BinaryOperator *op) {
    return isAssignment(op) && !isValueAssignment(op);
}

std::vector<const BinaryOperator *> getChildForFindInitForVar(const Stmt *parent, const VarDecl *var, bool alsoinloop,
                                                              bool alsoinforconstruct) {
    std::vector<const BinaryOperator *> ret;
    if (parent == NULL) {
        return ret;
    }
    if (var == NULL) {
        return ret;
    }
    if (isa<BinaryOperator>(parent)) {
        if (isAssignment(cast<const BinaryOperator>(parent))) {
            if (const DeclRefExpr *exp = getFirstChild<DeclRefExpr>(parent)) {
                if (exp->getDecl() == var) {
                    ret.push_back(cast<BinaryOperator>(parent));
                }
            }
        }
    } else {
        for (Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());
             i != e; ++i) {
            if (auto binOp = dyn_cast_or_null<BinaryOperator>(*i)) {
                if (isAssignment(binOp)) {
                    if (const DeclRefExpr *exp = getFirstChild<DeclRefExpr>(*i)) {
                        if (exp->getDecl() == var) {
                            ret.push_back(cast<BinaryOperator>(*i));
                            break;
                        }
                    }
                }
            } else if (dyn_cast_or_null<Stmt>(*i)) {
                if (IfStmt *ifS = dyn_cast<IfStmt>(*i)) {
                    std::vector<const BinaryOperator *> inThen = getChildForFindInitForVar(ifS->getThen(), var);
                    if (inThen.size() != 0) {
                        concatVector<const BinaryOperator *>(ret, inThen);
                    }
                    if (const Stmt *elseS = ifS->getElse()) {
                        std::vector<const BinaryOperator *> inElse = getChildForFindInitForVar(elseS, var);
                        if (inElse.size() != 0) {
                            concatVector<const BinaryOperator *>(ret, inElse);
                            if (inThen.size() != 0) {
                                break; // initialization in both
                            }
                        }
                    }
                } else if (alsoinloop || (!isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i))) {
                    if (isa<ForStmt>(*i)) {
                        std::vector<const BinaryOperator *> temp;
                        if (alsoinforconstruct) {
                            auto init = cast<ForStmt>(*i)->getInit();
                            temp = getChildForFindInitForVar(init, var, alsoinloop, alsoinforconstruct);
                            if (temp.size() != 0) {
                                concatVector<const BinaryOperator *>(ret, temp);
                                break;
                            }
                        }
                    }

                    std::vector<const BinaryOperator *> temp = getChildForFindInitForVar(*i, var), inloop;
                    if (temp.size() != 0) {
                        concatVector<const BinaryOperator *>(ret, temp);
                        break;
                    }
                } else if (auto forStmt = dyn_cast_or_null<ForStmt>(*i)) {
                    std::vector<const BinaryOperator *> temp;
                    if (alsoinforconstruct) {
                        auto init = forStmt->getInit();
                        temp = getChildForFindInitForVar(init, var, alsoinloop, alsoinforconstruct);
                        if (temp.size() != 0) {
                            concatVector<const BinaryOperator *>(ret, temp);
                            break;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

std::vector<const BinaryOperator *> getChildForFindVarAssignment(const Stmt *parent, const VarDecl *var,
                                                                 bool alsoinloop, bool alsoinforconstruct,
                                                                 bool pinited) {
    std::vector<const BinaryOperator *> ret;
    if (var == NULL) {
        return ret;
    }
    if (parent == NULL) {
        return ret;
    }

    bool inited = pinited || var->hasInit();
    if (auto binOp = dyn_cast<const BinaryOperator>(parent)) {
        if (isAssignment(binOp)) {
            if (const DeclRefExpr *exp = getFirstChild<DeclRefExpr>(binOp->getLHS())) {
                if (exp->getDecl() == var) {
                    if (inited) {
                        ret.push_back(binOp);
                    } else {
                        inited = true;
                    }
                }
            }
        }
    } else {
        for (Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());
             i != e; ++i) {
            if (auto binOp = dyn_cast_or_null<BinaryOperator>(*i)) {
                if (isAssignment(binOp)) {
                    if (const DeclRefExpr *exp = getFirstChild<DeclRefExpr>(*i)) {
                        if (exp->getDecl() == var) {
                            if (inited) {
                                ret.push_back(binOp);
                            } else {
                                inited = true;
                            }
                        }
                    }
                }
            } else if (dyn_cast_or_null<Stmt>(*i)) {
                if (IfStmt *ifS = dyn_cast<IfStmt>(*i)) {
                    std::vector<const BinaryOperator *> inThen =
                        getChildForFindVarAssignment(ifS->getThen(), var, alsoinloop, alsoinforconstruct, inited);
                    if (inThen.size() != 0) {
                        concatVector<const BinaryOperator *>(ret, inThen);
                        inited = true;
                    }
                    if (const Stmt *elseS = ifS->getElse()) {
                        std::vector<const BinaryOperator *> inElse =
                            getChildForFindVarAssignment(elseS, var, alsoinloop, alsoinforconstruct, inited);
                        if (inElse.size() != 0) {
                            concatVector<const BinaryOperator *>(ret, inElse);
                            inited = true;
                        }
                    }
                } else if (alsoinloop) {
                    if (isa<ForStmt>(*i)) {
                        std::vector<const BinaryOperator *> temp;
                        if (alsoinforconstruct) {
                            temp = getChildForFindVarAssignment(cast<const ForStmt>(*i)->getInit(), var, alsoinloop,
                                                                alsoinforconstruct, inited);
                            if (temp.size() != 0) {
                                concatVector<const BinaryOperator *>(ret, temp);
                                inited = true;
                            }
                            temp = getChildForFindVarAssignment(cast<const ForStmt>(*i)->getInc(), var, alsoinloop,
                                                                alsoinforconstruct, inited);
                            if (temp.size() != 0) {
                                concatVector<const BinaryOperator *>(ret, temp);
                                inited = true;
                            }
                        }
                        temp = getChildForFindVarAssignment(cast<const ForStmt>(*i)->getBody(), var, alsoinloop,
                                                            alsoinforconstruct, true);

                        if (temp.size() != 0) {
                            concatVector<const BinaryOperator *>(ret, temp);
                            inited = true;
                        }
                    } else if (isa<WhileStmt>(*i)) {
                        std::vector<const BinaryOperator *> temp = getChildForFindVarAssignment(
                            cast<const WhileStmt>(*i)->getBody(), var, alsoinloop, alsoinforconstruct, true);
                        if (temp.size() != 0) {
                            concatVector<const BinaryOperator *>(ret, temp);
                            inited = true;
                        }
                    } else if (isa<DoStmt>(*i)) {
                        std::vector<const BinaryOperator *> temp = getChildForFindVarAssignment(
                            cast<const DoStmt>(*i)->getBody(), var, alsoinloop, alsoinforconstruct, true);
                        if (temp.size() != 0) {
                            concatVector<const BinaryOperator *>(ret, temp);
                            inited = true;
                        }
                    } else {
                        std::vector<const BinaryOperator *> temp =
                            getChildForFindVarAssignment(*i, var, alsoinloop, alsoinforconstruct, inited);
                        if (temp.size() != 0) {
                            concatVector<const BinaryOperator *>(ret, temp);
                            inited = true;
                        }
                    }
                } else if (!isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i)) {
                    std::vector<const BinaryOperator *> temp =
                        getChildForFindVarAssignment(*i, var, alsoinloop, alsoinforconstruct, inited);
                    if (temp.size() != 0) {
                        concatVector<const BinaryOperator *>(ret, temp);
                        inited = true;
                    }
                }
            }
        }
    }
    return ret;
}

const FunctionDecl *getParentFunctionDecl(const Stmt *stmt, ASTContext &Context) {
    if (stmt == NULL) {
        return NULL;
    }

    const DeclStmt *ret = getParentOfType<DeclStmt>(stmt, Context, -1);
    if (ret == NULL) {
        return NULL;
    } else if (ret->isSingleDecl()) {
        if (isa<FunctionDecl>(ret->getSingleDecl())) {
            return cast<const FunctionDecl>(ret->getSingleDecl());
        } else {
            return getParentFunctionDecl(ret, Context);
        }
    } else {
        return NULL;
    }
}

bool isPartOfFunction(const Stmt *stmt, ASTContext &Context) {
    if (stmt == NULL) {
        return false;
    }

    const FunctionDecl *decl = getParentFunctionDecl(stmt, Context);
    return decl != NULL;
}

bool isLocal(const Stmt *stmt, ASTContext &Context) {
    if (stmt == NULL) {
        return false;
    }

    return isPartOfFunction(stmt, Context);
}

bool isLocal(const Decl *decl, ASTContext &Context) {
    if (decl == NULL) {
        return false;
    }

    return isLocal(getParentOfType<DeclStmt>(decl, Context), Context);
}

bool isParentOf(const Stmt *parent, const Stmt &stmt) {
    if (parent == NULL) {
        return false;
    }

    return isParentOf(*parent, stmt);
}

bool isParentOf(const Stmt &parent, const Stmt &stmt) {
    for (Stmt::child_iterator i = cast_away_const(parent.child_begin()), e = cast_away_const(parent.child_end());
         i != e; ++i) {
        if (cast<const Stmt>(*i) == &stmt) {
            return true;
        }
        if (isParentOf(**i, stmt)) {
            return true;
        }
    }
    return false;
}

bool isParentOf(const Stmt *parent, const Decl &decl, ASTContext &Context) {
    const DeclStmt *stmt = getParentOfType<DeclStmt>(&decl, Context, 3);
    if (stmt == NULL) {
        return false;
    }
    return isParentOf(parent, *stmt);
}

bool isParentOf(const Stmt &parent, const Decl &decl, ASTContext &Context) {
    const DeclStmt *stmt = getParentOfType<DeclStmt>(&decl, Context, 3);
    if (stmt == NULL) {
        return false;
    }
    return isParentOf(parent, *stmt);
}

bool isInitializedBefore(const DeclRefExpr &ref, ASTContext &Context) {
    const VarDecl *decl = cast<const VarDecl>(ref.getDecl());
    if (decl->getInit() != NULL) { // if declaration is initialization => every
                                   // use after that is an assignment
        return true;
    } else {
        const CompoundStmt *parent = getParentCompoundStmt(decl, Context);
        std::vector<const BinaryOperator *> inits = getChildForFindInitForVar(parent, decl, false);
        for (const BinaryOperator *init : inits) { // else check if ref is used in initialization
            if (init->getLHS() == &ref) {
                return false;
            }
        }
        return true;
    }
}

std::vector<const DeclRefExpr *> getAllRefs(const Stmt &parent, const VarDecl &var) {
    std::vector<const DeclRefExpr *> ret;
    for (Stmt::child_iterator i = cast_away_const(parent.child_begin()), e = cast_away_const(parent.child_end());
         i != e; ++i) {
        if ((*i != NULL) && isa<Stmt>(*i)) {
            if (isa<DeclRefExpr>(*i)) {
                if (cast<const DeclRefExpr>(*i)->getDecl() == &var)
                    ret.push_back(cast<const DeclRefExpr>(*i));
            } else {
                std::vector<const DeclRefExpr *> list = getAllRefs(**i, var);
                if (list.size() != 0) {
                    concatVector<const DeclRefExpr *>(ret, list);
                }
            }
        }
    }

    return ret;
}

const DeclRefExpr *getLatestRef(const Stmt &parent, const VarDecl &var) {
    std::vector<const DeclRefExpr *> refs = getAllRefs(parent, var);

    std::vector<const DeclRefExpr *> ret;
    for (const DeclRefExpr *ref : refs) {
        if (ret.size()) {
            for (const DeclRefExpr *reference : ret) {
                if (reference->getEndLoc() < ref->getEndLoc()) {
                    ret.clear();
                    ret.push_back(ref);
                }
            }
        } else {
            ret.push_back(ref);
        }
    }

    for (const DeclRefExpr *ref : ret) {
        return ref;
    }
    return NULL;
}

bool isArithmetic(const BinaryOperator &op) {
    int code = op.getOpcode();
    return code == BinaryOperatorKind::BO_Mul || code == BinaryOperatorKind::BO_Div ||
           code == BinaryOperatorKind::BO_Rem || code == BinaryOperatorKind::BO_Add ||
           code == BinaryOperatorKind::BO_Sub || code == BinaryOperatorKind::BO_Shl ||
           code == BinaryOperatorKind::BO_Shr || code == BinaryOperatorKind::BO_And ||
           code == BinaryOperatorKind::BO_Or || code == BinaryOperatorKind::BO_Xor;
}

const BinaryOperator &getBinaryOperatorWithRightedtRHS(const BinaryOperator &op) {
    if (isa<BinaryOperator>(op.getRHS()) && isArithmetic(op)) {
        return getBinaryOperatorWithRightedtRHS(cast<const BinaryOperator>(*op.getRHS()));
    } else {
        return op;
    }
}

const DeclRefExpr *getDeclRefExprOfImplicitConstructExpr(const MaterializeTemporaryExpr *matexpr) {
    if (matexpr != NULL && matexpr->getTemporary() != NULL && isa<CXXBindTemporaryExpr>(matexpr->getTemporary())) {
        const CXXBindTemporaryExpr *tempbind = cast<const CXXBindTemporaryExpr>(matexpr->getTemporary());
        if (isa<CXXConstructExpr>(tempbind->getSubExpr())) {
            const CXXConstructExpr *constructexpr = cast<const CXXConstructExpr>(tempbind->getSubExpr());
            if (constructexpr->getNumArgs() == 1 && constructexpr->getArg(0)->IgnoreImplicit() != NULL &&
                isa<DeclRefExpr>(constructexpr->getArg(0)->IgnoreImplicit())) {
                const DeclRefExpr *ref = cast<const DeclRefExpr>(constructexpr->getArg(0)->IgnoreImplicit());
                return ref;
            }
        }
    }
    return NULL;
}

bool isVisible(const Decl &decl, const Stmt &position, ASTContext &Context) {
    const DeclStmt *declstmt = getParentOfType<DeclStmt>(&decl, Context);

    if (declstmt == NULL) {
        return false;
    }
    ASTContext::DynTypedNodeList list = Context.getParents(*declstmt);
    if (!list.empty()) {
        const Stmt *parent = list[0].get<Stmt>();

        if (parent == NULL) {
            return false;
        }

        if (isParentOf(*parent, position) && decl.getEndLoc() < position.getBeginLoc()) {
            return true;
        }
    }
    return false;
}
