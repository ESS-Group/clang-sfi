#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-wvav-safe"

std::string WVAVInjectorSAFE::toString() {
    return "WVAVSAFE";
};

WVAVInjectorSAFE::WVAVInjectorSAFE(bool alsoOverwritten) { // Wrong value assigned to variable
    Matcher.addMatcher(varDecl(hasAncestor(compoundStmt())).bind("varDecl"), createMatchHandler("varDecl"));
}

bool WVAVInjectorSAFE::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    Expr *val = cast<BinaryOperator>(current.stmt)->getRHS();
    SourceLocation start = val->getLocStart(),
        end = val->getLocEnd();
    SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
    if (isa<CXXBoolLiteralExpr>(val)) {
        bool value = cast<CXXBoolLiteralExpr>(val)->getValue();
        if (value) {
            R.ReplaceText(range, "false");
        } else {
            R.ReplaceText(range, "true");
        }
    } else {
        std::string text = R.getRewrittenText(range);
        R.ReplaceText(range, text + "^0xFF");
    }

    return true;
}

bool WVAVInjectorSAFE::checkStmt(const Decl &decl, std::string binding, ASTContext &Context) {
    if (binding.compare("varDecl") == 0 && isa<VarDecl>(decl)) {
        std::vector<const BinaryOperator *> list =
            getChildForFindVarAssignment(getParentCompoundStmt(&decl, Context), cast<VarDecl>(&decl), true);
        for (const BinaryOperator *op : list) {
            assert(op != NULL);
            auto lhs = op->getLHS();
            assert(lhs != NULL);
            if (isValueAssignment(op) && isInitializedBefore(cast<DeclRefExpr>(*lhs), Context)) {
                if (const ForStmt *forstmt = getParentOfType<ForStmt>(&decl, Context, 3)) {
                    assert(forstmt->getCond() != NULL);
                    assert(forstmt->getInc() != NULL);
                    if (isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl, Context)) {
                    } else {
                        nodeCallback(binding, *op);
                    }
                } else {
                    nodeCallback(binding, *op);
                }
            }
        }

        return false;
    }
    return false;
}
