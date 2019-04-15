#include "_all.h"
#include "clang/Lex/Lexer.h"

#define DEBUG_TYPE "clang-sfi-injector-mrs"

std::string MRSInjector::toString() {
    return "MRS";
};

MRSInjector::MRSInjector() { // Missing if construct plus statements plus else
                             // plus statements
    Matcher.addMatcher(returnStmt().bind("returnStmt"), createMatchHandler("returnStmt"));
}

bool MRSInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.binding.compare("returnStmt") == 0) {
        const Stmt *stmt = current.stmt;
        LLVM_DEBUG(dbgs() << "MRS: matched statement\n");

        Token TheTok;
        if (Lexer::getRawToken(stmt->getBeginLoc(), TheTok, Context.getSourceManager(), Context.getLangOpts())) {
            LLVM_DEBUG(dbgs() << "getRawToken failed\n");
            return false;
        } else if (TheTok.getRawIdentifier().compare("return") != 0) {
            LLVM_DEBUG(dbgs() << TheTok.getName() << " does not look like an explicit return stmt; we do not care\n");
            return false;
        }

        SourceLocation start = stmt->getBeginLoc(), end = stmt->getEndLoc();
        SourceRange range(start, end);
        LLVM_DEBUG(dbgs() << "MRS: Removed range for returnStmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
        return R.RemoveText(range);
    } else {
        assert(false && "Unknown binding in MRS injector");
        std::cerr << "Unknown binding in MRS injector" << std::endl;
    }
    return false;
}
bool MRSInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    return true;
}
