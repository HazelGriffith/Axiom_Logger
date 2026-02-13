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

namespace fs = std::filesystem;

namespace cadmium {
    class AxiomLogger: public Logger {
        private:
            std::string outputpath; // filepath to the transition checks
            std::string atppath; // filepath to the ATP executeable
            std::map<std::string, std::map<std::string, std::string>> modelSpecPaths; // filepaths to axiom files for each atomic model
            std::ofstream file;
            std::map<std::string, std::map<std::string, std::map<std::string, std::string>> state_variables_per_model; // state variable {name,{type: $type}{value: $val} pairs for each atomic model
            std::map<std::string, bool> firstLog_per_model; // is true for given model if it has not been logged yet
            std::string language;
            double curr_time = 0;
        public:
            /**
             * Constructor function.
             * @param outputpath filepath to the transition checks
             * @param atppath filepath to the ATP executeable
             */
            AxiomLogger(std::string outputpath, 
                    std::string atppath, 
                    std::map<std::string, std::string> modelAxiomPaths,

                    std::string language): 
                                    Logger(), 
                                    outputpath(std::move(outputpath)), 
                                    atppath(std::move(atppath)), 
                                    modelAxiomPaths(std::move(modelAxiomPaths)), 
                                    language(std::move(language)),
                                    file() {}

            // Starts the output file stream
            void start() override {
                for (auto specPaths : modelSpecPaths){
                    auto modelName = specPaths.first;
                    auto devsPath = specPaths.second.at("devsmap");
                    auto axiomsPath = specPaths.second.at("axioms");

                    std::ifstream devsmapFile(devsPath);

                    nlohmann::json modelData = nlohmann::json::parse(devsmapFile);
                    

                }



                for (auto modelAxiomPath : modelAxiomPaths){
                    auto modelName = modelAxiomPath.first;
                    auto filepath = modelAxiomPath.second;

                    std::ifstream modelAxiomFile(filepath);
                    std::string line;
                    std::map<std::string, std::map<std::string, std::string>> variables;

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
                                std::map<std::string, std::string> variable = {{"type",""},{"value",""}};
                                variables.insert({line.substr(start, end - start),variable});
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
                } else {
                    // Just print it
                }
            }

            std::string buildConjecture(std::vector<std::string> next_state_values){
                std::string conjecture = "";
                for (int i = 0; i < next_state_values.size(); i++){
                    conjecture += "(" + next_state_values[i] + ")";
                    if (i < next_state_values.size()-1){
                        conjecture += "&";
                    }
                }
                return conjecture;
            }

            void logModel(double time, long modelId, const std::shared_ptr<AtomicInterface>& model, bool logOutput) override {
                
                if (time > curr_time){
                    curr_time = time;
                }

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
                    std::string new_lines = "";
                    std::string conjecture = language + "(next_state_conjecture,conjecture,(";
                    std::vector<std::string> next_state_values;

                    for (auto variable : state_variable_values_per_model[modelName]){
                        std::string variableName = variable.first;
                        auto variableNameLength = variableName.size();
                        auto position = state.find(variableName)+variableNameLength+1;
                        auto end_pos = state.find(";",position);
                        std::string next_value = state.substr(position,end_pos-position);
                        std::string curr_value = state_variable_values_per_model.at(modelName).at(variableName);
                        std::string curr_value_axiom = language + "(" + variableName + "_value,axiom,(" + variableName + " = " + curr_value + ")).\n";
                        new_lines += curr_value_axiom;
                        next_state_values.push_back("next_" + variableName + " = " + next_value);
                    }

                    new_lines += "\n";

                    for (const auto& inPort: model->getInPorts()) {

                        std::string portName = inPort->getId();
                        std::size_t number_of_msgs = inPort->size();

                        std::string number_of_msgs_axiom = language + "(" + portName + "_msgs_rcvd,axiom,(num_rcvd(" + portName + ") = ";
                        if (number_of_msgs > 0){
                            number_of_msgs_axiom += std::to_string(number_of_msgs) + ")).\n";

                            std::string value_rcvd = inPort->logMessage(number_of_msgs-1);
                            std::string val_rcvd_axiom = language + "(val_rcvd_" + portName + "_value,axiom,(val_rcvd_" + portName + " = " + value_rcvd + ")).\n";
                            new_lines += val_rcvd_axiom;

                        } else {
                            number_of_msgs_axiom += "0)).\n";
                        }
                        new_lines += number_of_msgs_axiom;

                        /**
                        for (std::size_t i = 0; i < inPort->size(); ++i) {
                            this->logOutput(time, modelId, model->getId(), inPort->getId(), inPort->logMessage(i));
                        }
                        */
                    }

                    new_lines += "\n";

                    if (logOutput){
                        for (const auto& outPort: model->getOutPorts()) {
                            std::string portName = outPort->getId();
                            std::size_t number_of_msgs = outPort->size();

                            std::string number_of_msgs_axiom = language + "(" + portName + "_msgs_output,axiom,(num_output(" + portName + ") = ";
                            if (number_of_msgs > 0){
                                number_of_msgs_axiom += std::to_string(number_of_msgs) + ")).\n";

                                std::string value_output = outPort->logMessage(number_of_msgs-1);
                                std::string val_output_axiom = language + "(val_output_" + portName + "_value,axiom,(val_output_" + portName + " = " + value_output + ")).\n";
                                new_lines += val_output_axiom;

                            } else {
                                number_of_msgs_axiom += "0)).\n";
                            }

                            new_lines += number_of_msgs_axiom;
                        }
                    }
                    
                    new_lines += "\n";

                    conjecture += buildConjecture(next_state_values) + ")).\n";
                    new_lines += conjecture;

                    std::ofstream temp_problem_file(tempFilepath,std::ios::app);
                    if (temp_problem_file.is_open()){
                        temp_problem_file << new_lines << std::endl;
                    }
                    temp_problem_file.close();
                }
                
            }
    };
}

#endif // CADMIUM_SIMULATION_LOGGER_AXIOM_HPP_