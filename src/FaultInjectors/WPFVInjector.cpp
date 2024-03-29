#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-wpfv"

std::string WPFVInjector::toString() {
    return "WPFV";
};

// clang-format off
WPFVInjector::WPFVInjector() { // Wrong variable used in parameter of function call
    Matcher.addMatcher(
        callExpr(
            allOf(
                unless(anyOf(
                    cudaKernelCallExpr(),
                    cxxOperatorCallExpr(),
                    userDefinedLiteral()
                )),
                hasAnyArgument(
                    anyOf(
                        ignoringImplicit(ignoringParenCasts(declRefExpr())),
                        ignoringImplicit(
                                cxxConstructExpr(
                                    hasArgument(0, ignoringImplicit(declRefExpr()))
                                )   
                            )
                        
                    )
                )
                    //implicit constructorcall + implicit cast
                )
            ).bind("functionCall"),
        createMatchHandler("functionCall")
    );
}
// clang-format on

bool WPFVInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.binding.compare("functionCall") == 0) {
        const DeclRefExpr *stmt = cast<DeclRefExpr>(current.stmt);
        assert(stmt != NULL);

        const VarDecl *arg = cast<VarDecl>(stmt->getDecl());
        const DeclContext *declContext = arg->getDeclContext();
        const FunctionDecl *fkt = cast<FunctionDecl>(declContext->getNonClosureAncestor());

        bool isParam = true;

        std::vector<const VarDecl *> localVariables;
        int paramCount = fkt->getNumParams();
        for (int i = 0; i < paramCount; i++) {
            const VarDecl *param = fkt->getParamDecl(i);
            if (param != arg &&
                param->getType().getNonReferenceType().getDesugaredType(Context) ==
                    arg->getType().getNonReferenceType().getDesugaredType(Context) &&
                localVariables.size() == 0) {
                localVariables.push_back(param);
            } else if (param == arg) {
                isParam = true;
            }
        }

        if (!isParam || localVariables.size() == 0) {
            for (DeclContext::decl_iterator it = declContext->decls_begin(), e = declContext->decls_end(); it != e;
                 ++it) {
                const VarDecl *vardecl = cast<VarDecl>(*it);
                assert(vardecl != NULL);
                if (isVisible(*vardecl, *stmt, Context) && vardecl != arg &&
                    arg->getType().getNonReferenceType().getDesugaredType(Context) ==
                        vardecl->getType().getNonReferenceType().getDesugaredType(Context)) {
                    if (!isParam) {
                        localVariables.clear();
                    }
                    localVariables.push_back(vardecl);
                    break;
                }
            }
        }
        std::string variableName(localVariables[0]->getName().data());

        SourceRange range = stmt->getSourceRange();
        SourceLocation start = range.getBegin(), end = range.getEnd();
        SourceRange expandedRange(start, end);
        LLVM_DEBUG(dbgs() << "WPFV: Replaced range for functionCall"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << " with " << variableName << "\n");
        return R.ReplaceText(expandedRange, variableName);
    } else {
        assert(false && "Unknown binding in WPFV injector");
        std::cerr << "Unknown binding in WPFV injector" << std::endl;
    }
    return false;
}

bool WPFVInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    for (auto i : getArgumentsOfType<DeclRefExpr>(cast<CallExpr>(&stmt))) {
        const ValueDecl *arg = cast<ValueDecl>(i->getDecl());
        const DeclContext *declContext = arg->getDeclContext();
        int varcount = 0;
        auto parent = getParentOfType<DeclStmt>(arg, Context);
        const FunctionDecl *fkt = NULL;
        if (parent != NULL) {
            fkt = getParentFunctionDecl(parent, Context);
        }
        if (fkt == NULL) {
            const FunctionDecl *_fkt = dyn_cast_or_null<FunctionDecl>(declContext->getNonClosureAncestor());
            if (_fkt != NULL) {
                int paramCount = _fkt->getNumParams();

                for (int i = 0; i < paramCount; i++) {
                    const VarDecl *param = _fkt->getParamDecl(i);
                    if (param != arg &&
                        param->getType().getNonReferenceType().getDesugaredType(Context) ==
                            arg->getType().getNonReferenceType().getDesugaredType(Context)) {
                        varcount++;
                        break;
                    }
                }

                for (DeclContext::decl_iterator it = _fkt->decls_begin(), e = _fkt->decls_end(); it != e; ++it) {
                    const VarDecl *vardecl = dyn_cast_or_null<VarDecl>(*it);
                    if (vardecl != NULL && vardecl != arg &&
                        vardecl->getType().getNonReferenceType().getDesugaredType(Context) ==
                            arg->getType().getNonReferenceType().getDesugaredType(Context)) {
                        assert(vardecl != NULL);
                        if (isVisible(*vardecl, *i, Context)) {
                            varcount++;
                            break;
                        }
                    }
                }

                if (varcount >= 1) {
                    assert(i != NULL);
                    nodeCallback(binding, *i);
                }
            }
        } else if (fkt != NULL) {
            int paramCount = fkt->getNumParams();

            for (int i = 0; i < paramCount; i++) {
                const VarDecl *param = fkt->getParamDecl(i);
                if (param != arg && param->getType() == arg->getType()) {
                    varcount++;
                    break;
                }
            }

            for (DeclContext::decl_iterator it = fkt->decls_begin(), e = fkt->decls_end(); it != e; ++it) {
                const VarDecl *vardecl = dyn_cast_or_null<VarDecl>(*it);
                if (vardecl != NULL && vardecl != arg &&
                    vardecl->getType().getNonReferenceType().getDesugaredType(Context) ==
                        arg->getType().getNonReferenceType().getDesugaredType(Context)) {
                    assert(vardecl != NULL);
                    if (isVisible(*vardecl, *i, Context)) {
                        varcount++;
                        break;
                    }
                }
            }

            if (varcount >= 1) {
                assert(i != NULL);
                nodeCallback(binding, *i);
            }
        } else { // variable is not local
        }
    }
    return false;
}
