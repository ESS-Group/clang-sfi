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
    if (range.getBegin().isFileID() && SM.getFileID(range.getBegin()) == SM.getFileID(range.getEnd())) {
        LLVM_DEBUG(dbgs() << "It may contain an expansion, but we just remove, so we remove it.\n");
        if (!considerFile(range.getBegin())) {
            return false;
        }
        Rewriter::RemoveText(range, opts);
        return true;
    }
    if (SM.isInSystemMacro(range.getBegin())) {
        LLVM_DEBUG(dbgs() << "It is a system macro, we do not modify it.\n");
        return false;
    }
    if (isCompletelyPartOfOneMacroExpansion(range) || isFunctionLikeMacroWithoutArguments(range)) {
        if (isCompletelyPartOfOneMacroExpansion(range)) {
            LLVM_DEBUG(dbgs() << "It isCompletelyPartOfOneMacroExpansion, we modify the macro.\n");
        }
        if (isFunctionLikeMacroWithoutArguments(range)) {
            LLVM_DEBUG(dbgs() << "It isFunctionLikeMacroWithoutArguments, we modify the macro.\n");
        }
        LLVM_DEBUG(dbgs() << "We try to handle the macro.\n");
        SourceRange spellingRange(getSourceMgr().getSpellingLoc(range.getBegin()), getSourceMgr().getSpellingLoc(range.getEnd()));
        if (!considerFile(spellingRange.getBegin())) {
            return false;
        }
        Rewriter::RemoveText(spellingRange, opts);
        return true;
    } else {
        LLVM_DEBUG(dbgs() << "It is not completely part of one expansion nor a function-like macro without arguments. We do not know how to handle. returning false\n");
        return false;
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
    if (range.getBegin().isFileID() && SM.getFileID(range.getBegin()) == SM.getFileID(range.getEnd())) {
        LLVM_DEBUG(dbgs() << "It may contain an expansion, but we just replace, so we replace it.\n");
        if (!considerFile(range.getBegin())) {
            return false;
        }
        Rewriter::ReplaceText(range, NewStr);
        return true;
    }
    if (SM.isInSystemMacro(range.getBegin())) {
        LLVM_DEBUG(dbgs() << "It is a system macro, we do not modify it.\n");
        return false;
    }
    if (isCompletelyPartOfOneMacroExpansion(range) || isFunctionLikeMacroWithoutArguments(range)) {
        if (isCompletelyPartOfOneMacroExpansion(range)) {
            LLVM_DEBUG(dbgs() << "It isCompletelyPartOfOneMacroExpansion, we modify the macro.\n");
        }
        if (isFunctionLikeMacroWithoutArguments(range)) {
            LLVM_DEBUG(dbgs() << "It isFunctionLikeMacroWithoutArguments, we modify the macro.\n");
        }
        LLVM_DEBUG(dbgs() << "We try to handle the macro.\n");
        SourceRange spellingRange(getSourceMgr().getSpellingLoc(range.getBegin()), getSourceMgr().getSpellingLoc(range.getEnd()));
        if (!considerFile(spellingRange.getBegin())) {
            return false;
        }
        Rewriter::ReplaceText(spellingRange, NewStr);
        return true;
    } else {
        LLVM_DEBUG(dbgs() << "It is not completely part of one expansion nor a function-like macro without arguments. We do not know how to handle. returning false\n");
        return false;
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

bool GenericRewriter::isCompletelyPartOfOneMacroExpansion(SourceRange range) {
    if (!startAndEndArePartOfTheSameExpansion(range)) {
        return false;
    }
    Preprocessor &PP = CI->getPreprocessor();
    PreprocessingRecord *PC = PP.getPreprocessingRecord();
    auto pprecordsInRange = PC->getPreprocessedEntitiesInRange(range);

    // If pprecordsInRange contains elements, there are expansions in the expansion.
    return pprecordsInRange.begin() == pprecordsInRange.end();
}

bool GenericRewriter::isFunctionLikeMacroWithoutArguments(SourceRange range) {
    if (!startAndEndArePartOfTheSameExpansion(range)) {
        return false;
    }
    SourceManager &SM = getSourceMgr();
    Preprocessor &PP = CI->getPreprocessor();
    PreprocessingRecord *PC = PP.getPreprocessingRecord();
    auto pprecordsInRange = PC->getPreprocessedEntitiesInRange(range);
    SourceRange expandedRange(SM.getExpansionLoc(range.getBegin()), SM.getExpansionLoc(range.getEnd()));

    int pprecordcount = 0;
    for (auto i = pprecordsInRange.begin(); i != pprecordsInRange.end(); i++) {
        pprecordcount++;
    }
    if (pprecordcount == 0) {
        // Function-like macros always have at least one entry inside.
        LLVM_DEBUG(dbgs() << "pprecordcount == 0\n");
        return false;
    }
    if (pprecordcount > 1) {
        // If there is more than one expansion inside, then there are arguments or a second expansion.
        LLVM_DEBUG(dbgs() << "pprecordcount > 1\n");
        return false;
    }

    auto firstpprecord = pprecordsInRange.begin();
    for (SourceLocation loc = firstpprecord->getSourceRange().getBegin();
        loc != firstpprecord->getSourceRange().getEnd(); loc = loc.getLocWithOffset(1)) {
        Token TheTok;
        if (Lexer::getRawToken(loc, TheTok, getSourceMgr(), getLangOpts())) {
            LLVM_DEBUG(dbgs() << "getRawToken failed\n");
            return false;
        }
        if (TheTok.getKind() == tok::l_paren && loc.getLocWithOffset(1) != firstpprecord->getSourceRange().getEnd()) {
            // It is a function-like macro with arguments, because there is an opening parenthesis and
            // it is not the last token.
            LLVM_DEBUG(dbgs() << "There is an opening parentheses in macro and it is not the last token\n");
            return false;
        }
    }
    if (firstpprecord->getSourceRange().getBegin() == expandedRange.getBegin()
        && firstpprecord->getSourceRange().getEnd() != expandedRange.getBegin()) {
        Token TheTok;
        if (Lexer::getRawToken(firstpprecord->getSourceRange().getEnd(), TheTok, getSourceMgr(), getLangOpts())) {
            LLVM_DEBUG(dbgs() << "getRawToken failed\n");
            return false;
        }
        if (TheTok.getKind() != tok::r_paren) {
            // It is a function-like macro with argument (it had an opening parenthesis),
            // but we did not find a closing parenthesis, although we checked that the
            // opening one was the last token -- that is weird.
            LLVM_DEBUG(dbgs() << "Token is no closing parenthesis, but " << TheTok.getName() << "\n");
            return false;
        }
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
        return !range.getBegin().isMacroID() && pprecordsInRange.begin() == pprecordsInRange.end();
    } else {
        auto pprecordsInRange = PC->getPreprocessedEntitiesInRange(range);
        // If the list is empty, there are no expansions in the range.
        return !range.getBegin().isMacroID() && pprecordsInRange.begin() == pprecordsInRange.end();
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
    if (fs::equivalent(fileName, patchingFileName)) {
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
