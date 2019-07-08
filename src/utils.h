#include "clang/AST/AST.h"

using namespace clang;

template <class T>
void deleteFromList(std::vector<T> &src, std::vector<T> &toDelete) {
    bool deleted = false;
    for (std::vector<const BinaryOperator *>::iterator i = src.begin(); i != src.end(); deleted ? i : i++) {
        deleted = false;
        for (T c : toDelete) {
            if (*i == c) {
                i = src.erase(i);
                deleted = true;
                break;
            }
        }
    }
}

template<class T>
const CompoundStmt *getParentCompoundStmt(const T *stmtOrDecl, ASTContext &Context);

template <class T>
void concatVector(std::vector<T> &dst, std::vector<T> &src) {
    dst.insert(dst.end(), src.begin(), src.end());
}

bool isIncDecUO(const UnaryOperator *op);

bool isAssignment(const BinaryOperator *op, bool anyAssign = true);

const Stmt *IgnoreCast(const Stmt *stmt, bool ignoreImplicit = true);

bool isAssignmentOrFC(const Stmt *stmt);

bool isValue(const Stmt *stmt);

bool isValueAssignment(const BinaryOperator *op);

bool isExprAssignment(const BinaryOperator *op);

template <class T>
const T *getFirstChild(const Stmt *parent) {
    if (parent == NULL) {
        return NULL;
    }
    for (Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());
         i != e; ++i) {
        if (*i != NULL) {
            if (isa<T>(*i)) {
                return cast<const T>(*i);
            } else if (const T *ret = getFirstChild<T>(*i)) {
                return ret;
            }
        }
    }
    return NULL;
}

std::vector<const BinaryOperator *> getChildForFindInitForVar(const Stmt *parent, const VarDecl *var,
                                                              bool alsoinloop = false, bool alsoinforconstruct = true);

std::vector<const BinaryOperator *> getChildForFindVarAssignment(const Stmt *parent, const VarDecl *var,
                                                                 bool alsoinloop = true, bool alsoinforconstruct = true,
                                                                 bool pinited = false);

template <class T, class SD>
const T *getParentOfType(const SD *stmt, ASTContext &Context, int maxDepth = 3) { // MaxDepth = -1 for to the root
    if (stmt != NULL && maxDepth != 0) {
        int i = 0;
        ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
        for (auto p : list) {
            if (auto ret = dyn_cast_or_null<T>(p.get<Stmt>())) {
                return ret;
            }
            i++;
            if (maxDepth != -1 && i > maxDepth) {
                return NULL;
            }
        }
    }
    return NULL;
}

template <class T>
bool hasParentOfType(const Stmt &stmt, ASTContext &Context) {
    return getParentOfType<T>(stmt, Context, -1) != NULL;
}

template <class T>
bool hasParentOfType(const Decl &decl, ASTContext &Context) {
    return getParentOfType<T>(decl, Context, -1) != NULL;
}

const FunctionDecl *getParentFunctionDecl(const Stmt *stmt, ASTContext &Context);

bool isPartOfFunction(const Stmt &stmt, ASTContext &Context);

bool isLocal(const Stmt &stmt, ASTContext &Context);

bool isLocal(const Decl &decl, ASTContext &Context);

template <class T>
bool hasChildOfType(const Stmt *stmt) {
    if (stmt == NULL) {
        return false;
    }

    for (Stmt::child_iterator i = cast_away_const(stmt->child_begin()), e = cast_away_const(stmt->child_end()); i != e;
         ++i) {
        if (isa<T>(*i)) {
            return true;
        } else if (hasChildOfType<T>(*i)) {
            return true;
        }
    }
    return false;
}

bool isParentOf(const Stmt *parent, const Stmt &stmt, ASTContext &Context);

bool isParentOf(const Stmt &parent, const Stmt &stmt, ASTContext &Context);

bool isParentOf(const Stmt *parent, const Decl &decl, ASTContext &Context);

bool isParentOf(const Stmt &parent, const Decl &decl, ASTContext &Context);

bool isInitializedBefore(const DeclRefExpr &ref, ASTContext &Context);

template <class T>
std::vector<const T *> getChildrenOfType(const Stmt &parent, bool first = true) {
    std::vector<const BinaryOperator *> ret;
    if (isa<T>(parent) && first) {
        ret.push_back(cast<const T>(&parent));
    }
    for (Stmt::child_iterator i = cast_away_const(parent.child_begin()), e = cast_away_const(parent.child_end());
         i != e; ++i) {
        if (isa<T>(*i)) {
            ret.push_back(cast<T>(*i));
        }
        std::vector<const T *> children = getChildrenOfType<T>(**i, false);
        if (children.size() != 0) {
            concatVector<const T *>(ret, children);
        }
    }
    return ret;
}

std::vector<const DeclRefExpr *> getAllRefs(const Stmt &parent, const VarDecl &var);

const DeclRefExpr *getLatestRef(const Stmt &parent, const VarDecl &var);

template <class T>
bool hasStmtOfType(std::vector<const Stmt *> list) {
    for (const Stmt *stmt : list) {
        if (isa<T>(stmt)) {
            return true;
        }
    }
    return false;
}

template <class T>
std::vector<const T *> getStmtsOfType(std::vector<const Stmt *> &list) {
    std::vector<const T *> ret;
    if (list.empty()) {
        return ret;
    }
    for (const Stmt *stmt : list) {
        if (stmt != NULL && isa<T>(stmt)) {
            ret.push_back(cast<T>(stmt));
        }
    }

    return ret;
}

template <class T>
bool _comparefunc(const T *st1, const T *st2) {
    return st1->getBeginLoc() < st2->getBeginLoc();
}

bool isArithmetic(const BinaryOperator &op);

const BinaryOperator &getBinaryOperatorWithRightedtRHS(const BinaryOperator &op);

template <class T>
std::vector<const T *> getChildrenFlat(const Stmt *parent) {
    std::vector<const T *> ret;
    for (Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());
         i != e; ++i) {
        if (*i != NULL) {
            if (isa<Expr>(*i)) {
                const Expr *expr = cast<Expr>(*i)->IgnoreImplicit()->IgnoreParenCasts();
                if (isa<T>(expr)) {
                    ret.push_back(cast<T>(expr));
                }
            } else if (isa<T>(*i)) {
                ret.push_back(cast<T>(*i));
            }
        }
    }
    return ret;
}

const DeclRefExpr *getDeclRefExprOfImplicitConstructExpr(const MaterializeTemporaryExpr *matexpr);

template <class T>
std::vector<const T *> getArgumentsOfType(const CallExpr *call) {
    std::vector<const T *> ret;
    const Expr *const *args = call->getArgs();
    for (int i = 0; i < call->getNumArgs(); i++) {
        const Expr *arg = args[i];
        if (isa<MaterializeTemporaryExpr>(arg)) {
            if (const DeclRefExpr *ref = getDeclRefExprOfImplicitConstructExpr(cast<MaterializeTemporaryExpr>(arg))) {
                if (isa<T>(ref)) {
                    ret.push_back(ref);
                }
            }
        }
        if (arg->IgnoreImpCasts() != NULL && arg->IgnoreImpCasts()->IgnoreParenCasts() != NULL &&
            isa<T>(arg->IgnoreImpCasts()->IgnoreParenCasts())) {
            ret.push_back(cast<const T>(arg->IgnoreImpCasts()->IgnoreParenCasts()));
        }
    }
    return ret;
}

bool isVisible(const Decl &decl, const Stmt &position, ASTContext &Context);
