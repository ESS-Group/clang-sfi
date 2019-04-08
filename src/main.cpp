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
#include "FaultInjector.h"

#include "libs/json.hpp"
using json = nlohmann::json;

using namespace clang;
using namespace clang::tooling;

// define commandline options for commonoptionparser
static llvm::cl::OptionCategory oCategory("clang-sfi");
static llvm::cl::opt<bool> VerboseOption("v", llvm::cl::cat(oCategory), llvm::cl::desc("verbose execution"));
static llvm::cl::opt<bool> VerboseVerboseOption("vv", llvm::cl::cat(oCategory), llvm::cl::desc("pass verbose to Clang"));
static llvm::cl::opt<std::string> PatchDirectoryOption("patchdir", llvm::cl::cat(oCategory), llvm::cl::init("injections"),
                                                  llvm::cl::desc("Directory where to store the patch files"));
static llvm::cl::opt<std::string> RootDirectoryOption(
    "sourcetree", llvm::cl::cat(oCategory), llvm::cl::init("."),
    llvm::cl::desc("Absolute path to source directory or '.' for current working directory. If defined, matches in all "
                   "files in the directory that are referenced from main source file."));
static llvm::cl::opt<std::string> ConfigOption("config", llvm::cl::cat(oCategory),
                                               llvm::cl::desc("Specify an optional configuration file"));

/// Factory to create SFIFrontendAction needed to submit parameters to FrontendActions constructor.
std::unique_ptr<FrontendActionFactory>
newSFIFrontendActionFactory(std::vector<std::string> injectors, FaultInjectorOptions &fiOpt) {
    class SFIFrontendActionFactory : public FrontendActionFactory {
      public:
        SFIFrontendActionFactory(std::vector<std::string> inj, FaultInjectorOptions &pfiOpt)
            : FrontendActionFactory(), injectors(inj), fiOpt(pfiOpt) {
        }
        FrontendAction *create() override {
            return new SFIAction(injectors, fiOpt);
        }
        bool runInvocation(std::shared_ptr<CompilerInvocation> Invocation,
                           FileManager *Files,
                           std::shared_ptr<PCHContainerOperations> PCHContainerOps,
                           DiagnosticConsumer *DiagConsumer) override {
            Invocation->getPreprocessorOpts().DetailedRecord = true;
            return FrontendActionFactory::runInvocation(Invocation, Files, PCHContainerOps, DiagConsumer);
        };

      private:
        std::vector<std::string> injectors;
        FaultInjectorOptions &fiOpt;
    };
    return std::unique_ptr<FrontendActionFactory>(new SFIFrontendActionFactory(injectors, fiOpt));
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

    auto sourcePathList = op.getSourcePathList();
    ClangTool Tool(op.getCompilations(), sourcePathList);
    bool verbose = VerboseOption.getValue() || VerboseVerboseOption.getValue();
    if (VerboseVerboseOption.getValue()) {
        ArgumentsAdjuster ardj = getInsertArgumentAdjuster("-v");
        Tool.appendArgumentsAdjuster(ardj);
    }
    std::vector<std::string> available;
    std::vector<std::string> injectors;

    // Add an instance of every available FaultInjector to list.
    // Only these instances can be added by the config.
    available.push_back("SMIFS");
    available.push_back("SMIA");
    available.push_back("SMIEB");
    available.push_back("SMLPA");

    available.push_back("MIFS");
    available.push_back("MIA");
    available.push_back("MIEB");
    available.push_back("MFC");
    available.push_back("MLAC");
    available.push_back("MLOC");
    available.push_back("MFC");
    available.push_back("MVIV");
    available.push_back("MVAV");
    available.push_back("WVAV");
    available.push_back("MVAE");
    available.push_back("OMVAE");
    available.push_back("OMVAV");
    available.push_back("OWVAV");
    available.push_back("WAEP");
    available.push_back("WPFV");
    available.push_back("MLPA");
    available.push_back("MRS");
    available.push_back("MIES");

    std::string cfgFileName = ConfigOption.getValue();
    std::string patchDir = PatchDirectoryOption.getValue();
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
        if (patchDir.compare("") == 0 && cfgFile.find("destDirectory") != cfgFile.end()) {
            patchDir = cfgFile["destDirectory"].get<std::string>();
        }
        if (cfgFile.find("injectors") != cfgFile.end()) {
            for (json::iterator it = cfgFile.find("injectors")->begin(); it != cfgFile.find("injectors")->end(); ++it) {
                for (auto injector : available) {
                    if (injector.compare(it->get<std::string>()) == 0) {
                        LLVM_DEBUG(dbgs() << "Adding " << injector << "\n");
                        injectors.push_back(injector + "Injector");
                        break;
                    }
                }
            }
        } else {
            LLVM_DEBUG(dbgs() << "Adding all injectors\n");
            for (auto injector : available) {
                LLVM_DEBUG(dbgs() << "Adding " << injector << "\n");
                injectors.push_back(injector + "Injector");
            }
        }
    } else {
        // no config file => add all available injectors
        LLVM_DEBUG(dbgs() << "Adding all injectors\n");

        for (auto injector : available) {
            LLVM_DEBUG(dbgs() << "Adding " << injector << "\n");
            injectors.push_back(injector + "Injector");
        }
    }

    patchDir = fs::absolute(patchDir);
    std::cout << "Injection patches should be saved in directory '" << patchDir << "'" << std::endl;
    if (!fs::exists(patchDir)) {
        if (!fs::create_directories(patchDir)) {
            std::cerr << "Could not create directory " << patchDir << std::endl;
            return 1;
        }
    }
    patchDir += fs::path::preferred_separator;

    std::vector<std::string> filesToConsider;
    auto cwd = fs::current_path();
    std::string path = cwd.native() + fs::path::preferred_separator;

    std::string rootDir = RootDirectoryOption.getValue();
    if (rootDir.compare("") != 0) {
        if (rootDir.compare(".") == 0 || rootDir.compare("cwd") == 0) {
            rootDir = path;
        }
        if (verbose) {
            std::cout << "Sourcetree directory defined, files in this directory are also considered for matches."
                      << std::endl;
        }
    } else {
        rootDir = path;
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
                if (verbose) {
                    std::cout << "Adding " << file << " to the list of considered files.\n";
                }
                filesToConsider.push_back(file);
            }
        }
    }

    FaultInjectorOptions fiOpt;
    fiOpt.patchDir = patchDir;
    fiOpt.rootDir = rootDir;
    fiOpt.verbose = verbose;
    fiOpt.fileList = filesToConsider;
    fiOpt.matchMacro = true;

    int ret = Tool.run(newSFIFrontendActionFactory(injectors, fiOpt).get());
    if (ret > 2 || ret < 0) {
        std::cerr << "Unknown error occurred." << std::endl;
        return 1;
    }
    if (ret == 2) {
        std::cout << "Some files were skipped, because there was no "
                     "compileCommand "
                     "for them in compile_commands.json!"
                  << std::endl;
    }
    if (ret == 1) {
        std::cout << "An error occurred while running the tool..." << std::endl;
        return 1;
    }

    std::cout << "Operation succeeded." << std::endl;
    return 0;
};
