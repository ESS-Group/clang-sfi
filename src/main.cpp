#include <fstream>
#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#define getCWD _getcwd
#else
#include <sys/stat.h>
#include <unistd.h>
#define _MAX_DIR PATH_MAX
#define getCWD getcwd
#endif

#include "llvm/Support/raw_ostream.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "clang-sfi"
using namespace llvm;

#include "SFIASTConsumer.h"

#include "libs/json.hpp"
using json = nlohmann::json;

using namespace clang;
using namespace clang::tooling;

std::string backupfile = "";
std::string backedupfile = "";

void replaceFileContent(std::string dest, std::string src) { // helper function to write to file
                                                             // and replace it given the path of a
                                                             // destination and a source file
    std::ifstream i(src.c_str());
    std::ofstream o(dest.c_str(), std::ofstream::trunc);
    o << i.rdbuf();
    o.flush();
    o.close();
    i.close();
}

// define commandline options for commonoptionparser
static llvm::cl::OptionCategory oCategory("clang-sfi");
static llvm::cl::opt<bool> VerboseOption("verbose", llvm::cl::cat(oCategory), llvm::cl::desc("verbose execution"));
static llvm::cl::opt<std::string> DirectoryOption("dir", llvm::cl::cat(oCategory), llvm::cl::init("injections"),
                                                  llvm::cl::desc("Directory where to store the patch files"));
static llvm::cl::opt<std::string> RootDirectoryOption(
    "sourcetree", llvm::cl::cat(oCategory), llvm::cl::init("injections"),
    llvm::cl::desc("Absolute path to source directory or '.' for current working directory. If defined, matches in all "
                   "files in the directory that are referenced from main source file."));
static llvm::cl::opt<std::string> ConfigOption("config", llvm::cl::cat(oCategory),
                                               llvm::cl::desc("Specify an optional configuration file"));

std::unique_ptr<FrontendActionFactory>
newSFIFrontendActionFactory(std::vector<FaultInjector *> injectors) { // factory for creating
                                                                      // SFIFrontendAction, needed to
                                                                      // submit injectors to
                                                                      // FrontendActions constructor
    class SFIFrontendActionFactory : public FrontendActionFactory {
      public:
        SFIFrontendActionFactory(std::vector<FaultInjector *> inj) : FrontendActionFactory(), injectors(inj) {
        }
        FrontendAction *create() override {
            return new SFIAction(injectors);
        }

      private:
        std::vector<FaultInjector *> injectors;
    };
    return std::unique_ptr<FrontendActionFactory>(new SFIFrontendActionFactory(injectors));
};

int main(int argc, const char **argv) {
    #ifndef NDEBUG
    // Modify debug flag to show up for our tool.
    StringMap<llvm::cl::Option*> &opts = llvm::cl::getRegisteredOptions();
    assert(opts.count("debug") == 1);
    opts["debug"]->setCategory(oCategory);

    // We could do the same for debug-only flag, but since this is of no use at the moment, we directly set the debug type.
    // assert(opts.count("debug-only") == 1);
    // opts["debug-only"]->setCategory(oCategory);
    ::llvm::setCurrentDebugType("clang-sfi");
    #endif

    CommonOptionsParser op(argc, argv, oCategory);

    std::string fileforInjection = "";
    auto sourcePathList = op.getSourcePathList();
    ClangTool Tool(op.getCompilations(), sourcePathList);
    if (sourcePathList.size() != 1) {
        std::cout << "Please select exactly one main file" << std::endl;
        return 1;
    } else {
        fileforInjection = op.getSourcePathList()[0];
    }
    std::string mainFileName = sourcePathList.at(0);
    std::vector<FaultInjector *> available;
    std::vector<FaultInjector *> injectors;

    bool verbose = VerboseOption.getValue();

    // Add an instance of every available FaultInjector to list.
    // Only these instances can be added by the config.
    available.push_back(new SMIFSInjector);
    available.push_back(new SMIAInjector);
    available.push_back(new SMIEBInjector);
    available.push_back(new SMLPAInjector);

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
    available.push_back(new OMVAEInjector);
    available.push_back(new OMVAVInjector);
    available.push_back(new OWVAVInjector);
    available.push_back(new WAEPInjector);
    available.push_back(new WPFVInjector);
    available.push_back(new MLPAInjector);
    available.push_back(new MRSInjector);
    available.push_back(new MIESInjector);

    struct stat buf;

    std::string cfgFile = ConfigOption.getValue();
    std::string dir = DirectoryOption.getValue();
    if (cfgFile.compare("") == 0) {
        cfgFile = "config.json";
    }

    json j;
    if (stat(cfgFile.c_str(), &buf) != -1) { // config file exists
        std::cout << "Using config (" << cfgFile << ")." << std::endl;
        std::ifstream i(cfgFile.c_str());
        i >> j;
        if (j.find("verbose") != j.end() && !verbose) {
            verbose = j["verbose"].get<bool>();
        }
        if (dir.compare("") == 0 && j.find("destDirectory") != j.end()) {
            dir = j["destDirectory"].get<std::string>();
        }
        if (j.find("injectors") != j.end()) {
            for (json::iterator it = j.find("injectors")->begin(); it != j.find("injectors")->end(); ++it) {
                for (FaultInjector *injector : available) {
                    if (injector->toString().compare(it->get<std::string>()) == 0) {
                        // std::cout<<injector->toString()<<endl;
                        injectors.push_back(injector);
                        break;
                    }
                }
            }
        } else {
            for (FaultInjector *injector : available) {
                injectors.push_back(injector);
            }
        }
    } else {
        // no config file => add all available injectors
        // std::cout<<"Config not find config: default action - inject all
        // errors"<<endl;

        for (FaultInjector *injector : available) {
            injectors.push_back(injector);
        }
    }
    std::cout << "Injecting: ";
    for (FaultInjector *injector : injectors) {
        std::cout << (injector == injectors[0] ? "" : ", ") << injector->toString();
    }
    std::cout << std::endl;
    if (dir.compare("") != 0) {
        std::cout << "Changing destination directory to '" << dir << "'" << std::endl;
#ifdef _WIN32
        int mkdirSuccess = _mkdir(dir.c_str());
#else
        int mkdirSuccess = mkdir(dir.c_str(), ACCESSPERMS);
#endif

        if (mkdirSuccess != 0 && errno != EEXIST) {
            std::cerr << "-Failed" << std::endl;
            return 1;
        }
    }
    std::vector<std::string> filesToConsider;
    char cwd[_MAX_DIR];
    getCWD(cwd, _MAX_DIR);
    std::string path(cwd);
#ifdef _WIN32
    char pathSep = '\\';
#else
    char pathSep = '/';
#endif
    path = path + pathSep;
    size_t pos = mainFileName.find_last_of('/');

    std::string rootDir = RootDirectoryOption.getValue();
    if (rootDir.compare("") != 0) {
        if (rootDir.compare(".") == 0 || rootDir.compare("cwd") == 0)
            rootDir = path;
        if (verbose)
            std::cout << "Sourcetree directory defined, files in this directory are also considered for matches."
                      << std::endl;
        for (FaultInjector *injector : injectors) {
            injector->setRootDir(rootDir);
        }
    }
    if (j.find("consideredFilesList") != j.end()) {
        std::string fileList = j["consideredFilesList"];

        if (verbose) {
            std::cout << "Searching for files to consider based on \"" << path << "\"" << std::endl;
        }
        if (stat(fileList.c_str(), &buf) != -1) {
            std::ifstream list(fileList);
            for (std::string file; std::getline(list, file);) {
                if (file.compare("") != 0 && file.compare("\n") != 0 && file.compare("\r\n") != 0 &&
                    file.compare("\r") != 0) {
                    if (stat((path + file).c_str(), &buf) != -1) {
                        filesToConsider.push_back(file);
                        if (verbose) {
                            std::cout << "Considering File: " << file << std::endl;
                        }
                    } else {
                        if (verbose) {
                            std::cerr << "File not found: " << file << std::endl;
                        }
                    }
                }
            }
        } else {
            std::cerr << "FileList defined, but File (" << fileList << ") not found." << std::endl
                      << "Exiting..." << std::endl;
        }
    }
    if (j.find("consideredFiles") != j.end()) {
        for (json::iterator it = j.find("consideredFiles")->begin(); it != j.find("consideredFiles")->end(); ++it) {
            std::string file = it->get<std::string>();
            if (stat(file.c_str(), &buf) != -1) {
                filesToConsider.push_back(file);
            }
        }
    }

    for (FaultInjector *inj : injectors) {
        // Set verbose and directory options for injectors.
        inj->setVerbose(verbose);
        inj->setDirectory(dir);
        inj->setRootDir(path);
        inj->setFileList(filesToConsider);
    }

    int ret = Tool.run(newSFIFrontendActionFactory(injectors).get());
    if (ret == 2) {
        std::cout << "Some files were skipped, because there was no "
                     "compileCommand "
                     "for them in compile_commands.json!"
                  << std::endl;
    }
    if (ret == 1) {
        std::cout << "An error occurred while running the tool..." << std::endl;
        return 1;
    } else {
        // Create overview in summary.json.
        json summary;
        int injectioncount = 0;
        std::cout << std::endl << std::endl << std::endl;
        std::cout << ">>>>> SUMMARY <<<<<" << std::endl;
        for (FaultInjector *injector : injectors) {
            int size = injector->locations.size();
            injectioncount += size;
        }
        for (FaultInjector *injector : injectors) {
            json injection;
            int size = injector->locations.size();
            std::string type = injector->toString();

            injection["type"] = type;
            injection["count"] = size;

            int i = 0;
            for (FaultInjector::StmtBinding &binding : injector->locations) {
                json loc;
                loc["begin"] = binding.location.begin.toString();
                loc["end"] = binding.location.end.toString();
                loc["index"] = i;
                summary[type].push_back(loc);
                i++;
            }
            summary["types"].push_back(type);
            summary["injections"].push_back(injection);
            float part = 0;
            if (injectioncount > 0) {
                part = ((float)size) / injectioncount * 100.0;
            }

            std::cout << "Injected " << size << " " << type << " faults." << std::endl
                      << "> " << size << "/" << injectioncount << " (" << roundf(part * 100) / 100 << "\%)"
                      << std::endl;
        }
        std::cout << ">>> Total injected faults: " << injectioncount << std::endl;
        summary["injectionCount"] = injectioncount;
        summary["fileName"] = fileforInjection;
        if (j.find("multipleRuns") != j.end()) {
            summary["multipleRuns"] = j["multipleRuns"];
        }
        if (j.find("timeout") != j.end()) {
            summary["timeout"] = j["timeout"];
        }
        if (j.find("compileCommand") != j.end()) {
            summary["compileCommand"] = j["compileCommand"];
        }
        if (j.find("compileCommandArgs") != j.end()) {
            summary["compileCommandArgs"] = j["compileCommandArgs"];
        }
        if (j.find("fileToExec") != j.end()) {
            summary["fileToExec"] = j["fileToExec"];
        }
        if (j.find("fileToExecArgs") != j.end()) {
            summary["fileToExecArgs"] = j["fileToExecArgs"];
        }
        summary["directory"] = dir;
        summary["verbose"] = verbose;

        std::ofstream o((dir.compare("") ? dir + "/" : "") + "summary.json");
        o << summary;
        o.flush();
        o.close();
        std::cout << "Saved summary at \"" << (dir.compare("") ? dir + "/" : "") + "summary.json"
                  << "\"" << std::endl;
    }

    std::cout << "Operation succeeded." << std::endl;
    return 0;
};
