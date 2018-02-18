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
#include "StmtHandler.h"

#include <iostream>
#include <fstream>

#include "llvm/Support/raw_ostream.h"
#include "FaultConstraints.cpp"
using namespace llvm;
//MIFS
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace std;

StmtHandler* FaultInjector::createStmtHandler(std::string binding){
    std::vector<std::string> bindings;
    bindings.push_back(binding);
    //StmtHandler ret(this,fileName,bindings/*,&FaultInjector::nodeCallback*/);
//StmtHandler(FaultInjector *pFaultInjector, std::string name, 
//std::vector<std::string> bindings, void (*nodeCallback)(std::string, const Stmt*))
    return new StmtHandler(this,fileName,bindings/*,&FaultInjector::nodeCallback*/);
}
FaultInjector::FaultInjector(){
    //Matcher = new MatchFinder();
}
FaultInjector::~FaultInjector(){
    //cout << "Deine Mudda";
    //delete Matcher;
}
FaultInjector::FaultInjector(const FaultInjector& that){
    cout << "not allowed"<<endl;
    //Matcher = that.Matcher;
    //fileName = that.fileName;
    //locations = that.locations;
    //bindings = that.bindings;
};
void FaultInjector::push(std::string binding, const Stmt *st){
    StmtBinding sb(binding, st);
    locations.push_back(sb);
    _sort();
}
/*void FaultInjector::inject(std::vector<StmtBinding> target, Rewriter &R){};//muss in jeder child Class implementiert werden
bool FaultInjector::checkStmt(const Stmt* stmt, std::string binding){
    cout<<"Found '"<< binding <<"'"<<endl;
    return true;
}//wird von jedem gematchten Statement aufgerufen und entscheidet ob dieses in locations aufgenommen werden soll
//außerdem können hier auch uU noch modifikationen vorgenommen werden
*/
void FaultInjector::matchAST(ASTContext &Context){
    //cout<<"FAULTINJECTOR::MATCHAST0"<<endl;
    Matcher.matchAST(Context);
    //cout<<"FAULTINJECTOR::MATCHAST1"<<endl;
}
void FaultInjector::setFileName(std::string name){
    //cout << "set Filename to "<<name<<endl;
    fileName = std::string(name);
}
std::string FaultInjector::getFileName(){
    std::string ret(fileName.c_str());
    //cout << fileName<<" | "<< ret <<endl;
    return ret;
}

        //Funktion wird aufgerufen, sobald komplette Datei geparst wurde => in locations stehen alle SourceRanges bei denen Injiziert werden kann
 //       std::vector<StmtBinding> locations;
        //TODO: Eventuell anstatt SourceRanges die Stmts verwenden, da eventuell noch sachen im AST gecheckt werden müssen!!
/*void FaultInjector::setSourceMgr(SourceManager &sourceManager){
    sourceMgr = &sourceManager;
}
SourceManager* FaultInjector::getSourceMgr(){
    return sourceMgr;
}*/
void FaultInjector::nodeCallback(std::string binding, const Stmt* stmt){
    push(binding, stmt);
}
//        MatchFinder Matcher;//child Classes have to add Matchers!!
void FaultInjector::_sort(){
    std::sort(
        locations.begin(),
        locations.end(),
        comparefunc
    );
}
bool FaultInjector::comparefunc(StmtBinding st1, StmtBinding st2){
    //return sr1.getBegin()<sr2.getBegin();
    return st2.stmt->getLocStart()<st1.stmt->getLocStart();
}
/*std::string FaultInjector::toString(){
    return "Nothing";
}*/

void FaultInjector::dumpStmt(const Stmt* stmt, ASTContext &Context){
    stmt->dumpPretty(Context);//printStmt(stmt, Context->getSourceManager(), Context.getLangOpts());
}
std::string FaultInjector::stmtToString(const Stmt* stmt/*, SourceManager &sourceManager*/,const LangOptions &langOpts){
    std::string statement;
    raw_string_ostream stream(statement);
    stmt->printPretty(stream, NULL, PrintingPolicy(langOpts));
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


void FaultInjector::printStep(StmtBinding current, const SourceManager &sourceManager, const LangOptions &langOpts, int i, int size){
    //StmtBinding current = target.at(i);
    cout<<"injecting '"<<toString()<<"' ["<<i+1<<"/"<<size<<"]"<<endl;
    cout << sourceRangeToString(current.stmt,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
    cout << stmtToString(current.stmt, langOpts)<<endl;
}//with printing statements
void FaultInjector::printStep(StmtBinding current,  const SourceManager &sourceManager, int i, int size){
    //StmtBinding current = target.at(i);
    cout<<"injecting '"<<toString()<<"' ["<<i+1<<"/"<<size<<"]"<<endl;
    cout << sourceRangeToString(current.stmt,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
    //cout << stmtToString(current.stmt, langOpts)<<endl;
}//only position
void FaultInjector::setVerbose(bool v){
    verbose = v;
}

void FaultInjector::setDirectory(std::string directory){
    dir = directory;
}
void FaultInjector::inject(std::vector<StmtBinding> target, ASTContext &Context){
    //cout<<"FAULTINJECTOR::INJECT:size:"<< target.size() <<endl;
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
        //i++;
    }
}
void FaultInjector::writeDown(std::string data, int i){
    std::string name = (dir.compare("")?dir+"/":"")+toString()+"_"+std::to_string(i);
    std::ofstream file(name+".cpp");
    file<<data;
    file.flush();
    file.close();
    //cout << ("diff \""+fileName+"\" \""+name+".cpp\" > \""+name+".patch\"").c_str()<<endl;
    system(("diff \""+fileName+"\" \""+name+".cpp\" > \""+name+".patch\"").c_str());
    //execl("/usr/bin/diff", "diff", "-u", "")
}

MIFSInjector::MIFSInjector(){//Missing if construct plus statements
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
}

std::string MIFSInjector::toString(){
    return "MIFS";
};
std::string MIFSInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIFSInjector::checkStmt(const Stmt* stmt, std::string binding){//no else
    //cout<<"check Node"<<endl;
    if(const IfStmt* ifS = (IfStmt *)(stmt)){
        if(!C9(ifS->getThen()))
            return false;
        return C8(ifS);
        /*if(const Stmt* Else = ifS->getElse())
            return false;
        else
            return true;
        */
    } else return false;
}



MIAInjector::MIAInjector(){//Missing if construct plus statements
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
}

std::string MIAInjector::toString(){
    return "MIA";
};
std::string MIAInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocStart().getLocWithOffset(-1));
    //SourceRange sr(IfS->getLocStart(), Else->getLocStart().getLocWithOffset(-1));//-1 Offset da sonst das erste Zeichen des Else blockes mit gelöscht wird
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIAInjector::checkStmt(const Stmt* stmt, std::string binding){//no else
    //cout<<"check Node"<<endl;
    if(const IfStmt* ifS = (IfStmt *)(stmt)){
        if(const Stmt* Else = ifS->getElse())
            return false;
        else
            return true;
    } else return false;
}


MIEBInjector::MIEBInjector(){//Missing if construct plus statements
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    Matcher.addMatcher(ifStmt().bind("ifStmt"), createStmtHandler("ifStmt"));

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
}

std::string MIEBInjector::toString(){
    return "MIEB";
};
std::string MIEBInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    /*
    const auto& parents = Context.getParents(*ifS);
    //IfStmt &ifStmt = *ifS;
    const clang::Stmt::const_child_iterator &it = ifS->getThen()->child_begin();
    StmtIterator st = cast_away_const(it);
    //auto begin = _range.begin();
    auto i = *it;
    int z = 0;
    while(st!=cast_away_const(ifS->getThen()->child_end())){
        if(!isa<NullStmt>(*st)){
            if(isa<CompoundStmt>(*st))
                cout<<"-----------------------compound"<<endl;
        z++;
        st->dump(Context.getSourceManager());
        cout<<"---------------------------childend"<<endl;
        st++;
        }
    }
    cout << z << "children"<<endl;
    */
    /*cout<<"#######################"<<endl;
    if(!parents.empty()){
        cout<<"----------------------------"<<endl;
        for(auto p : parents){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }
    } else
        cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    cout<<"#######################"<<endl;*/
    //ifS->IgnoreImplicit()->dump(Context.getSourceManager());
    
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getElse()->getLocStart().getLocWithOffset(-1));
    //SourceRange sr(IfS->getLocStart(), Else->getLocStart().getLocWithOffset(-1));//-1 Offset da sonst das erste Zeichen des Else blockes mit gelöscht wird
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MIEBInjector::checkStmt(const Stmt* stmt, std::string binding){//no else
    //cout<<"check Node"<<endl;
    
    //if(
        const IfStmt* ifS = (IfStmt *)(stmt);
    //){
        if(!C9(ifS->getThen()))
            return false;
        return !C8(ifS);
        /*if(const Stmt* Else = ifS->getElse())
            return true;
        else
            return false;*/
    //} else return false;
}
