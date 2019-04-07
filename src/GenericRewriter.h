#ifndef REWRITER_H
#define REWRITER_H

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;

// The access modifier should be `private` here and only allowed methods noted with the `using` directive.
class GenericRewriter : public clang::Rewriter {
public:
    using clang::Rewriter::setSourceMgr;
    using clang::Rewriter::getSourceMgr;
    using clang::Rewriter::getLangOpts;

    bool RemoveText(SourceRange range, RewriteOptions opts = RewriteOptions());
    bool ReplaceText(SourceRange range, StringRef NewStr);
    bool InsertText(SourceLocation Loc, StringRef Str, bool InsertAfter = true, bool indentNewLines = false);

    bool startAndEndArePartOfTheSameExpansion(SourceRange range);
    bool containsMacroExpansion(SourceRange range);
    bool isCompletelyPartOfOneMacroExpansion(SourceRange range);
    bool isFunctionLikeMacroWithoutArguments(SourceRange range);
    bool rangeIsFreeOfMacroExpansions(SourceRange range);

    void setCI(CompilerInstance *CI);
private:
    CompilerInstance *CI;
};

#endif
