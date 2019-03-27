#include "FaultInjector.h"

#include <fstream>

#include <iterator>

#include "libs/dtl/dtl/dtl.hpp"

using namespace clang;
using namespace clang::ast_matchers;

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "clang-sfi-injector"
using namespace llvm;

void FaultInjector::setRootDir(std::string dir) {
    rootDir = dir;
}

void FaultInjector::setFileList(std::vector<std::string> list) {
    fileList = list;
}

FaultInjector *FaultInjector::createMatchHandler(std::string binding) {
    return this;
}

void FaultInjector::run(const MatchFinder::MatchResult &Result) {
    LLVM_DEBUG(dbgs() << "Run MatchHandler\n");
    SourceManager &SM = Result.Context->getSourceManager();
    auto bindings = Result.Nodes.getMap();
    for (auto i : bindings) {
        auto binding = i.first;
        if (const Stmt *stmt = dyn_cast_or_null<Stmt>(Result.Nodes.getNodeAs<Stmt>(binding))) {
            run_stmt_or_decl(Result, SM, binding, *stmt);
        } else if (const Decl *decl = dyn_cast_or_null<Decl>(Result.Nodes.getNodeAs<Decl>(binding))) {
            run_stmt_or_decl(Result, SM, binding, *decl);
        }
    }
}

template <typename SD>
void FaultInjector::run_stmt_or_decl(const MatchFinder::MatchResult &Result, SourceManager &SM, std::string binding,
                                   SD &stmtOrDecl) {
    if (!SM.isInSystemHeader(stmtOrDecl.getBeginLoc()) && // do not match on system headers or system macros
        !SM.isInSystemMacro(stmtOrDecl.getBeginLoc())) {
        std::string name = FaultInjector::getFileName(stmtOrDecl, SM);
        if (considerFile(name)) {
            if (checkStmt(stmtOrDecl, binding, *Result.Context)) {
                nodeCallback(binding, stmtOrDecl);
            }
        }
    }
}

bool FaultInjector::considerFile(std::string fileName) {
    if (getFileName().compare(fileName) == 0) {
        return true;
    } else if (rootDir.compare("") != 0 && fileName.find_first_of(rootDir, 0) == 0) {
        // is in source tree and rootDir is defined
        return true;
    } else if (fileList.size() != 0) {
        for (std::string name : fileList) {
            if (fileName.compare(name) == 0 || fileName.compare(rootDir + name) == 0) {
                return true;
            }
        }
    }

    return false;
}

bool FaultInjector::checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) {
    return false;
}

bool FaultInjector::checkStmt(const Decl &stmt, std::string binding, ASTContext &Context) {
    return false;
}

FaultInjector::FaultInjector() {
}

FaultInjector::~FaultInjector() {
}

template<class T>
void FaultInjector::push(std::string binding, const T &stmtOrDecl, bool left, bool isMacroExpansion) {
    StmtBinding sb(binding, stmtOrDecl, left, isMacroExpansion);
    locations.push_back(sb);
    _sort();
}

template<class T>
void FaultInjector::push(std::string binding, std::vector<const T *> list) {
    StmtBinding sb(binding, list);
    locations.push_back(sb);
    _sort();
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

void FaultInjector::nodeCallback(std::string binding, const Stmt &stmt, bool left) {
    push(binding, stmt, left);
}

void FaultInjector::nodeCallback(std::string binding, const Decl &decl, bool left) {
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

bool FaultInjector::comparefunc(StmtBinding st1, StmtBinding st2) {
    SourceLocation l1, l2;
    if (st1.isList) {
        if (st1.isStmt) {
            l1 = st1.stmtlist[0]->getBeginLoc();
            for (const Stmt *stmt : st1.stmtlist) {
                if (stmt->getBeginLoc() < l1) {
                    l1 = stmt->getBeginLoc();
                }
            }
        } else {
            l1 = st1.decllist[0]->getBeginLoc();
            for (const Decl *decl : st1.decllist) {
                if (decl->getBeginLoc() < l1) {
                    l1 = decl->getBeginLoc();
                }
            }
        }
    } else {
        if (st1.isStmt) {
            l1 = st1.stmt->getBeginLoc();
        } else {
            l1 = st1.decl->getBeginLoc();
        }
    }

    if (st2.isList) {
        if (st2.isStmt) {
            l2 = st2.stmtlist[0]->getBeginLoc();
            for (const Stmt *stmt : st2.stmtlist) {
                if (stmt->getBeginLoc() < l2) {
                    l2 = stmt->getBeginLoc();
                }
            }
        } else {
            l2 = st2.decllist[0]->getBeginLoc();
            for (const Decl *decl : st2.decllist) {
                if (decl->getBeginLoc() < l2) {
                    l2 = decl->getBeginLoc();
                }
            }
        }
    } else {
        if (st2.isStmt) {
            l2 = st2.stmt->getBeginLoc();
        } else {
            l2 = st2.decl->getBeginLoc();
        }
    }
    return l1 < l2;
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
} // only position

class Diff {
  public:
    std::string name;
    std::string dir;
    std::string diff;
    Diff(std::string name, std::string dir, std::string diff) : diff(diff), dir(dir), name(name) {
    }
};

void getLines(std::string str, std::vector<std::string> &lines) {
    std::istringstream stream(str);
    std::string temp;
    while (std::getline(stream, temp)) {
        lines.push_back(temp);
    }
}

void FaultInjector::generatePatchFile(StmtBinding current, ASTContext &Context, int i, bool isMacroDefinition) {
    LLVM_DEBUG(dbgs() << "Entering generatePatchFile\n");
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if (inject(current, Context, R)) { // default case - no match in macro definitions
        LLVM_DEBUG(dbgs() << "Injection in Rewriter was successful\n");
        std::map<clang::FileID, clang::RewriteBuffer>::iterator buffit;
        assert(R.buffer_begin() != R.buffer_end() && "No rewrite buffer found");
        for (buffit = R.buffer_begin(); buffit != R.buffer_end();
             buffit++) { // iterate through all RewriteBuffers in the Rewriter (only Buffers where the Rewriter changed
                         // sth.)
            clang::FileID fileId = buffit->first;
            const clang::FileEntry *fe = R.getSourceMgr().getFileEntryForID(fileId);
            std::vector<Diff> diffs;
            if (fe != NULL) {
                const clang::DirectoryEntry *dire = fe->getDir();
                if (dire != NULL) {
                    llvm::MemoryBuffer *origBuff = R.getSourceMgr().getBuffer(fileId);
                    if (origBuff != NULL) {
                        std::string dir = dire->getName();
                        std::string fileName = fe->getName();
                        RewriteBuffer &buffer = buffit->second;

                        // get lines vector for comparison
                        std::vector<std::string> rewritten;
                        getLines(rewriteBufferToString(buffer), rewritten);
                        std::vector<std::string> original;
                        std::string orig = origBuff->getBuffer();
                        getLines(orig, original);

                        // get unified diff
                        dtl::Diff<std::string, std::vector<std::string>> d(original, rewritten);
                        std::stringstream unified;
                        d.onHuge();
                        d.compose();
                        d.composeUnifiedHunks();
                        // d.printUnifiedFormat();
                        // d.printUnifiedFormat(unified);

                        auto hunks = d.getUniHunks();
                        bool hasDiff = false;
                        for (auto hunk : hunks) { // hunks to string
                            std::vector<std::string> lines;
                            int iLast = -1;
                            int iCurr = -1;
                            for (auto change : hunk.change) {
                                iCurr++;
                                switch (change.second.type) {
                                case dtl::SES_ADD:
                                    lines.push_back(SES_MARK_ADD + change.first); //<< std::endl;
                                    iLast = iCurr;
                                    hasDiff = true;
                                    break;
                                case dtl::SES_DELETE:
                                    lines.push_back(SES_MARK_DELETE + change.first); // << std::endl;
                                    iLast = iCurr;
                                    hasDiff = true;
                                    break;
                                case dtl::SES_COMMON:
                                    lines.push_back(SES_MARK_COMMON + change.first); //<< std::endl;
                                    break;
                                }
                            }
                            if (iLast != -1) {
                                int prefixpadding = std::distance(hunk.common[0].begin(), hunk.common[0].end());
                                int postfixpadding =
                                    iCurr -
                                    iLast; // same as std::distance(hunk.common[1].begin(), hunk.common[1].end()), but
                                           // more efficient this way because already calculated before.
                                unified << "@@ -" << hunk.a + prefixpadding;
                                int b = hunk.b - postfixpadding - prefixpadding;
                                if (b > 1) { // if hunk.b==1 ',1' is optional
                                    unified << "," << b;
                                }
                                unified << " +" << hunk.c + prefixpadding;

                                int d = hunk.d - postfixpadding - prefixpadding;
                                if (d > 1) { // if hunk.d==1 ',1' is optional
                                    unified << "," << d;
                                }
                                unified << " @@" << std::endl;

                                int iCurr = -1;
                                for (std::string change : lines) {
                                    iCurr++;
                                    if (iCurr > iLast) {
                                        break;
                                    }

                                    // https://stackoverflow.com/questions/484213/replace-line-breaks-in-a-stl-string
                                    std::string::size_type pos = 0; // Must initialize
                                    while ((pos = change.find("\r", pos)) != std::string::npos) {
                                        change.erase(pos, 1);
                                    }
                                    unified << change << std::endl;
                                }
                            }
                        }
                        if (hasDiff) {
                            std::string temp = unified.str();

                            diffs.push_back(Diff(fileName, dir, temp));
                        }
                    } else {
                        std::cerr << "Did not find SourceBuffer" << std::endl;
                    }
                } else {
                    std::cerr << "Did not find DirectoryEntry" << std::endl;
                }
            } else {
                std::cerr << "Did not find FileEntry" << std::endl;
            }

            if (diffs.size() > 0) {
                std::stringstream data;
                for (Diff diff : diffs) {
                    // first filename
                    data << "---" << diff.name << std::endl;
                    data << "+++" << diff.name << std::endl;
                    // then hunks
                    data << diff.diff;
                    // diff.print();
                }

                std::string name = (dir.compare("") ? dir + "/" : "") + toString() + "_" + std::to_string(i) + ".patch";
                LLVM_DEBUG(dbgs() << "New Patch File: " << name << "\n");
                if (verbose) {
                    std::cerr << "New Patch File: " << name << std::endl;
                }
                std::ofstream file(name);
                if (!file.good()) {
                    std::cerr << "Could not open file " << name << " for write" << std::endl;
                } else {
                    file << data.str();
                    file.flush();
                    file.close();

                    if (verbose) {
                        std::cout << " -Success" << std::endl;
                    }
                    LLVM_DEBUG(dbgs() << "Generated patch file successfully\n");
                }
            } else {
                std::cerr << "Injection did not generate hunks" << std::endl;
            }
        }
    } else {
        std::cerr << "Injection failed" << std::endl;
    }
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
            printStep(current, Context.getSourceManager(), Context.getLangOpts(), i, target.size());
        }

        generatePatchFile(current, Context, i++, isMacroDefinition);
    }
}

template<class T>
std::string FaultInjector::getFileName(const T &stmtOrDecl, SourceManager &SM) {
    SourceLocation start = stmtOrDecl.getBeginLoc();
    if (start.isMacroID()) {
        return std::string(SM.getFilename(SM.getExpansionLoc(start)));
    } else {
        return std::string(SM.getFilename(start));
    }
}
