#include "MatchHandler.h"

#include "FaultInjector.h"

MatchHandler::MatchHandler(FaultInjector *pFaultInjector, std::string fileName, std::vector<std::string> bindings)
    : bindings(bindings), fileName(fileName) {
    faultInjector = pFaultInjector;
}

bool considerFile(FaultInjector *injector, std::string fileName) {
    if (injector->getFileName().compare(fileName) == 0) {
        return true;
    } else if (injector->rootDir.compare("") != 0 &&
               fileName.rfind(injector->rootDir, 0) == 0) { // is in source tree and rootDir is defined
        return true;
    } else if (injector->fileList.size() != 0) {
        for (std::string name : injector->fileList) {
            if (fileName.compare(name) == 0 || fileName.compare(injector->rootDir + name) == 0) {
                return true;
            }
        }
    }

    return false;
}

void MatchHandler::run(const MatchFinder::MatchResult &Result) {
    SourceManager &SM = Result.Context->getSourceManager();
    for (std::string binding : bindings) {
        if (const Stmt *stmt = dyn_cast<Stmt>(Result.Nodes.getNodeAs<Stmt>(binding))) {
            run_stmt_or_decl(Result, SM, binding, stmt);
        } else if (const Decl *decl = dyn_cast<Decl>(Result.Nodes.getNodeAs<Decl>(binding))) {
            run_stmt_or_decl(Result, SM, binding, decl);
        }
    }
}

template <typename SD>
void MatchHandler::run_stmt_or_decl(const MatchFinder::MatchResult &Result, SourceManager &SM, std::string binding,
                                   SD *stmtOrDecl) {
    if (stmtOrDecl != NULL && !SM.isInSystemHeader(stmtOrDecl->getLocStart()) && // do not match on system headers or system macros
        !SM.isInSystemMacro(stmtOrDecl->getLocStart())) {
        SourceLocation start = stmtOrDecl->getLocStart();
        bool isMacro = start.isMacroID();
        std::string name = FaultInjector::getFileName<SD>(stmtOrDecl, SM);
        if (!isMacro) {
            // Only consider nodes of the currently parsed file.
            if (considerFile(faultInjector, name)) {
                if (faultInjector->checkStmt(stmtOrDecl, binding, *Result.Context)) {
                    faultInjector->nodeCallback(binding, stmtOrDecl);
                }
            }
        } else {
            // MacroExpansion is to consider
            if (faultInjector->matchMacroExpansion && considerFile(faultInjector, name)) {
                if (faultInjector->checkMacroExpansion(stmtOrDecl, binding, *Result.Context)) {
                    faultInjector->nodeCallbackMacroExpansion(binding, stmtOrDecl);
                }
            }
            // MacroDefinition is to consider
            if (faultInjector->matchMacroDefinition &&
                considerFile(faultInjector, std::string(SM.getFilename(SM.getSpellingLoc(start))))) {
                if (faultInjector->checkMacroDefinition(stmtOrDecl, binding, *Result.Context)) {
                    if (!faultInjector->isMacroDefinitionAdded(SM.getSpellingLoc(start), SM)) {
                        faultInjector->nodeCallbackMacroDef(binding, stmtOrDecl, SM);
                    }
                }
            }
        }
    }
}
