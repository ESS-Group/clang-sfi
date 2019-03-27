#ifndef _ALL_H
#define _ALL_H

#include <iostream>

#include "clang/ASTMatchers/ASTMatchers.h"

#include "FaultConstraints.h"
#include "FaultInjector.h"
#include "utils.h"

using namespace clang::ast_matchers;

#include "llvm/Support/Debug.h"
using namespace llvm;

class MIFSInjector : public FaultInjector {
  public:
    MIFSInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class SMIFSInjector : public MIFSInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MIAInjector : public FaultInjector {
  public:
    MIAInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class SMIAInjector : public MIAInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MIEBInjector : public FaultInjector {
  public:
    MIEBInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class SMIEBInjector : public MIEBInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class WAEPInjector : public FaultInjector {
  public:
    WAEPInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class WPFVInjector : public FaultInjector {
  public:
    WPFVInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MFCInjector : public FaultInjector {
  public:
    MFCInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MLOCInjector : public FaultInjector {
  public:
    MLOCInjector();
    std::string toString() override;
    // void inject(std::vector<StmtBinding> target, ASTContext &Context)
    // override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    // clang::Rewriter inject(StmtBinding current, ASTContext &Context, bool left);
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MLACInjector : public FaultInjector {
  public:
    MLACInjector();
    std::string toString() override;
    // void inject(std::vector<StmtBinding> target, ASTContext &Context)
    // override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    // clang::Rewriter inject(StmtBinding current, ASTContext &Context, bool left);
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MVIVInjector : public FaultInjector {
  public:
    MVIVInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Decl &decl, std::string binding, ASTContext &Context) override;
};
class MVIVInjectorSAFE : public FaultInjector {
  public:
    MVIVInjectorSAFE();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Decl &decl, std::string binding, ASTContext &Context) override;
};

class MVAVInjector : public FaultInjector {
  public:
    MVAVInjector(bool alsoOverwritten = false);
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};
class MVAVInjectorSAFE : public FaultInjector {
  public:
    MVAVInjectorSAFE(bool alsoOverwritten = false);
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Decl &decl, std::string binding, ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};

class OMVAVInjector : public MVAVInjector {
  public:
    OMVAVInjector();
    std::string toString() override;
};
class WVAVInjector : public FaultInjector {
  public:
    WVAVInjector(bool alsoOverwritten = false);
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};
class WVAVInjectorSAFE : public FaultInjector {
  public:
    WVAVInjectorSAFE(bool alsoOverwritten = false);
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Decl &decl, std::string binding, ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};

class OWVAVInjector : public WVAVInjector {
  public:
    OWVAVInjector();
    std::string toString() override;
};
class MVAEInjector : public FaultInjector {
  public:
    MVAEInjector(bool alsoOverwritten = false);
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};
class MVAEInjectorSAFE : public FaultInjector {
  public:
    MVAEInjectorSAFE();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Decl &decl, std::string binding, ASTContext &Context) override;

  protected:
    bool alsoOverwritten;
};

class OMVAEInjector : public MVAEInjector {
  public:
    OMVAEInjector();
    std::string toString() override;
};
class MLPAInjector : public FaultInjector {
  public:
    MLPAInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class SMLPAInjector : public MLPAInjector {
  public:
    std::string toString() override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MIESInjector : public FaultInjector {
  public:
    MIESInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

class MRSInjector : public FaultInjector {
  public:
    MRSInjector();
    std::string toString() override;
    bool inject(StmtBinding current, ASTContext &Context, clang::Rewriter &R) override;
    bool checkStmt(const Stmt &stmt, std::string binding, ASTContext &Context) override;
};

#endif
