#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mia"

std::string MIAInjector::toString() {
    return "MIA";
};
std::string SMIAInjector::toString() {
    return "SMIA";
};

MIAInjector::MIAInjector() { // Missing if construct around statements
    Matcher.addMatcher(ifStmt(unless(hasElse(stmt()))).bind("ifStmt"), createMatchHandler("ifStmt"));
}

bool MIAInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    if (current.binding.compare("ifStmt") == 0) {
        const IfStmt *ifS = cast<IfStmt>(current.stmt);

        SourceLocation start = ifS->getBeginLoc(), end = ifS->getThen()->getBeginLoc().getLocWithOffset(-1);
        SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
        R.RemoveText(range);
        LLVM_DEBUG(dbgs() << "MIA: Removed range for ifStmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
    } else {
        assert(false && "Unknown binding in MIA injector");
        std::cerr << "Unknown binding in MIA injector" << std::endl;
    }
    return true;
}
bool MIAInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    const IfStmt ifS = cast<IfStmt>(stmt);
    return C9(ifS.getThen(), &Context);
}

bool SMIAInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    const IfStmt ifS = cast<IfStmt>(stmt);
    return C9(ifS.getThen(), &Context, false, 5, true); // && C2(ifS,Context);
}
