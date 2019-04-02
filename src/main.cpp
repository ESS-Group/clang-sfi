#include <fstream>
#include <stdio.h>
#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif

#include "llvm/Support/raw_ostream.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "clang-sfi"
using namespace llvm;

#include "SFIAction.h"

#include "libs/json.hpp"
using json = nlohmann::json;

using namespace clang;
using namespace clang::tooling;

// define commandline options for commonoptionparser
static llvm::cl::OptionCategory oCategory("clang-sfi");
static llvm::cl::opt<bool> VerboseOption("v", llvm::cl::cat(oCategory), llvm::cl::desc("verbose execution"));
static llvm::cl::opt<bool> VerboseVerboseOption("vv", llvm::cl::cat(oCategory), llvm::cl::desc("pass verbose to Clang"));
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
        bool runInvocation(std::shared_ptr<CompilerInvocation> Invocation,
                           FileManager *Files,
                           std::shared_ptr<PCHContainerOperations> PCHContainerOps,
                           DiagnosticConsumer *DiagConsumer) override {
            Invocation->getPreprocessorOpts().DetailedRecord = true;
            return FrontendActionFactory::runInvocation(Invocation, Files, PCHContainerOps, DiagConsumer);
        };

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

    // We do the same for debug-only flag.
    assert(opts.count("debug-only") == 1);
    opts["debug-only"]->setCategory(oCategory);
    // Additionally we preset the value with clang-sfi
    ::llvm::setCurrentDebugType("clang-sfi");
    #endif

    CommonOptionsParser op(argc, argv, oCategory);

    // LLVM_DEBUG(dbgs() << "Normal debug\n");
    // DEBUG_WITH_TYPE("clang2", dbgs() << "Extended debug\n");

    std::string fileforInjection = "";
    auto sourcePathList = op.getSourcePathList();
    ClangTool Tool(op.getCompilations(), sourcePathList);
    bool verbose = VerboseOption.getValue() || VerboseVerboseOption.getValue();
    if (VerboseVerboseOption.getValue()) {
        ArgumentsAdjuster ardj = getInsertArgumentAdjuster("-v");
        Tool.appendArgumentsAdjuster(ardj);
    }
    if (sourcePathList.size() != 1) {
        std::cout << "Please select exactly one main file" << std::endl;
        return 1;
    } else {
        fileforInjection = op.getSourcePathList()[0];
    }
    std::string mainFileName = sourcePathList.at(0);
    std::vector<FaultInjector *> available;
    std::vector<FaultInjector *> injectors;

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

    std::string cfgFileName = ConfigOption.getValue();
    std::string dir = DirectoryOption.getValue();
    if (cfgFileName.compare("") == 0) {
        cfgFileName = "config.json";
    }

    json cfgFile;
    if (fs::exists(cfgFileName)) {
        std::cout << "Using config (" << cfgFileName << ")." << std::endl;
        std::ifstream cfgFileInStream(cfgFileName.c_str());
        cfgFileInStream >> cfgFile;
        if (cfgFile.find("verbose") != cfgFile.end() && !verbose) {
            verbose = cfgFile["verbose"].get<bool>();
        }
        if (dir.compare("") == 0 && cfgFile.find("destDirectory") != cfgFile.end()) {
            dir = cfgFile["destDirectory"].get<std::string>();
        }
        if (cfgFile.find("injectors") != cfgFile.end()) {
            for (json::iterator it = cfgFile.find("injectors")->begin(); it != cfgFile.find("injectors")->end(); ++it) {
                for (FaultInjector *injector : available) {
                    if (injector->toString().compare(it->get<std::string>()) == 0) {
                        LLVM_DEBUG(dbgs() << "Adding " << injector->toString() << "\n");
                        injectors.push_back(injector);
                        break;
                    }
                }
            }
        } else {
            LLVM_DEBUG(dbgs() << "Adding all injectors\n");
            for (FaultInjector *injector : available) {
                injectors.push_back(injector);
            }
        }
    } else {
        // no config file => add all available injectors
        LLVM_DEBUG(dbgs() << "Adding all injectors\n");

        for (FaultInjector *injector : available) {
            injectors.push_back(injector);
        }
    }

    std::cout << "Injecting: ";
    for (FaultInjector *injector : injectors) {
        std::cout << (injector == injectors[0] ? "" : ", ") << injector->toString();
    }
    std::cout << std::endl;

    dir = fs::absolute(dir);
    std::cout << "Injection patches should be saved in directory '" << dir << "'" << std::endl;
    if (!fs::exists(dir)) {
        if (!fs::create_directories(dir)) {
            std::cerr << "Could not create directory " << dir << std::endl;
            return 1;
        }
    }
    dir += fs::path::preferred_separator;

    std::vector<std::string> filesToConsider;
    auto cwd = fs::current_path();
    std::string path = cwd.native() + fs::path::preferred_separator;
    size_t pos = mainFileName.find_last_of('/');

    std::string rootDir = RootDirectoryOption.getValue();
    if (rootDir.compare("") != 0) {
        if (rootDir.compare(".") == 0 || rootDir.compare("cwd") == 0) {
            rootDir = path;
        }
        if (verbose) {
            std::cout << "Sourcetree directory defined, files in this directory are also considered for matches."
                      << std::endl;
        }
        for (FaultInjector *injector : injectors) {
            injector->setRootDir(rootDir);
        }
    }
    if (cfgFile.find("consideredFilesList") != cfgFile.end()) {
        std::string fileList = cfgFile["consideredFilesList"];

        if (verbose) {
            std::cout << "Searching for files to consider based on \"" << path << "\"" << std::endl;
        }
        if (fs::exists(fileList)) {
            std::ifstream list(fileList);
            for (std::string file; std::getline(list, file);) {
                if (file.compare("") != 0 && file.compare("\n") != 0 && file.compare("\r\n") != 0 &&
                    file.compare("\r") != 0) {
                    if (fs::exists(path + file)) {
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
    if (cfgFile.find("consideredFiles") != cfgFile.end()) {
        for (json::iterator it = cfgFile.find("consideredFiles")->begin(); it != cfgFile.find("consideredFiles")->end(); ++it) {
            std::string file = it->get<std::string>();
            if (fs::exists(file)) {
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
        inj->setMatchMacro(true);
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
        if (cfgFile.find("multipleRuns") != cfgFile.end()) {
            summary["multipleRuns"] = cfgFile["multipleRuns"];
        }
        if (cfgFile.find("timeout") != cfgFile.end()) {
            summary["timeout"] = cfgFile["timeout"];
        }
        if (cfgFile.find("compileCommand") != cfgFile.end()) {
            summary["compileCommand"] = cfgFile["compileCommand"];
        }
        if (cfgFile.find("compileCommandArgs") != cfgFile.end()) {
            summary["compileCommandArgs"] = cfgFile["compileCommandArgs"];
        }
        if (cfgFile.find("fileToExec") != cfgFile.end()) {
            summary["fileToExec"] = cfgFile["fileToExec"];
        }
        if (cfgFile.find("fileToExecArgs") != cfgFile.end()) {
            summary["fileToExecArgs"] = cfgFile["fileToExecArgs"];
        }
        summary["directory"] = dir;
        summary["verbose"] = verbose;

        std::ofstream o(dir + "summary.json");
        if (!o.good()) {
            std::cerr << "File " << dir + "summary.json" << " could not be opened for write" << std::endl;
        } else {
            o << summary;
            o.flush();
            o.close();
            std::cout << "Saved summary at \"" << dir + "summary.json"
                    << "\"" << std::endl;
        }
    }

    std::cout << "Operation succeeded." << std::endl;
    return 0;
};
