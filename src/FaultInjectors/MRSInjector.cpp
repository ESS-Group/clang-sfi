#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mrs"

std::string MRSInjector::toString() {
    return "MRS";
};

MRSInjector::MRSInjector() { // Missing if construct plus statements plus else
                             // plus statements
    Matcher.addMatcher(returnStmt().bind("returnStmt"), createMatchHandler("returnStmt"));
}

bool MRSInjector::inject(StmtBinding current, ASTContext &Context, GenericRewriter &R) {
    if (current.binding.compare("returnStmt")) {
        const Stmt *stmt = current.stmt;

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
