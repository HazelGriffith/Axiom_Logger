#ifndef CADMIUM_SIMULATION_LOGGER_AXIOM_HPP_
#define CADMIUM_SIMULATION_LOGGER_AXIOM_HPP_

#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include "cadmium/simulation/logger/logger.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

struct modelInfo {
    std::map<std::string, std::string> state_variables; // map of strings for {variable name: current value} pairs
    std::map<std::string, std::string> in_ports; // map of strings for {in port name: in port value} pairs
    std::map<std::string, std::string> out_ports; // map of strings for {out port name: out port value} pairs
    std::string axiomFilePath;
    std::string DEVSMapFilePath;
};

void from_json(const json& j, modelInfo& mi){
    j.at("s").get_to(mi.state_variables);
    j.at("x").get_to(mi.in_ports);
    j.at("y").get_to(mi.out_ports);
}

namespace cadmium {
    class AxiomLogger: public Logger {
        private:
            std::string outputpath; // filepath to the transition checks
            std::string atppath; // filepath to the ATP executeable
            std::ofstream file;
            std::map<std::string, modelInfo> state_per_model; // Stores every model's variables, in ports, and out ports
            
        public:
            /**
             * Constructor function.
             * @param outputpath filepath to the transition checks
             * @param atppath filepath to the ATP executeable
             */
            AxiomLogger(std::string outputpath, 
                    std::string atppath, 
                    std::map<std::string, std::string> modelAxiomPaths): 
                                                Logger(), 
                                                outputpath(std::move(outputpath)), 
                                                atppath(std::move(atppath)), 
                                                modelAxiomPaths(std::move(modelAxiomPaths)), 
                                                file() {}

            // Starts the output file stream
            void start() override {

                for (auto modelDEVSMapPath : modelDEVSMapPaths){
                    auto modelName = modelDEVSMapPath.first;
                    auto filepath = modelDEVSMapPath.second;

                    std::map<std::string, std::string> variables;
                    std::map<std::string, std::string> inPorts;
                    std::map<std::string, std::string> outPorts;

                    std::ifstream modelDEVSMapFile(filepath);
                    json modelData = json::parse(modelDEVSMapFile);
                    modelInfo mi = modelData.get<modelInfo>();

                    std::cout << mi << std::endl;

                }
                




                for (auto modelAxiomPath : modelAxiomPaths){
                    auto modelName = modelAxiomPath.first;
                    auto filepath = modelAxiomPath.second;

                    std::ifstream modelAxiomFile(filepath);
                    std::string line;
                    std::map<std::string, std::string> variables;

                    if (modelAxiomFile.is_open()){
                        while (std::getline(modelAxiomFile, line)){
                            if (line == "%-----STATE VARIABLE DEFINITIONS"){
                                break;
                            }
                        }

                        while (std::getline(modelAxiomFile, line)){
                            if (line == "%-----INPUT PORT DEFINITIONS") {
                                break;
                            } else if (line != ""){
                                auto start = line.find("(")+1;
                                auto end = line.find("_type");
                                variables.insert({line.substr(start, end - start),""});
                                std::getline(modelAxiomFile, line);
                                std::getline(modelAxiomFile, line); // skips "next_" variable types
                            }
                        }

                        modelAxiomFile.close();
                    } else {
                        std::cerr << "Unable to open the model axiom file at: " << filepath << std::endl;
                    }

                    state_variable_values_per_model.insert({modelName, variables});
                    firstLog_per_model.insert({modelName, true});
                }


                file.open(outputpath);
                file << "Checking state transition correctness with " << atppath << std::endl;

            }

            //! It closes the output file after the simulation.
            void stop() override {
                file.close();
            }

            /**
             * Virtual method to log atomic models' output messages.
             * @param time current simulation time.
             * @param modelId ID of the model that generated the output message.
             * @param modelName name of the model that generated the output message.
             * @param portName name of the model port in which the output message was created.
             * @param output string representation of the output message.
             */
            void logOutput(double time, long modelId, const std::string& modelName, const std::string& portName, const std::string& output) override {
                std::cout << "\x1B[32m" << time << "," << modelId << "," << modelName << "," << portName << "," << output << "\033[0m" << std::endl;
            }

            /**
             * Virtual method to log atomic models' states.
             * @param time current simulation time.
             * @param modelId ID of the model that generated the output message.
             * @param modelName name of the model that generated the output message.
             * @param state string representation of the state.
             */
            void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {
                if (firstLog_per_model[modelName]){
                    for (auto variable : state_variable_values_per_model[modelName]){
                        std::string variableName = variable.first;
                        auto variableNameLength = variableName.size();
                        auto position = state.find(variableName)+variableNameLength+1;
                        auto end_pos = state.find(";",position);
                        std::string value = state.substr(position,end_pos-position);
                        state_variable_values_per_model.at(modelName).at(variableName) = value;
                    }
                    firstLog_per_model[modelName] = false;
                }
            }


            void logModel(double time, long modelId, const std::shared_ptr<AtomicInterface>& model, bool logOutput) override {
                
                std::string modelName = model->getId();
                if (modelAxiomPaths.count(modelName) != 0){
                    std::string tempFilepath = modelAxiomPaths[modelName].substr(0,modelAxiomPaths[modelName].find(modelName));
                    tempFilepath += modelName + "_temp.p";

                    try{
                        fs::copy_file(modelAxiomPaths[modelName], tempFilepath, fs::copy_options::overwrite_existing);
                    } catch (fs::filesystem_error const& er){
                        std::cerr << "Could not copy the file: " << er.what() << std::endl;
                    }
                    
                    std::string state = model->logState();

                    for (auto variable : state_variable_values_per_model[modelName]){
                        std::string variableName = variable.first;
                        auto variableNameLength = variableName.size();
                        auto position = state.find(variableName)+variableNameLength+1;
                        auto end_pos = state.find(";",position);
                        std::string value = state.substr(position,end_pos-position);

                    }
                }
                
            }
    };
}

#endif // CADMIUM_SIMULATION_LOGGER_AXIOM_HPP_