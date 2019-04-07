#include "GenericRewriter.h"
#include <iostream>

#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessingRecord.h"

using namespace clang;

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "clang-sfi-generic-rewriter"
using namespace llvm;

bool GenericRewriter::RemoveText(SourceRange range, RewriteOptions opts) {
    LLVM_DEBUG(dbgs() << "Start of RemoveText()\n");
    if (rangeIsFreeOfMacroExpansions(range)) {
        LLVM_DEBUG(dbgs() << "It is free of macros, we call the rewriter.\n");
        return Rewriter::RemoveText(range, opts);
    }
    if (isCompletelyPartOfOneMacroExpansion(range) || isFunctionLikeMacroWithoutArguments(range)) {
        if (isCompletelyPartOfOneMacroExpansion(range)) {
            LLVM_DEBUG(dbgs() << "It isCompletelyPartOfOneMacroExpansion, we modify the macro.\n");
        }
        if (isFunctionLikeMacroWithoutArguments(range)) {
            LLVM_DEBUG(dbgs() << "It isFunctionLikeMacroWithoutArguments, we modify the macro.\n");
        }
        /// TODO: Must be done.
        std::cout << "We could handle the macro\n";
        return false;
    } else {
        LLVM_DEBUG(dbgs() << "It is not completely part of one expansion nor a function-like macro without arguments. We do not know how to handle. returning false\n");
        return false;
    }
    return false;
}

bool GenericRewriter::ReplaceText(SourceRange range, StringRef NewStr) {
    SourceRange expandedRange = SourceRange(getSourceMgr().getExpansionLoc(range.getBegin()), getSourceMgr().getExpansionLoc(range.getEnd()));
    return Rewriter::ReplaceText(expandedRange.getBegin(), getRangeSize(expandedRange), NewStr);
}

bool GenericRewriter::InsertText(SourceLocation Loc, StringRef Str, bool InsertAfter, bool indentNewLines) {
    SourceLocation expandedLoc = getSourceMgr().getExpansionLoc(Loc);
    return Rewriter::InsertText(expandedLoc, Str, InsertAfter, indentNewLines);
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
    Preprocessor &PP = CI->getPreprocessor();
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
    SourceManager &SM = getSourceMgr();
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
            // opening one was the last one -- that is weird.
            LLVM_DEBUG(dbgs() << "Token is no closing parenthesis, but " << TheTok.getName() << "\n");
            return false;
        }
    }
    return true;
}

bool GenericRewriter::rangeIsFreeOfMacroExpansions(SourceRange range) {
    SourceManager &SM = getSourceMgr();
    Preprocessor &PP = CI->getPreprocessor();
    PreprocessingRecord *PC = PP.getPreprocessingRecord();
    auto pprecordsInRange = PC->getPreprocessedEntitiesInRange(range);
    // If the list is empty, there are no expansions in the range.
    return !range.getBegin().isMacroID() && pprecordsInRange.begin() == pprecordsInRange.end();
}

void GenericRewriter::setCI(CompilerInstance *CI) {
    this->CI = CI;
}
