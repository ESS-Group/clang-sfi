#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

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
#include "libs/execWithTimeout.cpp"
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

void replaceFileContent(std::string dest, std::string src){
            std::ifstream i(src.c_str());
            std::ofstream o(dest.c_str(), std::ofstream::trunc);
            o<<i.rdbuf();
            o.flush();
            o.close();
            i.close();
}

int main(int argc, const char **argv){
    CommonOptionsParser op(argc, argv, oCategory);
    //std::vector<std::string> srcs;
    //srcs.push_back("src/test.cpp");
    //std::cout << op.getSourcePathList().size() << std::endl << op.getCompilations() << std::endl;
    //clang::tooling::CompilationDatabase comp = ;
    //clang::tooling::CompilationDatabase cd = clang::tooling::CompilationDatabase.autoDetectFromSource(srcs, "ERROR");
    std::string fileforInjection = "";
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    if(op.getSourcePathList().size()!=1){
        std::cout<<"Please Only Select 1 File"<<std::endl;
        return 1;
    } else
        fileforInjection = op.getSourcePathList()[0];
    
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
    if(cfgFile.compare("")==0) 
        cfgFile = "config.json";

    json j;
    if(stat(cfgFile.c_str(), &buf) != -1){
        cout<<"Using config ("<<cfgFile<<")."<<endl;
        std::ifstream i("config.json");
        i>>j;
        
        if(dir.compare("")==0 && j.find("destDirectory")!=j.end()){
            dir = j["destDirectory"].get<std::string>();
        }
        if(j.find("injectors")!=j.end()){
            for(json::iterator it=j.find("injectors")->begin();it!=j.find("injectors")->end();++it){
                for(FaultInjector * injector:available){
                    if(injector->toString().compare(*it)==0){
                        //cout<<injector->toString()<<endl;
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
        //cout<<"Config not find config: default action - inject all errors"<<endl;

        for(FaultInjector * injector:available){
            injectors.push_back(injector);
        }
    }
    cout << "Injecting: ";
    for(FaultInjector * injector:injectors){
        cout<<(injector==injectors[0]?"":", ")<<injector->toString();
    }
    cout << endl;
    if(dir.compare("")!=0){
        cout<<"Changing destination directory to '"<<dir<<"'"<<endl;
        if(mkdir(dir.c_str(), ACCESSPERMS) && errno != EEXIST){
            cerr<<"-Failed"<<endl;
            return 1;
        }
    }
    //cout << "start tool" << endl;
    //cout<<(VerboseOption.getValue()?1:0)<<endl;
    for(FaultInjector *inj : injectors){
        inj->setVerbose(verbose);
        inj->setDirectory(dir);
    }
    //return 0;
    //cout<<DirectoryOption.getValue()<<endl;
    //cout<<(DirectoryOption.getValue().compare("")?1:0)<<endl;
    int ret = Tool.run(newSFIFrontendActionFactory(injectors).get());
    if(ret==2){
        cout << "Some Files were skipped, because there was no compileCommand for them in compile_commands.json!!!"<<endl;
    }
    if(ret == 1){
        cout << "An Error occured while running this Tool..."<<endl;
        return 1;
    } else {
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
        summary["fileName"] = fileforInjection;
        std::ofstream o((dir.compare("")?dir+"/":"")+"summary.json");
        o <<summary;
        o.flush();
        o.close();
        cout << "saved summary at \""<<(dir.compare("")?dir+"/":"")+"summary.json"<<"\""<<endl;

        if(j.find("compileCommand")!=j.end() && j.find("fileToExec")!=j.end()){
            int timeout = 0;
            if(j.find("compileCommand")!=j.end())
                timeout = j["timeout"].get<int>();
            std::string outputdir = ((dir.compare("")?dir+"/":"")+"output");
            if(mkdir(outputdir.c_str(), ACCESSPERMS) && errno != EEXIST){
                cerr<<"Error creating dir for output"<<endl;
                return 1;
            }
            std::vector<const char *> compileArgs;
            compileArgs.push_back(j["compileCommand"].get<std::string>().c_str());
            if(j.find("compileCommandArgs")!=j.end()){
                for(json::iterator it=j.find("compileCommandArgs")->begin();it!=j.find("compileCommandArgs")->end();++it){
                    compileArgs.push_back(it->get<std::string>().c_str());
                }
            }
            compileArgs.push_back(NULL);


            std::vector<const char *> args;
            args.push_back(j["fileToExec"].get<std::string>().c_str());
            if(j.find("fileToExecArgs")!=j.end()){
                for(json::iterator it=j.find("fileToExecArgs")->begin();it!=j.find("fileToExecArgs")->end();++it){
                    args.push_back(it->get<std::string>().c_str());
                }
            }
            args.push_back(NULL);

            //if(j.find("compileCommand")!=j.end())
            std::string compileCommand = j["compileCommand"].get<std::string>();;
            std::string fileToExec = j["fileToExec"].get<std::string>();;
            cout << "Found compileCommand in config ..."<<endl;
            cout << "Making backup for '"<< fileforInjection <<"'"<<endl;
            replaceFileContent((dir.compare("")?dir+"/":"")+"backup.cpp", fileforInjection);
            /*
            std::ifstream i(fileforInjection);
            std::ofstream o((dir.compare("")?dir+"/":"")+"backup.cpp", std::ofstream::trunc);
            o<<i;
            o.flush();
            o.close();
            i.close();
            */
            cout<<"... done."<<endl;
            for(FaultInjector * injector:injectors){
                cout << "Injecting Fault ("<<injector->toString()<<")"<<endl;
                int count = injector->locations.size();
                std::string fault = injector->toString();
                for(int i=0; i < count ; i++){
                    std::stringstream ss;
                    ss<<(dir.compare("")?dir+"/":"")<<fault<<i;
                    std::string fileName;
                    ss>>fileName;
                    replaceFileContent(fileforInjection,fileName);
                    if(int exitCode = execv(compileCommand.c_str(), (char * const *)&compileArgs)){
                        cout << "Compiling of '"<<fileName<<"' exited with "<< exitCode<<endl;
                    } else {
                        cout << "Injecting '"<< fileName << "'"<<endl;
                        if(fork()==0){
                            
                            int fd1 = open((outputdir+"/"+fileName+".stdout").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                            int fd2 = open((outputdir+"/"+fileName+".stderr").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                            dup2(fd1,1);
                            dup2(fd2,2);
                            close(fd1);
                            close(fd2);
                            //timeout
                            int exitCode = 0;
                            if(timeout!=0)
                                exec_with_timeout((char * const*)&args, timeout);
                            else
                                exitCode = execv(fileToExec.c_str(), (char * const*)&args);
                            if(exitCode){
                                json err;
                                err["fault"] = fault;
                                err["index"] = i;
                                err["exitCode"] = exitCode;
                                summary["failRuns"].push_back(err);
                            }
                            exit(0);//exit child process
                        } else {
                            cout << "running" << endl;
                            wait(NULL);//waits for execution of all child processes (here everytime only 1)
                            cout << "done" << endl;
                        }
                    }
                }
            }
            std::ofstream o((dir.compare("")?dir+"/":"")+"summary.json");
            o <<summary;
            o.flush();
            o.close();
        }
        //cout<<"end tool"<<endl;
        return 0;
    }
};