#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mrs"

std::string MRSInjector::toString() {
    return "MRS";
};

MRSInjector::MRSInjector() { // Missing if construct plus statements plus else
                             // plus statements
    Matcher.addMatcher(returnStmt().bind("returnStmt"), createMatchHandler("returnStmt"));
}

bool MRSInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    if (current.binding.compare("returnStmt")) {
        const Stmt *stmt = current.stmt;

        SourceLocation start = stmt->getLocStart(), end = stmt->getLocEnd();
        SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
        R.RemoveText(range);
        LLVM_DEBUG(dbgs() << "MRS: Removed range for returnStmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
    } else {
        assert(false && "Unknown binding in MRS injector");
        std::cerr << "Unknown binding in MRS injector" << std::endl;
    }
    return true;
}
bool MRSInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    return true;
}
