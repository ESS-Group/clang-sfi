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

bool FaultInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    return false;
}

bool FaultInjector::checkStmt(const Decl* stmt, std::string binding, ASTContext &Context){//no else
    return false;
}
FaultInjector::FaultInjector(){
    //Matcher = new MatchFinder();
}
FaultInjector::~FaultInjector(){
    //delete Matcher;
}
//FaultInjector::FaultInjector(const FaultInjector& that){
//    cout << "not allowed"<<endl;
    //Matcher = that.Matcher;
    //fileName = that.fileName;
    //locations = that.locations;
    //bindings = that.bindings;
//};
void FaultInjector::push(std::string binding, const Stmt *st){
    StmtBinding sb(binding, st);
    locations.push_back(sb);
    _sort();
}
void FaultInjector::push(std::string binding, const Decl *st){
    //cout<<"push0"<<endl;
    StmtBinding sb(binding, st);
    //cout<<"push1"<<endl;
    locations.push_back(sb);
    //cout<<"push2"<<endl;
    //st->dumpColor();
    _sort();
    //cout<<"push3"<<endl;
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

void FaultInjector::nodeCallback(std::string binding, const Decl* decl){
    push(binding, decl);
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
    //cout<<"compare"<<endl;
    //cout<<(st1.isStmt?1:0)<<(st2.isStmt?1:0)<<endl;
    SourceLocation l1,l2;
    if(st1.isStmt)
        l1 = st1.stmt->getLocStart();
    else
        l1 = st1.decl->getLocStart();

    if(st2.isStmt)
        l2 = st2.stmt->getLocStart();
    else
        l2 = st2.decl->getLocStart();
    //if()
    return l2<l1;
    //return st2.stmt->getLocStart()<st1.stmt->getLocStart();
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
void FaultInjector::dumpStmt(const Decl* decl){
    decl->dump();//printStmt(stmt, Context->getSourceManager(), Context.getLangOpts());
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
    //StmtBinding current = target.at(i);
    cout<<"injecting '"<<toString()<<"' ["<<i+1<<"/"<<size<<"]"<<endl;
    if(current.isStmt){
        cout << sourceRangeToString(current.stmt,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
        cout << stmtToString(current.stmt, langOpts)<<endl;
    } else {
        cout << sourceRangeToString(current.decl,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
        cout << stmtToString(current.decl, langOpts)<<endl;
    }
}//with printing statements
void FaultInjector::printStep(StmtBinding current,  const SourceManager &sourceManager, int i, int size){
    //StmtBinding current = target.at(i);
    cout<<"injecting '"<<toString()<<"' ["<<i+1<<"/"<<size<<"]"<<endl;
    if(current.isStmt){
        cout << sourceRangeToString(current.stmt,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
    } else {
        cout << sourceRangeToString(current.decl,sourceManager)<<endl;//current.stmt -> getLocStart().printToString(Context.getSourceManager())<< " - "<<current.stmt -> getLocEnd().printToString(Context.getSourceManager())<<endl;
    }
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
bool MIFSInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    //cout<<"check Node"<<endl;
    if(const IfStmt* ifS = (IfStmt *)(stmt)){
        if(!C9(ifS->getThen()))
            return false;
        return C8(ifS) && C2(stmt, Context);
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
bool MIAInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
       if(const IfStmt* ifS = (IfStmt *)(stmt)){
        if(!C9(ifS->getThen()))
            return false;
        return C8(ifS);
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
bool MIEBInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
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





MFCInjector::MFCInjector(){
    Matcher.addMatcher(callExpr(unless(anyOf(hasParent(varDecl(isDefinition())), hasParent(returnStmt()), cxxOperatorCallExpr(), hasParent(callExpr()), hasParent(expr())))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    //Matcher.addMatcher(callExpr().bind("FunctionCall"), createStmtHandler("FunctionCall"));
}

std::string MFCInjector::toString(){
    return "MFC";
};
std::string MFCInjector::inject(StmtBinding current, ASTContext &Context){

    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MFCInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else

    //if(locations.size()==2)
    //    return false;
    //cout<<"-----"<<endl;
        //stmt->getLocStart().dump(Context.getSourceManager());
    //cout<<childCount(stmt)<<"+++++"<<Context.getParents(*stmt).size()<< (((const CallExpr*)stmt)->isValueDependent()?"5":"6") <<endl;
        //((const CallExpr*)stmt)->IgnoreImplicit()->dump(Context.getSourceManager());
        //stmt->child_begin()->dump(Context.getSourceManager());
        //if(Context.getParents(*stmt)[0].get<CallExpr>() != 0)
        //Context.getParents(*stmt)[0].get<CallExpr>()->getCalleeDecl()->dump();
        //((const CallExpr*)stmt) ->getCalleeDecl()->dump();
    //cout<<"-----"<<endl;
    return true;
}





void MLACInjector::inject(std::vector<StmtBinding> target, ASTContext &Context){
    //cout<<"FAULTINJECTOR::INJECT:size:"<< target.size() <<endl;
    int i = 0;
    for(StmtBinding current : target){
        if(verbose)
            printStep(current, Context.getSourceManager(), Context.getLangOpts(),2*(i++),target.size()*2);
        else
            printStep(current, Context.getSourceManager(),i++,target.size()*2);
        std::string result = inject(current, Context, true);
        if(result.compare("")){
            cout<<" -Success"<<endl;
            writeDown(result, i-1);
        } else
            cerr << "-Failed"<<endl;


        if(verbose)
            printStep(current, Context.getSourceManager(), Context.getLangOpts(),2*(i++),target.size()*2);
        else
            printStep(current, Context.getSourceManager(),i++,target.size()*2);
        result = inject(current, Context, false);
        if(result.compare("")){
            cout<<" -Success"<<endl;
            writeDown(result, i-1);
        } else
            cerr << "-Failed"<<endl;
        //i++;
    }
}
std::string MLACInjector::inject(StmtBinding current, ASTContext &Context){
    return "";
}
MLACInjector::MLACInjector(){
    //anyOf(ifStmt(), doStmt(), switchStmt(), whileStmt())
    //hasAncestor(ifStmt())
    Matcher.addMatcher(binaryOperator(anyOf(hasAncestor(expr(anyOf(hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt())))),hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt()))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    //Matcher.addMatcher(callExpr().bind("FunctionCall"), createStmtHandler("FunctionCall"));
}

std::string MLACInjector::toString(){
    return "MLAC";
};
std::string MLACInjector::inject(StmtBinding current, ASTContext &Context, bool left){

    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceLocation start, end;
    if(left){
        start = ((const BinaryOperator *)current.stmt)->getOperatorLoc().getLocWithOffset(-1);//.getLocWithOffset(1);
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();
    }else {
        start = ((const BinaryOperator *)current.stmt)->getLHS()->getLocStart();
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocStart().getLocWithOffset(-1);
    }


    SourceRange range(start, end);
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MLACInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    return ((const BinaryOperator *)stmt)->getOpcode() == 18;
}
MLOCInjector::MLOCInjector(){
    //anyOf(ifStmt(), doStmt(), switchStmt(), whileStmt())
    //hasAncestor(ifStmt())
    Matcher.addMatcher(binaryOperator(anyOf(hasAncestor(expr(anyOf(hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt())))),hasParent(ifStmt()),hasParent(doStmt()),hasParent(switchStmt()),hasParent(whileStmt()))).bind("FunctionCall"), createStmtHandler("FunctionCall"));
    //Matcher.addMatcher(callExpr().bind("FunctionCall"), createStmtHandler("FunctionCall"));
}

std::string MLOCInjector::toString(){
    return "MLOC";
};
std::string MLOCInjector::inject(StmtBinding current, ASTContext &Context, bool left){

    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceLocation start, end;
    if(left){
        start = ((const BinaryOperator *)current.stmt)->getOperatorLoc().getLocWithOffset(-1);//.getLocWithOffset(1);
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocEnd();
    }else {
        start = ((const BinaryOperator *)current.stmt)->getLHS()->getLocStart();
        end = ((const BinaryOperator *)current.stmt)->getRHS()->getLocStart().getLocWithOffset(-1);
    }


    SourceRange range(start, end);
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MLOCInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else

    return ((const BinaryOperator *)stmt)->getOpcode() == 19;
}
void MLOCInjector::inject(std::vector<StmtBinding> target, ASTContext &Context){
    //cout<<"FAULTINJECTOR::INJECT:size:"<< target.size() <<endl;
    int i = 0;
    for(StmtBinding current : target){
        if(verbose)
            printStep(current, Context.getSourceManager(), Context.getLangOpts(),2*(i++),target.size()*2);
        else
            printStep(current, Context.getSourceManager(),i++,target.size()*2);
        std::string result = inject(current, Context, true);
        if(result.compare("")){
            cout<<" -Success"<<endl;
            writeDown(result, i-1);
        } else
            cerr << "-Failed"<<endl;


        if(verbose)
            printStep(current, Context.getSourceManager(), Context.getLangOpts(),2*(i++),target.size()*2);
        else
            printStep(current, Context.getSourceManager(),i++,target.size()*2);
        result = inject(current, Context, false);
        if(result.compare("")){
            cout<<" -Success"<<endl;
            writeDown(result, i-1);
        } else
            cerr << "-Failed"<<endl;
        //i++;
    }
}
std::string MLOCInjector::inject(StmtBinding current, ASTContext &Context){
    return "";
}





const CompoundStmt* getParentCompoundStmt(const Stmt *stmt, ASTContext &Context){
    ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
    //cout << list.size() << " Parents";
    /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    if(!list.empty()){
        if(list[0].get<Stmt>()!=NULL){
            if(isa<CompoundStmt>(list[0].get<Stmt>())){
                const CompoundStmt* container = list[0].get<CompoundStmt>();
                return container;
            } else return NULL;
        }
        /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    } //else
        //return false;
        //cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    //cout<<"#######################"<<endl;
    return NULL;
}
const CompoundStmt* getParentCompoundStmt(const Decl *decl, ASTContext &Context){
    ASTContext::DynTypedNodeList list = Context.getParents(*decl);
    //cout << list.size() << " Parents";
    //cout<<"----------------------------"<<endl;
      //  for(auto p : list){
        //    p.get<Stmt>()->dump(Context.getSourceManager());
        //}
    if(!list.empty()){
        if(list[0].get<Stmt>() != NULL){
            if(isa<CompoundStmt>(list[0].get<Stmt>())){
                const CompoundStmt* container = list[0].get<CompoundStmt>();
                return container;
            } else if(isa<DeclStmt>(list[0].get<Stmt>())){
                return getParentCompoundStmt(list[0].get<Stmt>(), Context);
            } else return NULL;
        }
        /*cout<<"----------------------------"<<endl;
        for(auto p : list){
            p.get<Stmt>()->dump(Context.getSourceManager());
        }*/
    } //else
        //return false;
        //cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl;
    //cout<<"#######################"<<endl;
    return NULL;
}
template<class T>
void concatVector(std::vector<T> &dst, std::vector<T> &src){
    dst.insert(dst.end(), src.begin(), src.end());
}
bool isAssignment(const BinaryOperator* op){
    return op->getOpcode()==20;
}
//isValueDeclaration
bool isValueAssignment(const BinaryOperator* op){
    if(isAssignment(op)){
        //Stmt::child_iterator i = cast_away_const(op->child_begin());
        const Stmt *stmt = op->getRHS();
        if(stmt!=NULL && (isa<IntegerLiteral>(stmt) || isa<CXXBoolLiteralExpr>(stmt) || isa<CharacterLiteral>(stmt) || isa<FloatingLiteral>(stmt) || isa<clang::StringLiteral>(stmt) ))
            return true;
        else
            return false;
    }else
        return false;
}

bool isExprAssignment(const BinaryOperator* op){
    return isAssignment(op) && !isValueAssignment(op);
}

template<class T>
const T* getFirstChild(const Stmt *parent){
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i != NULL){
            if(isa<T>(*i)){
                return (const T *) *i;
            } else {
                if(const T* ret = getFirstChild<T>(*i))
                    return ret; 
            }
        }
    }
    return NULL;
}
/*
std::string stmtToString(const Decl* decl, const LangOptions &langOpts){
    std::string statement;
    raw_string_ostream stream(statement);
    decl->print(stream, PrintingPolicy(langOpts));
    stream.flush();
    return statement;
}*/

std::vector<const BinaryOperator*> getChildForFindInitForVar(const Stmt *parent, const VarDecl* var, bool alsoinloop = false){
    std::vector<const BinaryOperator*> ret;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == NULL){

            //cout << "getChildForFindInitForVar >> Found NULL" << endl;
        }else if(isa<BinaryOperator>(*i)){
            ////cout << "getChildForFindInitForVar >> Found BinaryOperator" << endl;
            if(isAssignment((const BinaryOperator*)*i)){
                ////cout << "getChildForFindInitForVar >> Found BinaryOperator >> isValueDeclaration" << endl;
                //i->dumpColor();
                if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(*i)){
                    ////exp->dumpColor();
                    ////var->dumpColor();
                    //cout << var->getName().data()<<endl;
                    if(exp->getDecl() == var){
                        ret.push_back((const BinaryOperator*) *i);
                        break;
                    }
                }
            }
        }else if(isa<Stmt>(*i) && *i != NULL){
            if(isa<IfStmt>(*i)){

                //cout << "getChildForFindInitForVar >> Found IfStmt" << endl;
                IfStmt *ifS = (IfStmt *)*i;

                std::vector<const BinaryOperator*> inThen = getChildForFindInitForVar(ifS->getThen(), var);
                if(inThen.size()!=0){
                    concatVector<const BinaryOperator*>(ret,inThen);
                }
                if(const Stmt* elseS = ifS->getElse()){
                    std::vector<const BinaryOperator*> inElse = getChildForFindInitForVar(elseS, var);
                    if(inElse.size()!=0){
                        concatVector<const BinaryOperator*>(ret,inElse);
                        if(inThen.size()!=0)
                            break;//initialization in both
                    }
                }
            }else if(alsoinloop || (!isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i))){

                //cout << "getChildForFindInitForVar >> Found Other" << endl;
                std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(*i, var), inloop;
                if(temp.size()!=0){
                    concatVector<const BinaryOperator*>(ret,temp);
                    break;
                }
                
            }
        }
    }
    return ret;
}







std::vector<const BinaryOperator*> getChildForFindVarAssignment(const Stmt *parent, const VarDecl* var, bool alsoinloop = true){
    std::vector<const BinaryOperator*> ret;
    if(parent == NULL)
        return ret;
    //parent->dumpColor();
    //cout<<"huhu"<<endl;
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == NULL){

            //cout << "getChildForFindInitForVar >> Found NULL" << endl;
        }else if(isa<BinaryOperator>(*i)){
            //cout << "getChildForFindInitForVar >> Found BinaryOperator" << endl;
            if(isAssignment((const BinaryOperator*)*i)){
                //cout << "getChildForFindInitForVar >> Found BinaryOperator >> isValueDeclaration" << endl;
                //i->dumpColor();
                //cout << "test1"<<endl;
                if(const DeclRefExpr* exp = getFirstChild<DeclRefExpr>(*i)){
                    //exp->dumpColor();
                    //var->dumpColor();
                    //cout << var->getName().data()<<endl;

                    //cout << "test2"<<endl;
                    if(exp->getDecl() == var){

                        //cout << "test3"<<endl;
                        ret.push_back((const BinaryOperator*) *i);
                        //break;
                    }
                }

                //cout << "test4"<<endl;
            }
        }else if(isa<Stmt>(*i) && *i != NULL){
            if(isa<IfStmt>(*i)){

                //cout << "getChildForFindInitForVar >> Found IfStmt" << endl;
                IfStmt *ifS = (IfStmt *)*i;

                std::vector<const BinaryOperator*> inThen = getChildForFindInitForVar(ifS->getThen(), var,alsoinloop);
                if(inThen.size()!=0){
                    concatVector<const BinaryOperator*>(ret,inThen);
                }
                if(const Stmt* elseS = ifS->getElse()){
                    std::vector<const BinaryOperator*> inElse = getChildForFindInitForVar(elseS, var, alsoinloop);
                    if(inElse.size()!=0){
                        concatVector<const BinaryOperator*>(ret,inElse);
                        //if(inThen.size()!=0)
                            //break;//initialization in both
                    }
                }
            }else if(alsoinloop){
                if(isa<ForStmt>(*i)){

                    //cout <<"Search in for"<<endl;    
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(((const ForStmt*)*i)->getBody(), var, alsoinloop);
                    //cout <<temp.size()<<endl;
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        /*for(auto a : temp){
                            a->dumpColor();
                        }*/
                        //break;
                    }
                }else if(isa<WhileStmt>(*i)){
                    //cout <<"Search in while"<<endl;
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(((const WhileStmt*)*i)->getBody(), var, alsoinloop);
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        //break;
                    }
                } else if(isa<DoStmt>(*i)){
                    //cout <<"Search in do"<<endl;
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(((const DoStmt*)*i)->getBody(), var, alsoinloop);
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        //break;
                    }
                } else {
                    std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(*i, var, alsoinloop);
                    if(temp.size()!=0){
                        concatVector<const BinaryOperator*>(ret,temp);
                        //break;
                    }
                }
                //cout << "getChildForFindInitForVar >> Found Other" << endl;
                
            }else if((!isa<ForStmt>(*i) && !isa<WhileStmt>(*i) && !isa<DoStmt>(*i))){
                std::vector<const BinaryOperator*> temp = getChildForFindInitForVar(*i, var, alsoinloop);
                if(temp.size()!=0){
                    concatVector<const BinaryOperator*>(ret,temp);
                    //break;
                }
            }
        }
    }
    return ret;
}
/*
std::vector<const BinaryOperation*> findInitForVar(const Stmt* parent, const VarDecl* var, bool alsoinloop=false){
    std::vector<const BinaryOperator*> ret;
    if(alsoinloop || (!isa<ForStmt>(parent) && !isa<WhileStmt>(parent) && !isa<DoStmt>(parent))){
        for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
            Stmt *stmt = *i;
            if(stmt != NULL){
                if(isa<BinaryOperator>(stmt)){
                    if(((const BinaryOperation *)stmt)->getOpcode()==20&&(const DeclRefExpr* declE =getChildForFindInitForVar(stmt, var, alsoinloop)));
                        ret.push_back(stmt);
                        break;
                    }
                }
            }
        }
    }
    return ret;
}
*/

template<class T>
const T* getParentOfType(const Stmt* stmt, ASTContext &Context, int maxDepth = 1){//MaxDepth = -1 for to the root
    T* ret;

    //cout<<"+line1"<<endl;
    if(stmt==NULL)
        return NULL;
    if(maxDepth!=0){
        //cout<<"+line2"<<endl;
        ASTContext::DynTypedNodeList list = Context.getParents(*stmt);
        //cout<<"+line3"<<endl;
        for(auto p : list){
            //cout<<"+line4"<<endl;
            if(isa<T>(p.get<Stmt>())){
                //cout<<"+line5.1.1"<<endl;
                return p.get<T>();
                //cout<<"+line5.1.2"<<endl;
            }else if(ret == NULL){
                //cout<<"+line5.2.1"<<endl;
                ret = (T*)(&(*getParentOfType<T>(p.get<T>(), Context, maxDepth-1)));
                //cout<<"+line5.2.2"<<endl;
            }
        }
    }
    //cout<<"+line6"<<endl;
    if(ret == NULL)
        return NULL;
    else
        return const_cast<const T*>(ret);
}

//getParentOfType<ForStmt>(decl,Context,3)
template<class T>
const T* getParentOfType(const Decl* decl, ASTContext &Context, int maxDepth = 1){//MaxDepth = -1 for to the root
    T* ret = NULL;
    //cout<<"-line1"<<endl;
    if(maxDepth!=0){
        ASTContext::DynTypedNodeList list = Context.getParents(*decl);

        //cout<<"-line2"<<endl;
        for(auto p : list){
            //cout<<"-line3"<<endl;
            if(isa<T>(p.get<Stmt>())){

            //cout<<"-line3.1.1"<<endl;
                return p.get<T>();
            //cout<<"-line3.1.2"<<endl;
            }else if(ret == NULL){
                //cout<<"-line3.2.1"<<endl;
                ret = (T*)(&(*getParentOfType<T>(p.get<T>(), Context, maxDepth-1)));
                //cout<<"-line3.2.2"<<endl;
            }
            //cout<<"-line4"<<endl;
        }
    }
    //cout <<"-line5"<<endl;
    
    if(ret == NULL){
        //cout <<"-line6.1"<<endl;
        return NULL;
    }else{
        //cout <<"-line6.2"<<endl;
        return const_cast<const T*>(ret);
    }
}




bool isParentOf(const Stmt* parent, const Stmt* stmt){
    for(Stmt::child_iterator i = cast_away_const(parent->child_begin()), e = cast_away_const(parent->child_end());i!=e;++i){
        if(*i == stmt)
            return true;
        if(isParentOf(*i, stmt))
            return true;
    }
    return false;
}
bool isParentOf(const Stmt* parent, const Decl* decl, ASTContext &Context){
    const DeclStmt* stmt = getParentOfType<DeclStmt>(decl,Context,3);
    if(stmt==NULL)
        return false;
    else
        return isParentOf(parent,getParentOfType<DeclStmt>(decl,Context,3));
    //bool ret = false;
    /*
    ASTContext::DynTypedNodeList list = Context.getParents(decl);
    if(list.empty() || list[0].get<Stmt>() == NULL)
        return false;
    return isParentOf(parent,(const Stmt *)list[0].get<Stmt>());
    */
}

MVIVInjector::MVIVInjector(){//Missing if construct plus statements
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    //anyOf(hasParent(anyOf(forStmt(), whileStmt(),doStmt()))
    Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(anyOf(
                callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
                hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
            )),
            hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));

    Matcher.addMatcher(
        //callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
            varDecl(
                    allOf(
                    unless(anyOf(
                        hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
                    )),
                    hasAncestor(compoundStmt()),
                    unless(varDecl(hasInitializer(expr())))
                    )
            ).bind("notInitialized"), createStmtHandler("notInitialized")); // in this case get next assignement

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
}

std::string MVIVInjector::toString(){
    return "MVIV";
};
std::string MVIVInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    //return "";
    //((const VarDecl*)current.decl)->getAnyInitializer()->dump(Context.getSourceManager());
    //((const VarDecl*)current.decl)->setInit(NULL);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if(current.isStmt){
        //const Stmt *temp = (const Stmt*)current.stmt;
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        VarDecl temp (*((const VarDecl*)current.decl));
        temp.setInit(NULL);
        const VarDecl* tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());

        
        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
        R.ReplaceText(range, withoutInit);
    }

    return getEditedString(R, Context);
    /*
    return "";
    
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
    */
}



/*
std::vector<const BinaryOperator*> getInitializationOfUninitializedVar(const VarDecl* var, ASTContext &Context){
    std::vector<const BinaryOperator*> ret;
    const CompoundStmt *scope = getParentCompoundStmt(var, Context);

}
*/
//int i = 0;
bool MVIVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
    if(binding.compare("notInitialized") == 0 && isa<VarDecl>(decl) /*&&i++ == 0*/){
        //if(decl != NULL)
        //cout << "DECL: " << stmtToString (decl, Context.getLangOpts())<<endl;
        //getParentCompoundStmt(decl, Context)->dump(Context.getSourceManager());
        std::vector<const BinaryOperator*> list = getChildForFindInitForVar(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, false);
        //cout << "SIZE: "<<list.size()<<endl;
        for(const BinaryOperator* op:list){
            
            if(isValueAssignment(op) && C2(op, Context)){
                //op->dump(Context.getSourceManager());
                nodeCallback(binding, op);
            }
            //cout<<"-----------"<<endl;
        }

        return false;
        //cout<<stmtToString(decl, Context.getLangOpts())<<endl;

        //cout<<"------------"<<(((const VarDecl*)decl)->hasInit()?1:0)<<endl;
        ((const VarDecl*)decl)->getInit()->dump(Context.getSourceManager());
        //decl->getDeclContext()->dumpDeclContext();
    }else
        return C2(decl, Context);
    //cout<<stmtToString(decl, Context.getLangOpts())<<endl;
    
    //return false;
}












MVAVInjector::MVAVInjector(){
    /*
    //StmtHandler handler = createStmtHandler("ifStmt");
    //test.push_back(handler);
    Matcher.addMatcher(stmt(allOf(binaryOperator(hasOperatorName("=")),
    unless(anyOf(hasDescendant(callExpr()),allOf(hasDescendant(binaryOperator()), unless(hasDescendant(binaryOperator(hasOperatorName("="))))),hasDescendant(unaryOperator()),hasDescendant(cxxNewExpr()),hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt()))))).bind("variable"), createStmtHandler("variable"));
    //hasDescendant(unaryOperator())
    //allOf(hasDescendant(binaryOperator()), unless(hasDescendant(binaryOperator(hasOperatorName("=")))))
    //hasDescendant(callExpr()),hasDescendant(cxxNewExpr()),hasDescendant(binaryOperator()),hasDescendant(unaryOperator())
    Matcher.addMatcher(varDecl(allOf(hasInitializer(unless(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator()))),
                        unless(anyOf(hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt()))))).bind("variable1"), createStmtHandler("variable1"));

    //cout<<"MIFSINJECTOR:CONSTRUCTOR" <<endl;
    */
    /*Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(anyOf(
                callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
                hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
            )),
            hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));
        /**//*Matcher.addMatcher(
        varDecl(hasInitializer(
            allOf(
            unless(anyOf(
                callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator()//,
                //hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
            )
            ),
            //unless(hasParent(forStmt())),
            hasAncestor(compoundStmt())
            )
        )).bind("variable"), createStmtHandler("variable"));
/**/
        Matcher.addMatcher(
        varDecl(//allOf(
                hasInitializer(
                    expr(unless(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),unless(hasAncestor(compoundStmt())))))
                    // allOf(
                    /*unless(anyOf(
                        callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),unless(hasAncestor(compoundStmt()))//,
                        //hasAncestor(forStmt()),hasAncestor(doStmt()),hasAncestor(whileStmt())
                    )
                    //),
                    //unless(hasParent(forStmt())),
                    
                    )*/
                )//,
                //hasAncestor(compoundStmt())
            ).bind("variable"), createStmtHandler("variable"));
/**/



    Matcher.addMatcher(
        //callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator(),
            varDecl(
                    //allOf(
                    //unless(hasInitializer(expr())),
                    hasAncestor(compoundStmt())
                    //,unless(varDecl(hasInitializer(expr())))
                    //)
            ).bind("notInitialized"), createStmtHandler("notInitialized")); // in this case get next assignement

}

std::string MVAVInjector::toString(){
    return "MVAV";
};




std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    //return "";
    //((const VarDecl*)current.decl)->getAnyInitializer()->dump(Context.getSourceManager());
    //((const VarDecl*)current.decl)->setInit(NULL);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if(current.isStmt){
        //const Stmt *temp = (const Stmt*)current.stmt;
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        VarDecl temp (*((const VarDecl*)current.decl));
        temp.setInit(NULL);
        const VarDecl* tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());

        
        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
        R.ReplaceText(range, withoutInit);
    }

    return getEditedString(R, Context);
}


//int i = 0;
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
//cout << "oha"<<++i<<endl;
//i++;
//decl -> dumpColor();
//cout << i<<endl;
    if(binding.compare("notInitialized") == 0 && isa<VarDecl>(decl) /*&& false /**//*&&i++ == 0*/){
        /*if(decl != NULL)
            cout << "DECL: " << stmtToString (decl, Context.getLangOpts())<<endl;
        else
            cout << "ERROR"<<endl;*/
        //getParentCompoundStmt(decl, Context)->dump(Context.getSourceManager());
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        //cout << "SIZE: "<<list.size()<<endl;
        for(const BinaryOperator* op:list){
            
            if(isValueAssignment(op)){
                //op->dump(Context.getSourceManager());
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){

            //cout<<"-----------4"<<endl;
                    if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){

            //cout<<"-----------5"<<endl;
                    } else if(C2(op, Context)){

            //cout<<"-----------6"<<endl;
                        nodeCallback(binding, op);
                    }
                } else if(C2(op, Context)){

            //cout<<"-----------7"<<endl;
                    nodeCallback(binding, op);
                }
                
            }
            //cout<<"-----------10"<<endl;
        }

        return false;
        /*cout<<stmtToString(decl, Context.getLangOpts())<<endl;

        cout<<"------------"<<(((const VarDecl*)decl)->hasInit()?1:0)<<endl;
        ((const VarDecl*)decl)->getInit()->dump(Context.getSourceManager());*/
        //decl->getDeclContext()->dumpDeclContext();
    }else{
        //return false;
        //cout << "MVAV TYPE2 - 1"<<i<<endl;
        //decl->dumpColor();
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){

            //cout << "MVAV TYPE2 - 2"<<i << endl;
            if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){

            } else {

            //cout << "MVAV TYPE2 - 3"<<i << endl;
               return C2(decl, Context); 
            }

            //cout << "MVAV TYPE2 - 4"<<i << endl;
        } else { 

            //cout << "MVAV TYPE2 - 5"<<i << endl;
            return C2(decl, Context);
        }
    }
    //cout<<stmtToString(decl, Context.getLangOpts())<<endl;
    
    return false;
}

/*
std::string MVAVInjector::inject(StmtBinding current, ASTContext &Context){

    //cout<<"Bearbeite "<<current.binding<<endl;
    return "";
    const IfStmt* ifS = (IfStmt *)(current.stmt);
    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    SourceRange range(ifS->getLocStart(), ifS->getThen()->getLocEnd());
    R.RemoveText(range);
    return getEditedString(R, Context);
}
bool MVAVInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
    cout<<"decl - "<<stmtToString(decl, Context.getLangOpts())<<endl;
    
    return false;
}*/
/*
bool MVAVInjector::checkStmt(const Stmt* stmt, std::string binding, ASTContext &Context){//no else
    
    const BinaryOperator* op = (const BinaryOperator*)stmt;
    //if(op->getOpcode()==20){//=
    cout<<stmtToString(op, Context.getLangOpts())<<endl;
    //}
    return false;
}
*/





MVAEInjector::MVAEInjector(){
        Matcher.addMatcher(
        varDecl(
                hasInitializer(
                    allOf(
                        expr(anyOf(callExpr(),cxxNewExpr(),binaryOperator(),unaryOperator())),
                        hasAncestor(compoundStmt())
                    )
                )
            ).bind("variable"), createStmtHandler("variable"));




    Matcher.addMatcher(
            varDecl(
                    hasAncestor(compoundStmt())
            ).bind("notInitialized"), createStmtHandler("notInitialized"));

}

std::string MVAEInjector::toString(){
    return "MVAE";
};




std::string MVAEInjector::inject(StmtBinding current, ASTContext &Context){

    Rewriter R;
    R.setSourceMgr(Context.getSourceManager(), Context.getLangOpts());
    if(current.isStmt){
        SourceRange range(current.stmt->getLocStart(), current.stmt->getLocEnd());
        R.RemoveText(range);
    } else {
        VarDecl temp (*((const VarDecl*)current.decl));
        temp.setInit(NULL);
        const VarDecl* tempP = &temp;
        std::string withoutInit = stmtToString(tempP, Context.getLangOpts());

        
        SourceRange range(current.decl->getLocStart(), current.decl->getLocEnd());
        R.ReplaceText(range, withoutInit);
    }

    return getEditedString(R, Context);
}


//int i = 0;
bool MVAEInjector::checkStmt(const Decl* decl, std::string binding, ASTContext &Context){//no else
//cout << "oha"<<++i<<endl;
//i++;
//decl -> dumpColor();
//cout << i<<endl;
    if(binding.compare("notInitialized") == 0 && isa<VarDecl>(decl) /*&& false /**//*&&i++ == 0*/){
        /*if(decl != NULL)
            cout << "DECL: " << stmtToString (decl, Context.getLangOpts())<<endl;
        else
            cout << "ERROR"<<endl;*/
        //getParentCompoundStmt(decl, Context)->dump(Context.getSourceManager());
        std::vector<const BinaryOperator*> list = getChildForFindVarAssignment(getParentCompoundStmt(decl, Context), (const VarDecl*)decl, true);
        //cout << "SIZE: "<<list.size()<<endl;
        for(const BinaryOperator* op:list){
            
            if(isExprAssignment(op)){
                //op->dump(Context.getSourceManager());
                if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){

            //cout<<"-----------4"<<endl;
                    if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){

            //cout<<"-----------5"<<endl;
                    } else if(C2(op, Context)){

            //cout<<"-----------6"<<endl;
                        nodeCallback(binding, op);
                    }
                } else if(C2(op, Context)){

            //cout<<"-----------7"<<endl;
                    nodeCallback(binding, op);
                }
                
            }
            //cout<<"-----------10"<<endl;
        }

        return false;
        /*cout<<stmtToString(decl, Context.getLangOpts())<<endl;

        cout<<"------------"<<(((const VarDecl*)decl)->hasInit()?1:0)<<endl;
        ((const VarDecl*)decl)->getInit()->dump(Context.getSourceManager());*/
        //decl->getDeclContext()->dumpDeclContext();
    }else{
        //return false;
        //cout << "MVAE TYPE2 - 1"<<endl;
        //decl->dumpColor();
        if(const ForStmt* forstmt = getParentOfType<ForStmt>(decl,Context,3)){

            //cout << "MVAV TYPE2 - 2"<<i << endl;
            if(isParentOf(forstmt->getCond(), decl, Context) || isParentOf(forstmt->getInc(), decl,Context)){

            } else {

            //cout << "MVAV TYPE2 - 3"<<i << endl;
               return C2(decl, Context); 
            }

            //cout << "MVAV TYPE2 - 4"<<i << endl;
        } else { 

            //cout << "MVAV TYPE2 - 5"<<i << endl;
            return C2(decl, Context);
        }
    }
    //cout<<stmtToString(decl, Context.getLangOpts())<<endl;
    
    return false;
}