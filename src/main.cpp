#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
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

#include "StmtHandler.h"
#include "FaultInjector.h"
#include "ASTConsumer.h"

#include "libs/json.hpp"

using json = nlohmann::json;

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;


static llvm::cl::OptionCategory oCategory("Matcher Sample");
static llvm::cl::opt<bool> VerboseOption("verbose", llvm::cl::cat(oCategory));
static llvm::cl::opt<std::string> DirectoryOption("dir", llvm::cl::cat(oCategory));
static llvm::cl::opt<std::string> ConfigOption("config", llvm::cl::cat(oCategory));

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
    std::vector<FaultInjector *> available;
    std::vector<FaultInjector *> injectors;

    bool verbose = VerboseOption.getValue();
    


    //MIFSInjector inj;
    available.push_back(new MIFSInjector);
    available.push_back(new MIAInjector);
    available.push_back(new MIEBInjector);
    available.push_back(new MFCInjector);
    available.push_back(new MLACInjector);
    available.push_back(new MLOCInjector);
    available.push_back(new MFCInjector);
    available.push_back(new MVIVInjector);
    available.push_back(new MVAVInjector);
    available.push_back(new WVAVInjector);
    available.push_back(new MVAEInjector);
    available.push_back(new WAEPInjector);
    available.push_back(new WPFVInjector);
    available.push_back(new MLPAInjector);
    available.push_back(new MRSInjector);
    available.push_back(new MIESInjector);


    struct stat buf;

    
    std::string cfgFile= ConfigOption.getValue();

    std::string dir=DirectoryOption.getValue();
    if(dir.compare("")!=0){
        cout<<"Changing destination directory to '"<<dir<<"'"<<endl;
        if(mkdir(dir.c_str(), ACCESSPERMS) && errno != EEXIST){
            cerr<<"-Failed"<<endl;
            return 1;
        }
    }
    if(cfgFile.compare("")!=0){

    } else 
        cfgFile = "config.json";

    if(stat(cfgFile.c_str(), &buf) != -1){
        cout<<"Using config ("<<cfgFile<<"). Injecting:"<<endl;
        std::ifstream i("config.json");
        json j;
        i>>j;
        
        if(j.find("injectors")!=j.end()){
            for(json::iterator it=j.find("injectors")->begin();it!=j.find("injectors")->end();++it){
                for(FaultInjector * injector:available){
                    if(injector->toString().compare(*it)==0){
                        cout<<injector->toString()<<endl;
                        injectors.push_back(injector);
                        break;
                    }
                }
            }
        } else {
            for(FaultInjector * injector:available){
                injectors.push_back(injector);
            }
        }
    } else {
        cout<<"Config not found: default action - inject all errors"<<endl;

        for(FaultInjector * injector:available){
            injectors.push_back(injector);
        }
    }
    //cout << "start tool" << endl;
    //cout<<(VerboseOption.getValue()?1:0)<<endl;
    for(FaultInjector *inj : injectors){
        inj->setVerbose(verbose);
        inj->setDirectory(dir);
    }
    //cout<<DirectoryOption.getValue()<<endl;
    //cout<<(DirectoryOption.getValue().compare("")?1:0)<<endl;
    Tool.run(newSFIFrontendActionFactory(injectors).get());
    json summary;
    int injectioncount = 0;
    cout<<endl<<endl<<endl;
    cout<<">>>>> SUMMARY <<<<<"<<endl;
    for(FaultInjector * injector:injectors){
        int size = injector->locations.size();
        injectioncount += size;
    }
    for(FaultInjector * injector:injectors){
        json injection;
        int size = injector->locations.size();
        std::string type = injector->toString();
        //injectioncount += size;
        injection["type"] = type;
        injection["count"] = size;
        summary["types"].push_back(type);
        summary["injections"].push_back(injection);
        float part = ((float)size)/injectioncount*100.0;

        cout<<"Injected "<<size<<" "<<type<<" faults."<<endl<<"> "<<size<<"/"<<injectioncount<<" ("<< roundf(part*100)/100 <<"\%)"<<endl;
    }
    cout<<">>> Total Injected faults: "<<injectioncount<<endl;
    summary["injectionCount"] = injectioncount;

    std::ofstream o((dir.compare("")?dir+"/":"")+"summary.json");
    o <<summary;
    o.flush();
    o.close();
    cout << "saved summary at \""<<(dir.compare("")?dir+"/":"")+"summary.json"<<"\""<<endl;
    //cout<<"end tool"<<endl;
    return 0;
};