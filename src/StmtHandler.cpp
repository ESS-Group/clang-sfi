#include "StmtHandler.h"

#include "FaultInjector.h"

StmtHandler::StmtHandler(FaultInjector *pFaultInjector, std::string fileName,
                         std::vector<std::string> bindings /*,void (*nodeCallback)(std::string, const Stmt*)*/)
    : /*nodeCallback(nodeCallback),*/ bindings(bindings), fileName(fileName) {
    faultInjector = pFaultInjector;
}
bool considerFile(FaultInjector *injector, std::string fileName) {
    if (injector->getFileName().compare(fileName) == 0)
        return true;
    else if (injector->considerFile != NULL && injector->considerFile(fileName)) {
        return true;
    } else if (injector->fileList != NULL) {
        for (std::string name : *(injector->fileList)) {
            if (name.compare(fileName) == 0) {
                return true;
            }
        }
    }

    return false;
}

void StmtHandler::run(const MatchFinder::MatchResult &Result) {
    SourceManager *SM = &Result.Context->getSourceManager();
    for (std::string binding : bindings) {
        if (const Stmt *stmt = Result.Nodes.getNodeAs<clang::Stmt>(binding)) {
            if (stmt != NULL) {
                SourceLocation start = stmt->getLocStart();
                bool isMacro = start.isMacroID();
                std::string name = FaultInjector::getFileName(stmt, *SM);
                if (!isMacro) {
                    // Only consider nodes of the currently parsed file.
                    if (considerFile(faultInjector, name)) {
                        if (faultInjector->checkStmt(stmt, binding, *Result.Context)) {
                            faultInjector->nodeCallback(binding, stmt);
                        }
                    }
                } else {
                    // MacroExpansion is to consider
                    if (considerFile(faultInjector, name) && faultInjector->matchMacroExpansion) {
                        if (faultInjector->checkMacroExpansion(stmt, binding, *Result.Context)) {
                            faultInjector->nodeCallbackMacroExpansion(binding, stmt);
                        }
                    }
                    // MacroDefinition is to consider
                    if (considerFile(faultInjector, std::string(SM->getFilename(SM->getSpellingLoc(start)))) &&
                        faultInjector->matchMacroDefinition) {
                        // SourceRange range(SM->getSpellingLoc(start), SM->getSpellingLoc(stmt->getLocEnd()));
                        if (faultInjector->checkMacroDefinition(stmt, binding, *Result.Context)) {
                            if (!faultInjector->isMacroDefinitionAdded(SM->getSpellingLoc(start), *SM)) {
                                faultInjector->nodeCallbackMacroDef(binding, stmt, *SM);
                            }
                        }
                    }
                }
            }

        } else if (const Decl *stmt = Result.Nodes.getNodeAs<clang::Decl>(binding)) {
            if (stmt != NULL) {
                SourceLocation start = stmt->getLocStart();
                bool isMacro = start.isMacroID();
                std::string name = FaultInjector::getFileName(stmt, *SM);
                if (!isMacro) {
                    // Only consider nodes of the currently parsed file.
                    if (considerFile(faultInjector, name)) {
                        if (faultInjector->checkStmt(stmt, binding, *Result.Context)) {
                            faultInjector->nodeCallback(binding, stmt);
                        }
                    }
                } else {
                    // MacroExpansion is to consider
                    if (considerFile(faultInjector, name) && faultInjector->matchMacroExpansion) {
                        if (faultInjector->checkMacroExpansion(stmt, binding, *Result.Context)) {
                            faultInjector->nodeCallbackMacroExpansion(binding, stmt);
                        }
                    }
                    // MacroDefinition is to consider
                    if (considerFile(faultInjector, std::string(SM->getFilename(SM->getSpellingLoc(start)))) &&
                        faultInjector->matchMacroDefinition) {
                        // SourceRange range(SM->getSpellingLoc(start), SM->getSpellingLoc(stmt->getLocEnd()));
                        if (faultInjector->checkMacroDefinition(stmt, binding, *Result.Context)) {
                            if (!faultInjector->isMacroDefinitionAdded(SM->getSpellingLoc(start), *SM)) {
                                faultInjector->nodeCallbackMacroDef(binding, stmt, *SM);
                            }
                        }
                    }
                }
            }
        }
    }
}
