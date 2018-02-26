/*
//ASTContext &Context
        //Rewriter rw;
        //rw.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
*/

#include "FaultInjector.h"

#include <vector>
#include <algorithm>
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"


#include "StmtHandler.h"
#include "utils.cpp"
#include "FaultConstraints.cpp"


//IO
#include <iostream>
#include <fstream>
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

StmtHandler* FaultInjector::createStmtHandler(std::string binding){
    std::vector<std::string> bindings;
    bindings.push_back(binding);
    return new StmtHandler(this,fileName,bindings);
}

bool FaultInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){
    return false;
}

bool FaultInjector::checkStmt(const Decl* stmt, std::string binding, ASTContext &Context){
    return false;
}

FaultInjector::FaultInjector(){
}

FaultInjector::~FaultInjector(){
}

void FaultInjector::push(std::string binding, const Stmt *st){
    StmtBinding sb(binding, st);
    locations.push_back(sb);
    _sort();
}
void FaultInjector::push(std::string binding, const Decl *st){
    StmtBinding sb(binding, st);
    locations.push_back(sb);
    _sort();
}

void FaultInjector::matchAST(ASTContext &Context){
    Matcher.matchAST(Context);
}

void FaultInjector::setFileName(std::string name){
    fileName = std::string(name);
}

std::string FaultInjector::getFileName(){
    std::string ret(fileName.c_str());
    return ret;
}

void FaultInjector::nodeCallback(std::string binding, const Stmt* stmt){
    push(binding, stmt);
}

void FaultInjector::nodeCallback(std::string binding, const Decl* decl){
    push(binding, decl);
}

void FaultInjector::_sort(){
    std::sort(
        locations.begin(),
        locations.end(),
        comparefunc
    );
}

bool FaultInjector::comparefunc(StmtBinding st1, StmtBinding st2){
    SourceLocation l1,l2;
    if(st1.isStmt)
        l1 = st1.stmt->getLocStart();
    else
        l1 = st1.decl->getLocStart();

    if(st2.isStmt)
        l2 = st2.stmt->getLocStart();
    else
        l2 = st2.decl->getLocStart();
    return l1<l2;//l2<l1;
}

void FaultInjector::dumpStmt(const Stmt* stmt, ASTContext &Context){
    stmt->dumpPretty(Context);
}

std::string FaultInjector::stmtToString(const Stmt* stmt, const LangOptions &langOpts){
    std::string statement;
    raw_string_ostream stream(statement);
    stmt->printPretty(stream, NULL, PrintingPolicy(langOpts));
    stream.flush();
    return statement;
}

void FaultInjector::dumpStmt(const Decl* decl){
    decl->dump();
}

std::string FaultInjector::stmtToString(const Decl* decl, const LangOptions &langOpts){
    std::string statement;
    raw_string_ostream stream(statement);
    decl->print(stream, PrintingPolicy(langOpts));
    stream.flush();
    return statement;
}

std::string FaultInjector::getEditedString(Rewriter &rewrite, ASTContext &Context){
    return rewriteBufferToString(rewrite.getEditBuffer(Context.getSourceManager().getMainFileID()));
}

std::string FaultInjector::rewriteBufferToString(RewriteBuffer &buffer){
    std::string str;
    raw_string_ostream stream(str);
    buffer.write(stream);
    stream.flush();
    return str;
}

std::string FaultInjector::sourceLocationToString(SourceLocation loc,const SourceManager &sourceManager){
    return loc.printToString(sourceManager);
}

std::string FaultInjector::sourceRangeToString(SourceRange range,const SourceManager &sourceManager){
    return sourceLocationToString(range.getBegin(), sourceManager) + " - " + sourceLocationToString(range.getEnd(),sourceManager);
}

std::string FaultInjector::sourceRangeToString(const Stmt *stmt,const SourceManager &sourceManager){
    return sourceRangeToString(stmt->getSourceRange(), sourceManager);
}

std::string FaultInjector::sourceRangeToString(const Decl *decl,const SourceManager &sourceManager){
    return sourceRangeToString(decl->getSourceRange(), sourceManager);
}


void FaultInjector::printStep(StmtBinding current, const SourceManager &sourceManager, const LangOptions &langOpts, int i, int size){
    cout<<"injecting '"<<toString()<<"' ["<<i+1<<"/"<<size<<"]"<<endl;
    if(current.isStmt){
        cout << sourceRangeToString(current.stmt,sourceManager)<<endl;
        cout << stmtToString(current.stmt, langOpts)<<endl;
    } else {
        cout << sourceRangeToString(current.decl,sourceManager)<<endl;
        cout << stmtToString(current.decl, langOpts)<<endl;
    }
}//with printing statements

void FaultInjector::printStep(StmtBinding current,  const SourceManager &sourceManager, int i, int size){
    cout<<"injecting '"<<toString()<<"' ["<<i+1<<"/"<<size<<"]"<<endl;
    if(current.isStmt){
        cout << sourceRangeToString(current.stmt,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
    } else {
        cout << sourceRangeToString(current.decl,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
    }
}//only position

void FaultInjector::setVerbose(bool v){
    verbose = v;
}

void FaultInjector::setDirectory(std::string directory){
    dir = directory;
}
void FaultInjector::inject(std::vector<StmtBinding> target, ASTContext &Context){
    int i = 0;
    
    for(StmtBinding current : target){
        if(verbose)
            printStep(current, Context.getSourceManager(), Context.getLangOpts(),i++,target.size());
        else
            printStep(current, Context.getSourceManager(),i++,target.size());
        std::string result = inject(current, Context);
        if(result.compare("")){
            cout<<" -Success"<<endl;
            writeDown(result, i-1);
        } else
            cerr << "-Failed"<<endl;
    }
}
void FaultInjector::writeDown(std::string data, int i){
    std::string name = (dir.compare("")?dir+"/":"")+toString()+"_"+std::to_string(i);
    std::ofstream file(name+".cpp");
    file<<data;
    file.flush();
    file.close();
    system(("diff \""+fileName+"\" \""+name+".cpp\" > \""+name+".patch\"").c_str());
}




//Algorithm faults
#include "FaultInjectors/MFCInjector.cpp"
#include "FaultInjectors/MIFSInjector.cpp"
#include "FaultInjectors/MIEBInjector.cpp"
//todo: MLPA

//Checking faults
#include "FaultInjectors/MIAInjector.cpp"
#include "FaultInjectors/MLACInjector.cpp"
#include "FaultInjectors/MLOCInjector.cpp"

//Assignement faults
#include "FaultInjectors/MVIVInjector.cpp"
#include "FaultInjectors/MVAVInjector.cpp"
#include "FaultInjectors/MVAEInjector.cpp"
#include "FaultInjectors/WVAVInjector.cpp"
//todo: WVAV

//Interface faults
//todo: WPFV
#include "FaultInjectors/WAEPInjector.cpp"
