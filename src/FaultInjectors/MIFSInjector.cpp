#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mifs"

std::string MIFSInjector::toString() {
    return "MIFS";
};
std::string SMIFSInjector::toString() {
    return "SMIFS";
};

MIFSInjector::MIFSInjector() { // Missing if construct plus statements
    Matcher.addMatcher(ifStmt(unless(hasElse(stmt()))).bind("ifStmt"), createMatchHandler("ifStmt"));
}

bool MIFSInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    if (current.binding.compare("ifStmt") == 0) {
        const IfStmt *ifS = cast<IfStmt>(current.stmt);

        SourceLocation start = ifS->getBeginLoc(), end = ifS->getEndLoc();
        SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
        R.RemoveText(range);
        LLVM_DEBUG(dbgs() << "MIFS: Removed range for ifStmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
    } else {
        assert(false && "Unknown binding in MIFS injector");
        std::cerr << "Unknown binding in MIFS injector" << std::endl;
    }
    return true;
}

bool MIFSInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    const IfStmt ifS = cast<IfStmt>(stmt);
    // THEN block should contain less than 5 statements.
    if (!C9(ifS.getThen(), &Context)) {
        return false;
    }
    // IF statement should not be the only statement in the block.
    return C2(stmt, Context);
}

bool SMIFSInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    const IfStmt ifS = cast<IfStmt>(stmt);
    if (!C9(ifS.getThen(), &Context, false, 5, true)) {
        return false;
    }
    return C2(stmt, Context);
}
