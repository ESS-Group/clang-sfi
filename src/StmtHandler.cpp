#include "StmtHandler.h"

#include "FaultInjector.h"

StmtHandler::StmtHandler(FaultInjector *pFaultInjector, std::string fileName, std::vector<std::string> bindings)
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

void StmtHandler::run(const MatchFinder::MatchResult &Result) {
    SourceManager &SM = Result.Context->getSourceManager();
    for (std::string binding : bindings) {
        run_stmt_or_decl(Result, SM, binding, Result.Nodes.getNodeAs<clang::Stmt>(binding));
        run_stmt_or_decl(Result, SM, binding, Result.Nodes.getNodeAs<clang::Decl>(binding));
    }
}

template <typename SD>
void StmtHandler::run_stmt_or_decl(const MatchFinder::MatchResult &Result, SourceManager &SM, std::string binding,
                                   SD *stmt) {
    if (stmt != NULL && !SM.isInSystemHeader(stmt->getLocStart()) && // do not match on system headers or system macros
        !SM.isInSystemMacro(stmt->getLocStart())) {
        SourceLocation start = stmt->getLocStart();
        bool isMacro = start.isMacroID();
        std::string name = FaultInjector::getFileName(stmt, SM);
        if (!isMacro) {
            // Only consider nodes of the currently parsed file.
            if (considerFile(faultInjector, name)) {
                if (faultInjector->checkStmt(stmt, binding, *Result.Context)) {
                    faultInjector->nodeCallback(binding, stmt);
                }
            }
        } else {
            // MacroExpansion is to consider
            if (faultInjector->matchMacroExpansion && considerFile(faultInjector, name)) {
                if (faultInjector->checkMacroExpansion(stmt, binding, *Result.Context)) {
                    faultInjector->nodeCallbackMacroExpansion(binding, stmt);
                }
            }
            // MacroDefinition is to consider
            if (faultInjector->matchMacroDefinition &&
                considerFile(faultInjector, std::string(SM.getFilename(SM.getSpellingLoc(start))))) {
                if (faultInjector->checkMacroDefinition(stmt, binding, *Result.Context)) {
                    if (!faultInjector->isMacroDefinitionAdded(SM.getSpellingLoc(start), SM)) {
                        faultInjector->nodeCallbackMacroDef(binding, stmt, SM);
                    }
                }
            }
        }
    }
}
