#include "_all.h"
#define DEBUG_TYPE "clang-sfi-injector-mies"

std::string MIESInjector::toString() {
    return "MIES";
};

MIESInjector::MIESInjector() { // Missing if construct plus statements plus else
                               // plus statements
    Matcher.addMatcher(ifStmt(hasElse(stmt())).bind("ifStmt"), createMatchHandler("ifStmt"));
}

bool MIESInjector::inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) {
    if (current.binding.compare("ifStmt")) {
        const IfStmt *ifS = cast<IfStmt>(current.stmt);

        SourceLocation start = ifS->getLocStart(), end = ifS->getElse()->getLocEnd();
        SourceRange range(R.getSourceMgr().getExpansionLoc(start), R.getSourceMgr().getExpansionLoc(end));
        R.RemoveText(range);
        LLVM_DEBUG(dbgs() << "MIES: Removed range for ifStmt"
                          << "\n"
                          << range.getBegin().printToString(R.getSourceMgr()) << "\n"
                          << range.getEnd().printToString(R.getSourceMgr()) << "\n");
    } else {
        assert(false && "Unknown binding in MIES injector");
        std::cerr << "Unknown binding in MIES injector" << std::endl;
    }
    return true;
}

bool MIESInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    const IfStmt ifS = cast<IfStmt>(stmt);
    // IF and ELSE block should not contain more than 5 statements.
    if (!C9(ifS.getThen())) {
        return false;
    }
    if (!C9(ifS.getElse())) {
        return false;
    }
    // IF statement should not be the only statement in the block.
    return C2(stmt, Context);
}
