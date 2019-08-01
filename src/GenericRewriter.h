#ifndef REWRITER_H
#define REWRITER_H

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;

/// GenericRewrite is actually an adapter to clang::Rewriter, but since
/// multiple methods can be just passed through, we extend clang::Rewriter.
/// But GenericRewriter cannot be used in all places a clang::Rewriter is used.
class GenericRewriter : private clang::Rewriter {
public:
    using clang::Rewriter::setSourceMgr;
    using clang::Rewriter::getSourceMgr;
    using clang::Rewriter::getLangOpts;
    using clang::Rewriter::buffer_begin;
    using clang::Rewriter::buffer_end;
    using clang::Rewriter::getRewrittenText;

    bool RemoveText(SourceRange range, RewriteOptions opts = RewriteOptions());
    bool ReplaceText(SourceRange range, StringRef NewStr);
    bool InsertText(SourceLocation Loc, StringRef Str, bool InsertAfter = true, bool indentNewLines = false);

    bool startAndEndArePartOfTheSameExpansion(SourceRange range);
    bool containsMacroExpansion(SourceRange range);
    bool rangeIsFreeOfMacroExpansions(SourceRange range);
    int numberNestedMacros(SourceLocation loc);
    SourceLocation getDirectExpansionLoc(SourceLocation loc);

    /// Check if the file given by \p patchingFileName is in the "project", i.e. is the main file or lies in the source tree.
    bool considerFile(SourceLocation loc);

    void setCI(CompilerInstance *CI);
    void setFileName(std::string fileName);
    void setRootDir(std::string rootDir);
    void setFileList(std::vector<std::string> fileList);
private:
    CompilerInstance *CI;
    std::string fileName;
    std::string rootDir = "";
    std::vector<std::string> fileList;
};

#endif
