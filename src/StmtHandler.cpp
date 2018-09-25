#include "StmtHandler.h"

#include "FaultInjector.h"

StmtHandler::StmtHandler(FaultInjector *pFaultInjector, std::string fileName,
                         std::vector<std::string> bindings /*,void (*nodeCallback)(std::string, const Stmt*)*/)
    : /*nodeCallback(nodeCallback),*/ bindings(bindings), fileName(fileName) {
    faultInjector = pFaultInjector;
}
void StmtHandler::run(const MatchFinder::MatchResult &Result) {
    for (std::string binding : bindings) {
        if (const Stmt *stmt = Result.Nodes.getNodeAs<clang::Stmt>(binding)) {
            std::string name(Result.Context->getSourceManager().getFilename(stmt->getLocStart()));
            if (faultInjector->getFileName().compare(name) == 0) { // Only consider nodes of the currently parsed file.
                if (faultInjector->checkStmt(stmt, binding, *Result.Context)) {
                    faultInjector->nodeCallback(binding, stmt);
                }
            }
        } else if (const Decl *stmt = Result.Nodes.getNodeAs<clang::Decl>(binding)) {
            std::string name(Result.Context->getSourceManager().getFilename(stmt->getLocStart()));
            if (faultInjector->getFileName().compare(name) == 0) { // Only consider nodes of the currently parsed file.
                if (faultInjector->checkStmt(stmt, binding, *Result.Context)) {
                    faultInjector->nodeCallback(binding, stmt);
                }
            }
        }
    }
}
