#include <fstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "llvm/Support/raw_ostream.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

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
    CommonOptionsParser op(argc, argv, oCategory);

    std::string fileforInjection = "";
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    if (op.getSourcePathList().size() != 1) {
        std::cout << "Please select exactly one main file" << std::endl;
        return 1;
    } else {
        fileforInjection = op.getSourcePathList()[0];
    }

    std::vector<FaultInjector *> available;
    std::vector<FaultInjector *> injectors;

    bool verbose = VerboseOption.getValue();

    // add an instance of every available FaultInjector to list
    // only this instaces can be added by the config
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

    for (FaultInjector *inj : injectors) {
        // set verbose and directory options for injectors
        inj->setVerbose(verbose);
        inj->setDirectory(dir);
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
        // create overview in summary.json
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
            float part = ((float)size) / injectioncount * 100.0;

            std::cout << "Injected " << size << " " << type << " faults." << std::endl
                 << "> " << size << "/" << injectioncount << " (" << roundf(part * 100) / 100 << "\%)" << std::endl;
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
