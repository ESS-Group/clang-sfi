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




int execv_cpp(const std::string &path,
              const std::vector<std::string> &argv)
{
    /* Convert arguments to C-style and call execv. If it returns
     * (fails), clean up and pass return value to caller. */

    if (argv.size() == 0) {
        errno = EINVAL;
        return -1;
    }

    std::vector<char *> vec_cp;
    vec_cp.reserve(argv.size() + 1);
    for (auto s : argv)
        vec_cp.push_back(strdup(s.c_str()));
    vec_cp.push_back(NULL);

    int retval = execv(path.c_str(), vec_cp.data());

    int save_errno = errno;
    for (auto p : vec_cp)
        free(p);
    errno = save_errno;
    return retval;
}


int execv_cpp(const std::vector<std::string> &argv)
{
    /* Overloaded. Use first element as path for simpler call. */

    if (argv.size() == 0) {
        errno = EINVAL;
        return -1;
    }

    return execv_cpp(argv[0], argv);
}

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
        bool backedup = false;
        json executionSummary;
        if(j.find("compileCommand")!=j.end() && j.find("fileToExec")!=j.end()){

            //cout << "huhu"<<endl;
            int timeout = 0;
            //cout<<1<<endl;
            if(j.find("timeout")!=j.end())
                timeout = j["timeout"].get<int>();
            //cout<<1<<endl;
            std::string outputdir = ((dir.compare("")?dir+"/":"")+"output");
            if(mkdir(outputdir.c_str(), ACCESSPERMS) && errno != EEXIST){
                cerr<<"Error creating dir for output"<<endl;
                return 1;
            }

            //cout << "huhu"<<endl;
            std::vector<char *> compileArgs;
            std::string _cmd = j["compileCommand"].get<std::string>();
            char *__cmd = new char[_cmd.length()+1];
            strcpy(__cmd, _cmd.c_str());
            compileArgs.push_back(__cmd);
            
            if(j.find("compileCommandArgs")!=j.end()){
                //cout << "1"<<endl;
                for(json::iterator it=j.find("compileCommandArgs")->begin();it!=j.find("compileCommandArgs")->end();++it){
                    //cout << "1.1"<<endl;
                    std::string arg = it->get<std::string>();
                    char *_arg = new char[arg.length()+1];
                    strcpy(_arg, arg.c_str());
                    compileArgs.push_back(_arg);
                    //cout << "1.2"<<endl;
                }
                //cout << "2"<<endl;
            }
            compileArgs.push_back(NULL);

            //cout << "huhu"<<endl;
            std::vector<char *> args;

            _cmd = j["fileToExec"].get<std::string>();
            __cmd = new char[_cmd.length()+1];
            strcpy(__cmd, _cmd.c_str());
            //compileArgs.push_back(__cmd);

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

            //if(j.find("compileCommand")!=j.end())
            std::string compileCommand = j["compileCommand"].get<std::string>();;
            std::string fileToExec = j["fileToExec"].get<std::string>();;
            cout << "Found compileCommand in config ..."<<endl;
            cout << "Making backup for '"<< fileforInjection <<"'"<<endl;
            if(!backedup){
                backedup = true;
                replaceFileContent((dir.compare("")?dir+"/":"")+"backup.cpp", fileforInjection);
            }
            
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
                    std::stringstream ss,ss1;
                    ss<<fault<<"_"<<i<<".cpp";
                    std::string baseFilename;
                    ss>>baseFilename;
                    ss1<<(dir.compare("")?dir+"/":"")<<baseFilename;
                    std::string fileName;
                    ss1>>fileName;
                    replaceFileContent(fileforInjection,fileName);
                    /*cout << compileCommand.c_str() << " " << endl;
                    for(auto i : compileArgs){
                        if(i!=NULL)
                            cout << "Arg "<<i<<endl;
                        else
                            cout << "Arg "<<"NULL"<<endl;
                    }*/
                    //char* const argss[] = {"/usr/bin/clang++","src/test.cpp",NULL};
                        //cout << errno<<endl;
                    //compileCommands, compileArgs
                    //if(int exitCode = execl("/usr/bin/clang++","-v","src/test.cpp",NULL)){
                    //if(int exitCode = execv_cpp("/usr/bin/clang++",compileArgs)){
                    /*std::vector<char *> _args;
                    _args.reserve(compileArgs.size() + 1);
                    for (auto s : compileArgs)
                        _args.push_back(strdup(s.c_str()));
                    _args.push_back(NULL);*/
                    //cout << "huhu"<<endl;









                        //int exitCode = 0;

                        cout << "Compiling '"<< fileName << "'"<<endl;
                        bool success = false;
                        int pid = fork();
                        if(pid == 0){
                            
                            int fd1 = open((outputdir+"/"+baseFilename+".compile.stdout").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                            int fd2 = open((outputdir+"/"+baseFilename+".compile.stderr").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                            //cout<<(outputdir+"/"+baseFilename+".compile.stderr").c_str()<<fd1<<"|"<<fd2<<endl;
                            dup2(fd1,1);
                            dup2(fd2,2);

                            close(fd1);
                            close(fd2);
                            //dup2(fd2,3);
                            //timeout
                            int exitCode = 0;
                            /*cout<<"command: "<<compileCommand<<endl;
                            for(char* arg:compileArgs){
                                if(arg!=NULL)
                                    cout <<"arg "<<arg<<endl;
                                else
                                    cout << "arg NULL"<<endl;
                            }*/
                            exitCode = execv(compileCommand.c_str(), compileArgs.data());
                            int serrno = errno;
                            //cout<<errno<<endl;
                            //cout <<"hohoho"<<endl;
                            /*if(exitCode){
                                json err;
                                err["fault"] = fault;
                                err["index"] = i;
                                err["exitCode"] = exitCode;
                                executionSummary["failCompileRuns"].push_back(err);
                            }
                            */
                            
                            //cout <<exitCode<<endl;
                            _exit(errno);//exit child process
                        } else {
                            //cout << "Compilation running" << endl;
                            //wait(NULL);//waits for execution of all child processes (here everytime only 1)
                            int exitCode=0;
                            //cout <<exitCode<<endl;
                            pid_t exited_pid = waitpid(pid,&exitCode,0);
                            //cout <<exitCode<<endl;

                            //cout <<exited_pid<<endl;

                            int status = WEXITSTATUS(exitCode);
                            if(exitCode){
                                json err;
                                err["fault"] = fault;
                                err["index"] = i;
                                err["exitCode"] = exitCode;
                                err["status"] = status;
                                executionSummary["failCompileRuns"].push_back(err);
                                cout<<">failed (exitCode: "<<exitCode<<", status: "<<status<<")"<<endl;
                                //cout<<pid<<endl;
                            }else{
                                cout<<">done."<<endl;
                                success = true;
                            }
                            //cout << "Compilation done" << endl;
                        }





                    //int errors = 0;


                    /*if(exitCode){
                        cout << errno<<endl;
                        cout << "Compilation of '"<<fileName<<"' exited with "<< exitCode<<endl;
                    } else */
                    if(success){
                        //cout <<"Compiled."<<endl;
                        cout << "Executing '"<< fileName << "'"<<endl;
                        if(fork()==0){
                            
                            int fd1 = open((outputdir+"/"+baseFilename+".stdout").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                            int fd2 = open((outputdir+"/"+baseFilename+".stderr").c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);


                            dup2(fd1,1);
                            dup2(fd2,2);
                            //timeout

                            close(fd1);
                            close(fd2);
                            int exitCode = 0;
                            if(timeout!=0){
                                execTimeoutReturn _exitCode = exec_with_timeout(args.data(), timeout);
                                exitCode = _exitCode.exitCode;
                                /*if(exitCode.timeout){
                                    json err;
                                    err["fault"] = fault;
                                    err["index"] = i;
                                    err["timeout"] = true;
                                    err["exitCode"] = exitCode.exitCode;
                                    executionSummary["failRuns"].push_back(err);
                                    cout<<"deine Mudda1"<<endl;
                                } else if(exitCode.exitCode){
                                    json err;
                                    err["fault"] = fault;
                                    err["index"] = i;
                                    err["exitCode"] = exitCode.exitCode;
                                    executionSummary["failRuns"].push_back(err);
                                    cout<<"deine Mudda2"<<endl;
                                }*/
                            }else{
                                /*cout<<fileToExec.c_str()<<endl;
                                for(char* arg:args){
                                    cout<<"arg "<<(arg==NULL?"NULL":arg)<<endl;
                                }*/

                                /*
                                cout<<"command: "<<fileToExec<<endl;
                                for(char* arg:args){
                                    if(arg!=NULL)
                                        cout <<"arg "<<arg<<endl;
                                    else
                                        cout << "arg NULL"<<endl;
                                }*/


                                execv(fileToExec.c_str(), args.data());
                                
                                exitCode = errno;
                                //cout<<"huhu"<<exitCode;
                                /*if(exitCode){
                                    json err;
                                    err["fault"] = fault;
                                    err["index"] = i;
                                    err["exitCode"] = exitCode;
                                    executionSummary["failRuns"].push_back(err);
                                    cout<<executionSummary<<endl;
                                    cout<<"deine Mudda3|"<<exitCode<<"|"<<i<<"|"<<fault<<endl;
                                    errors++;
                                }*/
                            }

                            exit(exitCode);//exit child process
                        } else {
                            //cout << "running" << endl;
                            //wait(NULL);//waits for execution of all child processes (here everytime only 1)
                            //cout << ">done." << endl;

                            int exitCode=0;
                            pid_t exited_pid = waitpid(-1,&exitCode,0);
                            int status = WEXITSTATUS(exitCode);
                            if(exitCode){
                                json err;
                                err["fault"] = fault;
                                err["index"] = i;
                                err["exitCode"] = exitCode;
                                err["status"] = status;
                                executionSummary["failRuns"].push_back(err);
                                //cout<<executionSummary<<endl;
                                //cout<<"deine Mudda3|"<<exitCode<<"|"<<i<<"|"<<fault<<endl;
                                //errors++;
                            }
                            //cout<<"Exit code:"<<exitCode<<endl;
                            if(exitCode)
                                cout<<">failed (exitCode: "<<exitCode<<", status: "<<status<<")"<<endl;
                            else
                                cout<<">done."<<endl;
                        }
                    }
                    //cout<<"huhu"<<errors;
                }
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
        cout<<"Operation succeeded."<<endl;
        return 0;
    }
};