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
#include <csignal>

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

std::string backupfile = "";
std::string backedupfile = "";


void replaceFileContent(std::string dest, std::string src){
            std::ifstream i(src.c_str());
            std::ofstream o(dest.c_str(), std::ofstream::trunc);
            o<<i.rdbuf();
            o.flush();
            o.close();
            i.close();
}


void signalHandler(int signum){
    if(signum == SIGSEGV){
        cout << endl << "Program ended with segmentation fault." << endl;
    } else {
        cout << endl << "Signal(" << signum << ") received. => Canceling Actions." << endl;
        if(backupfile.compare("") != 0 && backedupfile.compare("") != 0){
            cout << "Need to rollback backup." << endl;
            replaceFileContent(backedupfile, backupfile);
            cout << "Rolled back." << endl;
        }
    }
    exit(signum);
}

void compile(json j){
        std::string dir = j["directory"];
        bool verbose = j["verbose"];
        std::string fileforInjection = j["fileName"];
        bool backedup = false;
        json executionSummary;



        if(j.find("compileCommand")!=j.end() && j.find("fileToExec")!=j.end()){
            
            int timeout = 0;
            
            if(j.find("timeout")!=j.end())
                timeout = j["timeout"].get<int>();

            executionSummary["timeout"]=timeout;

            int multipleRuns = 1;
            
            if(j.find("multipleRuns")!=j.end())
                multipleRuns = j["multipleRuns"].get<int>();
            if(multipleRuns<1)
                multipleRuns = 1;
            executionSummary["runs"]=multipleRuns;
            
            std::string outputdir = ((dir.compare("")?dir+"/":"")+"output");
            if(mkdir(outputdir.c_str(), ACCESSPERMS) && errno != EEXIST){
                cerr<<"Error creating dir for output"<<endl;
                exit(1);
            }


            std::vector<char *> compileArgs;
            std::string _cmd = j["compileCommand"].get<std::string>();
            char *__cmd = new char[_cmd.length()+1];
            strcpy(__cmd, _cmd.c_str());
            compileArgs.push_back(__cmd);
            
            if(j.find("compileCommandArgs")!=j.end()){
                for(json::iterator it=j.find("compileCommandArgs")->begin();it!=j.find("compileCommandArgs")->end();++it){
                    std::string arg = it->get<std::string>();
                    char *_arg = new char[arg.length()+1];
                    strcpy(_arg, arg.c_str());
                    compileArgs.push_back(_arg);
                }
                
            }
            compileArgs.push_back(NULL);


            std::vector<char *> args;

            _cmd = j["fileToExec"].get<std::string>();
            __cmd = new char[_cmd.length()+1];
            strcpy(__cmd, _cmd.c_str());
            

            args.push_back(__cmd);


            if(j.find("fileToExecArgs")!=j.end()){
                for(json::iterator it=j.find("fileToExecArgs")->begin();it!=j.find("fileToExecArgs")->end();++it){

                    std::string arg = it->get<std::string>();
                    char *_arg = new char[arg.length()+1];
                    strcpy(_arg, arg.c_str());

                    args.push_back(_arg);
                }
            }
            args.push_back(NULL);


            std::string compileCommand = j["compileCommand"].get<std::string>();;
            std::string fileToExec = j["fileToExec"].get<std::string>();;
            cout << "Found compileCommand in config ..."<<endl;
            cout << "Making backup for '"<< fileforInjection <<"'"<<endl;
            if(!backedup){
                backedup = true;
                backupfile = (dir.compare("")?dir+"/":"")+"backup.cpp";
                backedupfile = std::string(fileforInjection.c_str());
                replaceFileContent((dir.compare("")?dir+"/":"")+"backup.cpp", fileforInjection);
            }
            
            


            cout<<"... done."<<endl;
            for(std::string type:j["types"]){

                int count = 0;
                for(json injection:j["injections"]){
                    if(injection["type"].get<std::string>().compare(type)==0)
                        count = injection["count"];
                }

                cout << "Compiling and executing injected fault ("<</*injector->toString()*/type<<")";
                if(verbose)cout<<" [0/"<<count<<"]"<<endl;
                else
                    cout <<endl;
                //int count = j["injections"][type]["count"];//injector->locations.size();
                std::string fault = type;//injector->toString();
                for(int i=0; i < count ; i++){
                    std::stringstream ss,ss1;
                    ss<<fault<<"_"<<i<<".cpp";
                    std::string baseFilename;
                    ss>>baseFilename;
                    ss1<<(dir.compare("")?dir+"/":"")<<baseFilename;
                    std::string fileName;
                    ss1>>fileName;
                    replaceFileContent(fileforInjection,fileName);

                        if(verbose)
                            cout << "Compiling '"<< fileName << "' ("<<fault<<" ["<<i+1<<"/"<<count<<"])"<<endl;
                        //else
                        //    cout << fault<<" ["<<i+1<<"/"<<count<<"]"<<endl;
                        bool success = false;
                        int pid = fork();
                        if(pid == 0){
                            
                            int fd1 = open((outputdir+"/"+baseFilename+".compile.stdout").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                            int fd2 = open((outputdir+"/"+baseFilename+".compile.stderr").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                            
                            dup2(fd1,1);
                            dup2(fd2,2);

                            close(fd1);
                            close(fd2);
                            
                            int exitCode = 0;
                            
                            exitCode = execv(compileCommand.c_str(), compileArgs.data());
                            int serrno = errno;

                            _exit(errno);//exit child process
                        } else {
                            int exitCode=0;
                            pid_t exited_pid = waitpid(pid,&exitCode,0);

                            int status = WEXITSTATUS(exitCode);
                            if(status){
                                json err;
                                err["fault"] = fault;
                                err["index"] = i;
                                err["exitCode"] = exitCode;
                                err["status"] = status;
                                executionSummary["failCompileRuns"].push_back(err);
                                if(verbose)cout<<">failed (exitCode: "<<exitCode<<", status: "<<status<<")"<<endl;
                            }else{
                                if(verbose)cout<<">done."<<endl;
                                success = true;
                            }
                        }
                        


                    if(success){
                        if(verbose)cout << "Executing '"<< fileName << "' ("<<fault<<" ["<<i+1<<"/"<<count<<"])"<<endl;

                        for(int k = 1 ; k<=multipleRuns ; k++){
                            if(multipleRuns != 1 && verbose)
                                cout <<"Run ["<<k<<"/"<<multipleRuns<<"]";

                            execTimeoutReturn *ret = (execTimeoutReturn *)mmap(NULL,sizeof(execTimeoutReturn), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
                            int *tErrno = (int *)mmap(NULL,sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
                            ret->timeout = false;
                            ret->exitCode = 0;
                            msync(ret,sizeof(execTimeoutReturn), MS_SYNC|MS_INVALIDATE);
                            int exitCode=0;
                            int child_pid = fork();
                            if(child_pid==0){
                                
                                int fd1 = open((outputdir+"/"+baseFilename+".stdout").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                                int fd2 = open((outputdir+"/"+baseFilename+".stderr").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);


                                dup2(fd1,1);
                                dup2(fd2,2);
                                //timeout

                                close(fd1);
                                close(fd2);
                                //cout<<"test"<<endl;
                                int exitCode = 0;
                                execTimeoutReturn _exitCode;
                                if(timeout!=0){
                                    _exitCode = exec_with_timeout(args.data(), timeout);
                                    ret->exitCode = _exitCode.exitCode;
                                    ret->timeout = _exitCode.timeout;
                                    msync(ret,sizeof(execTimeoutReturn), MS_SYNC|MS_INVALIDATE);
                                }else{
                                    ret->exitCode = exitCode;
                                    ret->timeout = false;
                                    msync(ret,sizeof(execTimeoutReturn), MS_SYNC|MS_INVALIDATE);
                                    execv(fileToExec.c_str(), args.data());
                                    exitCode = errno;
                                    cerr<<"Exited with errno: "<<exitCode<<endl;
                                    *tErrno=errno;
                                    msync(tErrno,sizeof(int), MS_SYNC|MS_INVALIDATE);
                                }
                                msync(ret,sizeof(execTimeoutReturn), MS_SYNC|MS_INVALIDATE);
                                exit(exitCode);//exit child process
                            } else {
                                pid_t exited_pid = waitpid(child_pid,&exitCode,0);
                                bool timedout = false;
                                if(timeout){
                                    exitCode = ret->exitCode;
                                    timedout = ret->timeout;
                                }

                                int errornum = *tErrno;
                                if(errornum == 0 && ret->errnumber)
                                    errornum = ret->errnumber;
                                munmap(ret, sizeof(execTimeoutReturn));
                                munmap(tErrno, sizeof(tErrno));

                                int status = WEXITSTATUS(exitCode);
                                if(status || errornum){
                                    json err;
                                    err["fault"] = fault;
                                    err["index"] = i;
                                    if(errornum){
                                        err["errno"] = errornum;
                                    } else {
                                        err["exitCode"] = exitCode;
                                        err["status"] = status;
                                        err["timeout"] = timedout;
                                    }
                                    
                                    err["run"] = k;
                                    executionSummary["failRuns"].push_back(err);

                                    if(verbose)cout<<">failed (exitCode: "<<exitCode<<", status: "<<status<<")"<<endl;
                                } else {


                                    json succ;
                                    succ["fault"] = fault;
                                    succ["index"] = i;
                                    succ["exitCode"] = exitCode;
                                    succ["status"] = status;
                                    executionSummary["succRuns"].push_back(succ);
                                    if(verbose)cout<<">done."<<endl;

                                }
                            }
                        }
                    }
                }
            }
            cout<<endl<<endl;
            cout<<"Summary:"<<endl;
            if(executionSummary.find("failCompileRuns")!=executionSummary.end()){
                cout << "- "<<std::distance(executionSummary.find("failCompileRuns")->begin(), executionSummary.find("failCompileRuns")->end()) << " failed compile runs."<<endl;
            } else {
                cout << "- No failed compile runs."<<endl;
            }
            if(executionSummary.find("failRuns")!=executionSummary.end()){
                cout << "- "<<std::distance(executionSummary.find("failRuns")->begin(), executionSummary.find("failRuns")->end()) << " failed runs."<<endl;
            } else {
                cout << "- No failed runs."<<endl;
            }
            std::ofstream o((dir.compare("")?dir+"/":"")+"executionsummary.json");
            //cout << summary;
            o <<executionSummary;
            o.flush();
            o.close();
            for(char* arg:compileArgs)
                delete [] arg;
            for(char* arg:args)
                delete [] arg;
            //delete [] __cmd;
        }
        if(backedup){
            replaceFileContent(fileforInjection, (dir.compare("")?dir+"/":"")+"backup.cpp");
            cout << "Resetting '"<<fileforInjection<<"' to backed up version"<<endl;
        }
}


static llvm::cl::OptionCategory oCategory("clang-sfi");
static llvm::cl::opt<bool> VerboseOption("verbose", llvm::cl::cat(oCategory));
static llvm::cl::opt<std::string> DirectoryOption("dir", llvm::cl::cat(oCategory));
static llvm::cl::opt<std::string> ConfigOption("config", llvm::cl::cat(oCategory));
static llvm::cl::opt<bool> NoInjectOption("no-inject", llvm::cl::cat(oCategory));
static llvm::cl::opt<bool> CompileOption("compile", llvm::cl::cat(oCategory));
//static llvm::cl::opt<bool> ExecuteOption("execute", llvm::cl::cat(oCategory));

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
    signal(SIGINT, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
    CommonOptionsParser op(argc, argv, oCategory);
    
    std::string fileforInjection = "";
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    if(op.getSourcePathList().size()!=1){
        std::cout<<"Please Only Select 1 File"<<std::endl;
        return 1;
    } else
        fileforInjection = op.getSourcePathList()[0];
        


    std::vector<FaultInjector *> available;
    std::vector<FaultInjector *> injectors;

    bool verbose = VerboseOption.getValue();
    

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
    if(!NoInjectOption.getValue()){
        if(cfgFile.compare("")==0) 
            cfgFile = "config.json";

        json j;
        if(stat(cfgFile.c_str(), &buf) != -1){
            cout<<"Using config ("<<cfgFile<<")."<<endl;
            std::ifstream i(cfgFile.c_str());
            i>>j;
            if(j.find("verbose")!=j.end() && !verbose)
                    verbose = j["verbose"].get<bool>();
            if(dir.compare("")==0 && j.find("destDirectory")!=j.end()){
                dir = j["destDirectory"].get<std::string>();
            }
            if(j.find("injectors")!=j.end()){
                for(json::iterator it=j.find("injectors")->begin();it!=j.find("injectors")->end();++it){
                    for(FaultInjector * injector:available){
                        if(injector->toString().compare(it->get<std::string>())==0){
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
        

        for(FaultInjector *inj : injectors){
            inj->setVerbose(verbose);
            inj->setDirectory(dir);
        }
    
    
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
                
                injection["type"] = type;
                injection["count"] = size;
                
                int i = 0;
                for(FaultInjector::StmtBinding& binding:injector->locations){
                    json loc;
                    loc["begin"] = binding.location.begin.toString();
                    loc["end"] = binding.location.end.toString();
                    loc["index"] = i;
                    summary[type].push_back(loc);
                    i++;
                }
                summary["types"].push_back(type);
                summary["injections"].push_back(injection);
                float part = ((float)size)/injectioncount*100.0;

                cout<<"Injected "<<size<<" "<<type<<" faults."<<endl<<"> "<<size<<"/"<<injectioncount<<" ("<< roundf(part*100)/100 <<"\%)"<<endl;
                
            }
            cout<<">>> Total Injected faults: "<<injectioncount<<endl;
            summary["injectionCount"] = injectioncount;
            summary["fileName"] = fileforInjection;
            if(j.find("multipleRuns")!=j.end())
                summary["multipleRuns"] = j["multipleRuns"];
            if(j.find("timeout")!=j.end())
                summary["timeout"] = j["timeout"];
            if(j.find("compileCommand")!=j.end())
                summary["compileCommand"] = j["compileCommand"];
            if(j.find("compileCommandArgs")!=j.end())
                summary["compileCommandArgs"] = j["compileCommandArgs"];
            if(j.find("fileToExec")!=j.end())
                summary["fileToExec"] = j["fileToExec"];
            if(j.find("fileToExecArgs")!=j.end())
                summary["fileToExecArgs"] = j["fileToExecArgs"];
            summary["directory"] = dir;
            summary["verbose"] = verbose;
            std::ofstream o((dir.compare("")?dir+"/":"")+"summary.json");
            o <<summary;
            o.flush();
            o.close();
            cout << "saved summary at \""<<(dir.compare("")?dir+"/":"")+"summary.json"<<"\""<<endl;


            if(verbose)
                summary["verbose"] = true;
            if(CompileOption.getValue())
                compile(summary);
            } 
        }else if(CompileOption.getValue()){
            if(cfgFile.compare("")!=0)
                cfgFile = (dir.compare("")?dir+"/":"")+"summary.json";
            if(stat(cfgFile.c_str(), &buf) != -1){
                cout<<"Using config ("<<cfgFile<<")."<<endl;
                std::ifstream i(cfgFile.c_str());
                json j;
                i>>j;
                if(verbose)
                    j["verbose"]=true;
                compile(j);
            } else {
                cout << "Config not found!!";
            }
        } else {
            cout << "Nothing to do!!";
        }

        cout<<"Operation succeeded."<<endl;
        return 0;
};