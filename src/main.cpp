#include <iostream>
#include <sstream>
#include <string>
//include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
//include "SFIAction.h"
#include "StmtHandler.h"
#include "FaultInjector.h"
#include "ASTConsumer.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;


static llvm::cl::OptionCategory oCategory("Matcher Sample");
static llvm::cl::opt<bool> VerboseOption("verbose", llvm::cl::cat(oCategory));
static llvm::cl::opt<std::string> DirectoryOption("dir", llvm::cl::cat(oCategory));

std::unique_ptr<FrontendActionFactory> newSFIFrontendActionFactory(std::vector<FaultInjector*> injectors){
    class SFIFrontendActionFactory: public FrontendActionFactory{
        public:
            SFIFrontendActionFactory(std::vector<FaultInjector*> inj):FrontendActionFactory(),injectors(inj){}
            FrontendAction *create() override{
                return new SFIAction(injectors);
            }
        private: 
            std::vector<FaultInjector*> injectors;
    };
    return std::unique_ptr<FrontendActionFactory>(new SFIFrontendActionFactory(injectors));
};


int main(int argc, const char **argv){
    CommonOptionsParser op(argc, argv, oCategory);
    std::vector<std::string> srcs;
    srcs.push_back("src/test.cpp");
    //std::cout << op.getSourcePathList().size() << std::endl << op.getCompilations() << std::endl;
    //clang::tooling::CompilationDatabase comp = ;
    //clang::tooling::CompilationDatabase cd = clang::tooling::CompilationDatabase.autoDetectFromSource(srcs, "ERROR");

    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    /*if(op.getSourcePathList().size()!=1){
        std::cout<<"Please Only Select 1 File"<<std::endl;
        return 1;
    }*/
    
    //            cout<<"test"<<endl;
    //nun kann SFIAction via templating einen std::vector<FaultInjector> erhalten
    std::vector<FaultInjector *> injectors;
    //MIFSInjector inj;
    /*injectors.push_back(new MIFSInjector);
    injectors.push_back(new MIAInjector);
    injectors.push_back(new MIEBInjector);
    injectors.push_back(new MFCInjector);
    injectors.push_back(new MLACInjector);
    injectors.push_back(new MLOCInjector);
    injectors.push_back(new MFCInjector);
    injectors.push_back(new MVIVInjector);*/
    //injectors.push_back(new MVAVInjector);
    injectors.push_back(new MVAEInjector);

    bool verbose = VerboseOption.getValue();
    
    //cout << "start tool" << endl;
    //cout<<(VerboseOption.getValue()?1:0)<<endl;
    std::string dir=DirectoryOption.getValue();
    if(dir.compare("")!=0){
        cout<<"Changing destination directory to '"<<dir<<"'"<<endl;
        if(mkdir(dir.c_str(), ACCESSPERMS) && errno != EEXIST){
            cerr<<"-Failed"<<endl;
            return 1;
        }
    }
    for(FaultInjector *inj : injectors){
        inj->setVerbose(verbose);
        inj->setDirectory(dir);
    }
    //cout<<DirectoryOption.getValue()<<endl;
    //cout<<(DirectoryOption.getValue().compare("")?1:0)<<endl;
    Tool.run(newSFIFrontendActionFactory(injectors).get());
    //cout<<"end tool"<<endl;
    return 0;
};