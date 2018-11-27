/*
//ASTContext &Context
        //Rewriter rw;
        //rw.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
*/
#include "FaultInjector.h"

#include <fstream>

#include "StmtHandler.h"

using namespace clang;
using namespace clang::ast_matchers;

StmtHandler *FaultInjector::createStmtHandler(std::string binding) {
    std::vector<std::string> bindings;
    bindings.push_back(binding);
    return new StmtHandler(this, fileName, bindings);
}

bool FaultInjector::checkStmt(const Stmt *stmt, std::string binding, ASTContext &Context) {
    return false;
}

bool FaultInjector::checkStmt(const Decl *stmt, std::string binding, ASTContext &Context) {
    return false;
}

bool FaultInjector::checkMacroExpansion(const Stmt *stmt, std::string binding, ASTContext &Context) {
    return checkStmt(stmt, binding, Context);
}

bool FaultInjector::checkMacroExpansion(const Decl *stmt, std::string binding, ASTContext &Context) {
    return checkStmt(stmt, binding, Context);
}

bool FaultInjector::checkMacroDefinition(const Stmt *stmt, std::string binding, ASTContext &Context) {
    return false;
}

bool FaultInjector::checkMacroDefinition(const Decl *stmt, std::string binding, ASTContext &Context) {
    return false;
}

FaultInjector::FaultInjector() {
}

FaultInjector::~FaultInjector() {
}

void FaultInjector::push(std::string binding, const Stmt *st, bool left, bool isMacroExpansion) {
    StmtBinding sb(binding, st, left, isMacroExpansion);
    locations.push_back(sb);
    _sort();
}
void FaultInjector::push(std::string binding, const Decl *st, bool left, bool isMacroExpansion) {
    StmtBinding sb(binding, st, left, isMacroExpansion);
    locations.push_back(sb);
    _sort();
}

void FaultInjector::push(std::string binding, std::vector<const Stmt *> list) {
    StmtBinding sb(binding, list);
    locations.push_back(sb);
    _sort();
}
void FaultInjector::push(std::string binding, std::vector<const Decl *> list) {
    StmtBinding sb(binding, list);
    locations.push_back(sb);
    _sort();
}

void FaultInjector::pushMakroDef(std::string binding, const Stmt *stmt, SourceManager &SM, bool left) {
    StmtBinding sb(binding, stmt, left);
    locations.push_back(sb);
    addedMacroPositions.push_back(SM.getSpellingLoc(stmt->getLocStart()));
    _sortMacro(SM);
}
void FaultInjector::pushMakroDef(std::string binding, const Decl *decl, SourceManager &SM, bool left) {
    StmtBinding sb(binding, decl, left);
    locations.push_back(sb);
    addedMacroPositions.push_back(SM.getSpellingLoc(decl->getLocStart()));
    _sortMacro(SM);
}
void FaultInjector::matchAST(ASTContext &Context) {
    Matcher.matchAST(Context);
}

void FaultInjector::setFileName(std::string name) {
    fileName = std::string(name);
}

std::string FaultInjector::getFileName() {
    std::string ret(fileName.c_str());
    return ret;
}

void FaultInjector::setMatchMacro(bool match) {
    matchMacroDefinition = match;
    matchMacroExpansion = match;
}

void FaultInjector::setMatchMacro(bool matchDef, bool matchExp) {
    matchMacroDefinition = matchDef;
    matchMacroExpansion = matchExp;
}

void FaultInjector::nodeCallbackMacroDef(std::string binding, const Stmt *stmt, SourceManager &SM, bool left) {
    pushMakroDef(binding, stmt, SM, left);
}
void FaultInjector::nodeCallbackMacroDef(std::string binding, const Decl *decl, SourceManager &SM, bool left) {
    pushMakroDef(binding, decl, SM, left);
}
bool FaultInjector::isMacroDefinitionAdded(SourceLocation locStart, SourceManager &SM) {
    BeforeThanCompare<SourceLocation> isBefore(SM);
    for (SourceLocation loc : addedMacroPositions) {
        if ((std::string(SM.getFilename(SM.getExpansionLoc(loc)))).compare(std::string(SM.getFilename(locStart))) ==
            0) {
            if (!isBefore(loc, locStart) && !isBefore(locStart, loc)) {
                return true;
            }
        }
    }
    return false;
}
void FaultInjector::nodeCallbackMacroExpansion(std::string binding, const Stmt *stmt, bool left) {
    push(binding, stmt, left, true);
}
void FaultInjector::nodeCallbackMacroExpansion(std::string binding, const Decl *decl, bool left) {
    push(binding, decl, left, true);
}

void FaultInjector::nodeCallback(std::string binding, const Stmt *stmt, bool left) {
    push(binding, stmt, left);
}

void FaultInjector::nodeCallback(std::string binding, const Decl *decl, bool left) {
    push(binding, decl, left);
}

void FaultInjector::nodeCallback(std::string binding, std::vector<const Stmt *> list) {
    push(binding, std::vector<const Stmt *>(list.begin(), list.end()));
}
void FaultInjector::nodeCallback(std::string binding, std::vector<const Decl *> list) {
    push(binding, std::vector<const Decl *>(list.begin(), list.end()));
}
void FaultInjector::_sort() {
    std::sort(locations.begin(), locations.end(), comparefunc);
}
void FaultInjector::_sortMacro(SourceManager &SM) {
    BeforeThanCompare<SourceLocation> isBefore(SM);
    std::sort(addedMacroPositions.begin(), addedMacroPositions.end(), isBefore);
    std::sort(macroLocations.begin(), macroLocations.end(), comparefunc);
}

bool FaultInjector::comparefunc(StmtBinding st1, StmtBinding st2) {
    SourceLocation l1, l2;
    if (st1.isList) {
        if (st1.isStmt) {
            l1 = st1.stmtlist[0]->getLocStart();
            for (const Stmt *stmt : st1.stmtlist) {
                if (stmt->getLocStart() < l1) {
                    l1 = stmt->getLocStart();
                }
            }
        } else {
            l1 = st1.decllist[0]->getLocStart();
            for (const Decl *decl : st1.decllist) {
                if (decl->getLocStart() < l1) {
                    l1 = decl->getLocStart();
                }
            }
        }
    } else {
        if (st1.isStmt) {
            l1 = st1.stmt->getLocStart();
        } else {
            l1 = st1.decl->getLocStart();
        }
    }

    if (st2.isList) {
        if (st2.isStmt) {
            l2 = st2.stmtlist[0]->getLocStart();
            for (const Stmt *stmt : st2.stmtlist) {
                if (stmt->getLocStart() < l2) {
                    l2 = stmt->getLocStart();
                }
            }
        } else {
            l2 = st2.decllist[0]->getLocStart();
            for (const Decl *decl : st2.decllist) {
                if (decl->getLocStart() < l2) {
                    l2 = decl->getLocStart();
                }
            }
        }
    } else {
        if (st2.isStmt) {
            l2 = st2.stmt->getLocStart();
        } else {
            l2 = st2.decl->getLocStart();
        }
    }
    return l1 < l2; // l2<l1;
}

void FaultInjector::dumpStmt(const Stmt *stmt, ASTContext &Context) {
    stmt->dumpPretty(Context);
}

std::string FaultInjector::stmtToString(const Stmt *stmt, const LangOptions &langOpts) {
    std::string statement;
    llvm::raw_string_ostream stream(statement);
    stmt->printPretty(stream, NULL, PrintingPolicy(langOpts));
    stream.flush();
    return statement;
}

void FaultInjector::dumpStmt(const Decl *decl) {
    decl->dump();
}

std::string FaultInjector::stmtToString(const Decl *decl, const LangOptions &langOpts) {
    std::string statement;
    llvm::raw_string_ostream stream(statement);
    decl->print(stream, PrintingPolicy(langOpts));
    stream.flush();
    return statement;
}

std::string FaultInjector::getEditedString(Rewriter &rewrite, ASTContext &Context) {
    return rewriteBufferToString(rewrite.getEditBuffer(Context.getSourceManager().getMainFileID()));
}

std::string FaultInjector::rewriteBufferToString(RewriteBuffer &buffer) {
    std::string str;
    llvm::raw_string_ostream stream(str);
    buffer.write(stream);
    stream.flush();
    return str;
}

std::string FaultInjector::sourceLocationToString(SourceLocation loc, const SourceManager &sourceManager) {
    return loc.printToString(sourceManager);
}

std::string FaultInjector::sourceRangeToString(SourceRange range, const SourceManager &sourceManager) {
    return sourceLocationToString(range.getBegin(), sourceManager) + " - " +
           sourceLocationToString(range.getEnd(), sourceManager);
}

std::string FaultInjector::sourceRangeToString(const Stmt *stmt, const SourceManager &sourceManager) {
    return sourceRangeToString(stmt->getSourceRange(), sourceManager);
}

std::string FaultInjector::sourceRangeToString(const Decl *decl, const SourceManager &sourceManager) {
    return sourceRangeToString(decl->getSourceRange(), sourceManager);
}

void FaultInjector::printStep(StmtBinding current, const SourceManager &sourceManager, const LangOptions &langOpts,
                              int i, int size) {
    std::cout << "injecting '" << toString() << "' [" << i + 1 << "/" << size << "]" << std::endl;
    if (current.isStmt) {
        if (current.isList) {
            // std::cout << sourceRangeToString(current.stmt,sourceManager)<<endl;
            std::cout << "List with " << current.stmtlist.size() << " Statements" << std::endl;
        } else {
            std::cout << sourceRangeToString(current.stmt, sourceManager) << std::endl;
            std::cout << stmtToString(current.stmt, langOpts) << std::endl;
        }
    } else {
        if (current.isList) {
            std::cout << "List with " << current.decllist.size() << " Decl" << std::endl;
        } else {
            std::cout << sourceRangeToString(current.decl, sourceManager) << std::endl;
            std::cout << stmtToString(current.decl, langOpts) << std::endl;
        }
    }
} // with printing statements

void FaultInjector::printStep(StmtBinding current, const SourceManager &sourceManager, int i, int size) {
    std::cout << "injecting '" << toString() << "' [" << i + 1 << "/" << size << "]" << std::endl;
    /*if(current.isStmt){
        std::cout <<
    sourceRangeToString(current.stmt,sourceManager)<<endl;//current.stmt ->
    getLocStart().printToString(Context.getSourceManager())<< " -
    "<<current.stmt ->
    getLocEnd().printToString(Context.getSourceManager())<<endl;
    } else {
        std::cout <<
    sourceRangeToString(current.decl,sourceManager)<<endl;//current.stmt ->
    getLocStart().printToString(Context.getSourceManager())<< " -
    "<<current.stmt ->
    getLocEnd().printToString(Context.getSourceManager())<<endl;
    }*/
} // only position
std::string FaultInjector::_inject(StmtBinding current, ASTContext &Context, bool isMacroDefinition) {
    return inject(current, Context); // default case - no match in macro definitions
}

void FaultInjector::setVerbose(bool v) {
    verbose = v;
}

void FaultInjector::setDirectory(std::string directory) {
    dir = directory;
}
void FaultInjector::inject(std::vector<StmtBinding> target, ASTContext &Context, bool isMacroDefinition) {
    int i = 0;

    for (StmtBinding current : target) {
        if (verbose) {
            printStep(current, Context.getSourceManager(), Context.getLangOpts(), i++, target.size());
        } else {
            i++;
        }
        // else
        //    printStep(current, Context.getSourceManager(),i++,target.size());
        std::string result = _inject(current, Context, isMacroDefinition);
        if (result.compare("")) {
            if (verbose) {
                std::cout << " -Success" << std::endl;
            }
            writeDown(result, i - 1);
        } else if (verbose) {
            std::cerr << "-Failed" << std::endl;
        }
    }
}
void FaultInjector::writeDown(std::string data, int i) {
    std::string name = (dir.compare("") ? dir + "/" : "") + toString() + "_" + std::to_string(i);
    std::ofstream file(name + ".cpp");
    file << data;
    file.flush();
    file.close();
    system(("diff -U 0 \"" + fileName + "\" \"" + name + ".cpp\" > \"" + name + ".patch\"").c_str());
}

std::string FaultInjector::getFileName(const Stmt *stmt, SourceManager &SM) {
    if (stmt != NULL) {
        SourceLocation start = stmt->getLocStart();
        if (start.isMacroID()) {
            return std::string(SM.getFilename(SM.getExpansionLoc(start)));
        } else {
            return std::string(SM.getFilename(start));
        }
    } else {
        return "";
    }
}

std::string FaultInjector::getFileName(const Decl *decl, SourceManager &SM) {
    if (decl != NULL) {
        SourceLocation start = decl->getLocStart();
        if (start.isMacroID()) {
            return std::string(SM.getFilename(SM.getExpansionLoc(start)));
        } else {
            return std::string(SM.getFilename(start));
        }
    } else {
        return "";
    }
}
