#include "GenericRewriter.h"
#include <iostream>
#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif

#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessingRecord.h"

using namespace clang;

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "clang-sfi-generic-rewriter"
using namespace llvm;

bool GenericRewriter::RemoveText(SourceRange range, RewriteOptions opts) {
    LLVM_DEBUG(dbgs() << "Start of RemoveText()\n");
    SourceManager &SM = getSourceMgr();
    if (rangeIsFreeOfMacroExpansions(range)) {
        LLVM_DEBUG(dbgs() << "It is free of macros, we call the rewriter.\n");
        if (!considerFile(range.getBegin())) {
            return false;
        }
        Rewriter::RemoveText(range, opts);
        return true;
    }

    if (SM.isInSystemMacro(range.getBegin()) || SM.isInSystemMacro(range.getEnd()) || !considerFile(range.getBegin()) || !considerFile(range.getEnd())) {
        LLVM_DEBUG(dbgs() << "It is a system macro, we do not modify it.\n");
        return false;
    }

    SourceLocation nextLoc;
    Optional<Token> tok;
    tok = Lexer::findNextToken(range.getEnd(), SM, getLangOpts());
    if (tok) {
        nextLoc = tok->getLocation();
    } else {
        tok = Lexer::findNextToken(SM.getSpellingLoc(range.getEnd()), SM, getLangOpts());
        if (tok) {
            nextLoc = tok->getLocation();
        }
    }

    SourceLocation loc = range.getEnd();
    while (SM.getFileID(range.getBegin()) != SM.getFileID(loc)) {
        SourceLocation loc2 = getDirectExpansionLoc(loc);
        if (loc2.isInvalid() || loc == loc2) {
            LLVM_DEBUG(dbgs() << "This should not happen.\n");
            return false;
        } else {
            loc = loc2;
        }
    }
    SourceRange remove;
    if (range.getBegin().isFileID()) {
        LLVM_DEBUG(dbgs() << "isFileID\n");
        remove.setBegin(range.getBegin());
        remove.setEnd(loc);
    } else {
        LLVM_DEBUG(dbgs() << "isNoFileID\n");
        remove.setBegin(SM.getSpellingLoc(range.getBegin()));
        remove.setEnd(SM.getSpellingLoc(loc));
    }
    if (getSourceMgr().isBeforeInTranslationUnit(remove.getEnd(), remove.getBegin())) {
        SourceLocation tmp = remove.getBegin();
        remove.setBegin(remove.getEnd());
        remove.setEnd(tmp);
    }
    LLVM_DEBUG(dbgs() << "Range to be removed:\n"
        << remove.getBegin().printToString(getSourceMgr()) << "\n"
        << remove.getEnd().printToString(getSourceMgr()) << "\n");
    if (!considerFile(remove.getBegin()) || !considerFile(remove.getEnd())) {
        LLVM_DEBUG(dbgs() << "File should not be considered.\n");
        return false;
    }

    if (SM.getFileID(range.getBegin()) == SM.getFileID(nextLoc)) {
        LLVM_DEBUG(dbgs() << "Next location is the same as start, we can remove it.\n");
        Rewriter::RemoveText(remove, opts);
        return true;
    }
    if (numberNestedMacros(range.getBegin()) >= numberNestedMacros(nextLoc)) {
        LLVM_DEBUG(dbgs() << "We have moved up in the macro hierarchy, therefore we can remove it.\n");
        Rewriter::RemoveText(remove, opts);
        return true;
    }
    return false;
}

bool GenericRewriter::ReplaceText(SourceRange range, StringRef NewStr) {
    LLVM_DEBUG(dbgs() << "Start of ReplaceText()\n");
    SourceManager &SM = getSourceMgr();
    if (rangeIsFreeOfMacroExpansions(range)) {
        LLVM_DEBUG(dbgs() << "It is free of macros, we call the rewriter.\n");
        if (!considerFile(range.getBegin())) {
            return false;
        }
        Rewriter::ReplaceText(range, NewStr);
        return true;
    }

    if (SM.isInSystemMacro(range.getBegin()) || SM.isInSystemMacro(range.getEnd()) || !considerFile(range.getBegin()) || !considerFile(range.getEnd())) {
        LLVM_DEBUG(dbgs() << "It is a system macro, we do not modify it.\n");
        return false;
    }

    SourceLocation nextLoc;
    Optional<Token> tok;
    tok = Lexer::findNextToken(range.getEnd(), SM, getLangOpts());
    if (tok) {
        nextLoc = tok->getLocation();
    } else {
        tok = Lexer::findNextToken(SM.getSpellingLoc(range.getEnd()), SM, getLangOpts());
        if (tok) {
            nextLoc = tok->getLocation();
        }
    }

    SourceLocation loc = range.getEnd();
    while (SM.getFileID(range.getBegin()) != SM.getFileID(loc)) {
        SourceLocation loc2 = getDirectExpansionLoc(loc);
        if (loc2.isInvalid() || loc == loc2) {
            LLVM_DEBUG(dbgs() << "This should not happen.\n");
            return false;
        } else {
            loc = loc2;
        }
    }
    SourceRange remove;
    if (range.getBegin().isFileID()) {
        LLVM_DEBUG(dbgs() << "isFileID\n");
        remove.setBegin(range.getBegin());
        remove.setEnd(loc);
    } else {
        LLVM_DEBUG(dbgs() << "isNoFileID\n");
        remove.setBegin(SM.getSpellingLoc(range.getBegin()));
        remove.setEnd(SM.getSpellingLoc(loc));
    }
    if (getSourceMgr().isBeforeInTranslationUnit(remove.getEnd(), remove.getBegin())) {
        SourceLocation tmp = remove.getBegin();
        remove.setBegin(remove.getEnd());
        remove.setEnd(tmp);
    }
    LLVM_DEBUG(dbgs() << "Range to be removed:\n"
        << remove.getBegin().printToString(getSourceMgr()) << "\n"
        << remove.getEnd().printToString(getSourceMgr()) << "\n");
    if (!considerFile(remove.getBegin()) || !considerFile(remove.getEnd())) {
        LLVM_DEBUG(dbgs() << "File should not be considered.\n");
        return false;
    }

    if (SM.getFileID(range.getBegin()) == SM.getFileID(nextLoc)) {
        LLVM_DEBUG(dbgs() << "Next location is the same as start, we can remove it.\n");
        Rewriter::ReplaceText(remove, NewStr);
        return true;
    }
    if (numberNestedMacros(range.getBegin()) >= numberNestedMacros(nextLoc)) {
        LLVM_DEBUG(dbgs() << "We have moved up in the macro hierarchy, therefore we can remove it.\n");
        Rewriter::ReplaceText(remove, NewStr);
        return true;
    }
    return false;
}

bool GenericRewriter::InsertText(SourceLocation Loc, StringRef Str, bool InsertAfter, bool indentNewLines) {
    LLVM_DEBUG(dbgs() << "Start of InsertText()\n");
    SourceManager &SM = getSourceMgr();
    if (!Loc.isMacroID()) {
        LLVM_DEBUG(dbgs() << "It is free of macros, we call the rewriter.\n");
        if (!considerFile(Loc)) {
            return false;
        }
        Rewriter::InsertText(Loc, Str, InsertAfter, indentNewLines);
        return true;
    } else if (Loc.isMacroID()) {
        LLVM_DEBUG(dbgs() << "Loc is in a macro, we call it with spelling location.\n");
        if (SM.isInSystemMacro(Loc)) {
            LLVM_DEBUG(dbgs() << "It is a system macro, we do not modify it.\n");
            return false;
        }
        if (!considerFile(SM.getSpellingLoc(Loc))) {
            return false;
        }
        Rewriter::InsertText(SM.getSpellingLoc(Loc), Str, InsertAfter, indentNewLines);
        return true;
    }
    return false;
}

bool GenericRewriter::containsMacroExpansion(SourceRange range) {
    SourceManager &SM = getSourceMgr();
    for (SourceLocation loc = range.getBegin(); loc != range.getEnd().getLocWithOffset(1); loc = loc.getLocWithOffset(1)) {
        if (SM.isMacroArgExpansion(loc) || SM.isMacroBodyExpansion(loc)) {
            return true;
        }
    }
    return false;
}

bool GenericRewriter::startAndEndArePartOfTheSameExpansion(SourceRange range) {
    SourceManager &SM = getSourceMgr();
    auto fileID1 = SM.getFileID(range.getBegin());
    auto fileID2 = SM.getFileID(range.getEnd());
    if (fileID1.isInvalid() || fileID2.isInvalid()) {
        assert(true && "FileID is invalid");
        std::cerr << "FileID is invalid" << std::endl;
        return false;
    }
    if (!range.getBegin().isMacroID()) {
        // It is not a macro.
        return false;
    }
    if (fileID1 != fileID2) {
        // Start of the range has different fileID than the end. This means that
        // one of them is not an expansion or it is a different expansion.
        return false;
    }
    return true;
}

bool GenericRewriter::rangeIsFreeOfMacroExpansions(SourceRange range) {
    Preprocessor &PP = CI->getPreprocessor();
    PreprocessingRecord *PC = PP.getPreprocessingRecord();
    if (getSourceMgr().isBeforeInTranslationUnit(range.getEnd(), range.getBegin())) {
        // This can happen if one of the locations is a macro at the beginning of the file.
        SourceRange inverseRange(range.getEnd(), range.getBegin());
        auto pprecordsInRange = PC->getPreprocessedEntitiesInRange(inverseRange);
        // If the list is empty, there are no expansions in the range.
        return !range.getBegin().isMacroID() && !range.getEnd().isMacroID() && pprecordsInRange.begin() == pprecordsInRange.end();
    } else {
        auto pprecordsInRange = PC->getPreprocessedEntitiesInRange(range);
        // If the list is empty, there are no expansions in the range.
        return !range.getBegin().isMacroID() && !range.getEnd().isMacroID() && pprecordsInRange.begin() == pprecordsInRange.end();
    }
}

int GenericRewriter::numberNestedMacros(SourceLocation loc) {
    SourceManager &SM = getSourceMgr();
    int i = 0;
    while (SM.isMacroArgExpansion(loc)) {
        i++;
        loc = SM.getImmediateSpellingLoc(loc);
    }
    return i;
}

SourceLocation GenericRewriter::getDirectExpansionLoc(SourceLocation loc) {
    SourceManager &SM = getSourceMgr();
    if (loc.isMacroID()) {
        return SM.getSLocEntry(SM.getFileID(loc)).getExpansion().getExpansionLocStart();
    } else {
        return loc;
    }
}

bool GenericRewriter::considerFile(SourceLocation loc) {
    if (getSourceMgr().isInSystemHeader(loc)) {
        return false;
    }
    SourceManager &SM = getSourceMgr();
    std::string patchingFileName = SM.getFilename(loc);
    std::string patchingFileNameCan = fs::weakly_canonical(patchingFileName);
    std::string rootDirCan = fs::weakly_canonical(rootDir);
    std::string fileNameCan = fs::weakly_canonical(fileName);
    LLVM_DEBUG(dbgs() << "Checking if " << patchingFileNameCan << " should be considered: ");
    if (patchingFileName.empty()) {
        LLVM_DEBUG(dbgs() << "No, empty filename\n");
        return false;
    } else if (fs::equivalent(fileName, patchingFileName)) {
        LLVM_DEBUG(dbgs() << "Yes, main file\n");
        return true;
    } else if (rootDir.compare("") != 0 && patchingFileNameCan.find_first_of(rootDirCan, 0) == 0) {
        LLVM_DEBUG(dbgs() << "Yes, is in source tree\n");
        return true;
    } else if (fileList.size() != 0) {
        LLVM_DEBUG(dbgs() << "checking if in fileList... ");
        for (std::string name : fileList) {
            if (patchingFileName.compare(name) == 0 || patchingFileName.compare(rootDir + name) == 0) {
                LLVM_DEBUG(dbgs() << "Yes\n");
                return true;
            }
        }
    }
    LLVM_DEBUG(dbgs() << "No\n");

    return false;
}

void GenericRewriter::setCI(CompilerInstance *CI) {
    this->CI = CI;
}
void GenericRewriter::setFileName(std::string fileName) {
    this->fileName = fileName;
}
void GenericRewriter::setRootDir(std::string rootDir) {
    this->rootDir = rootDir;
}
void GenericRewriter::setFileList(std::vector<std::string> fileList) {
    this->fileList = fileList;
}
