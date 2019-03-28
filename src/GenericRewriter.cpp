#include "GenericRewriter.h"
#include <iostream>

using namespace clang;

bool GenericRewriter::RemoveText(SourceRange range, RewriteOptions opts) {
    SourceRange expandedRange = SourceRange(getSourceMgr().getExpansionLoc(range.getBegin()), getSourceMgr().getExpansionLoc(range.getEnd()));
    return Rewriter::RemoveText(expandedRange.getBegin(), getRangeSize(expandedRange, opts), opts);
}

bool GenericRewriter::ReplaceText(SourceRange range, StringRef NewStr) {
    SourceRange expandedRange = SourceRange(getSourceMgr().getExpansionLoc(range.getBegin()), getSourceMgr().getExpansionLoc(range.getEnd()));
    return Rewriter::ReplaceText(expandedRange.getBegin(), getRangeSize(expandedRange), NewStr);
}

bool GenericRewriter::InsertText(SourceLocation Loc, StringRef Str, bool InsertAfter, bool indentNewLines) {
    SourceLocation expandedLoc = getSourceMgr().getExpansionLoc(Loc);
    return Rewriter::InsertText(expandedLoc, Str, InsertAfter, indentNewLines);
}