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
#include <regex>
#include "cadmium/simulation/logger/logger.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

struct modelInfo {
    std::map<std::string, std::string> state_variables; // map of strings for {variable name: variable type} pairs
    std::map<std::string, std::pair<std::string, std::string>> variable_values; // map of variable names to current and next value pairs
    std::map<std::string, std::string> in_ports; // map of strings for {in port name: in port type} pairs
    std::map<std::string, std::string> in_port_values; // map of input ports and their current values
    std::map<std::string, std::string> out_ports; // map of strings for {out port name: out port type} pairs
    std::map<std::string, std::string> out_port_values; // map of output ports and their current values
    

    modelInfo copy(){
        modelInfo mi2;
        mi2.state_variables = state_variables;
        mi2.variable_values = variable_values;
        mi2.in_ports = in_ports;
        mi2.in_port_values = in_port_values;
        mi2.out_ports = out_ports;
        mi2.out_port_values = out_port_values;
        return mi2;
    }
};

std::ostream& operator << (std::ostream& os, const modelInfo& mi) {

    os << "Model(" << "s{ ";
    for (const auto& [key, value]: mi.state_variables ){
        os << key << " : " << value << "," << std::endl;
    }
    
    os << "x{ ";
    for (const auto& [key, value]: mi.in_ports){
        os << key << " : " << value << "," << std::endl;
    }

    os << "y{ ";
    for (const auto& [key, value]: mi.out_ports){
        os << key << " : " << value << "," << std::endl;
    }
    os << std::endl;
    return os; 
}

void from_json(const json& j, modelInfo& mi){
    j.at("s").get_to(mi.state_variables);
    j.at("x").get_to(mi.in_ports);
    j.at("y").get_to(mi.out_ports);
}

namespace cadmium {
    class AxiomLogger: public Logger {
        private:
            std::string outputpath; // filepath to the simulation log
            std::string atppath; // filepath to the ATP executeable
            std::string axiomFolderPath; // filepath to the folder with axiom files
            std::string devsmapFolderPath; // filepath to the folder with DEVSMap files
            std::vector<std::string> modelnames; // list of names for each model type
            std::map<long, std::pair<std::string, modelInfo>> activeModels; // Associates every simulated model with its type and unique state
            std::map<std::string, modelInfo> modelTypeInfo; // Stores every model types' variables, in ports, and out ports
            std::ofstream file; // file for the simulation log to be printed to
            std::string sep; // seperator character for the log
            
        public:
            /**
             * Constructor function.
             * @param outputpath filepath to the simulation log
             * @param atppath filepath to the ATP executeable
             * @param axiomFolderPath filepath to the ATP executeable
             * @param devsmapFolderPath; // filepath to the folder with DEVSMap files
             * @param modelnames; // list of names for each model type
             * @param sep; // seperator character for the log
             */
            AxiomLogger(std::string outputpath, 
                    std::string atppath, 
                    std::string axiomFolderPath,
                    std::string devsmapFolderPath,
                    std::vector<std::string> modelnames, 
                    std::string sep): 
                                    Logger(), 
                                    outputpath(std::move(outputpath)), 
                                    atppath(std::move(atppath)), 
                                    axiomFolderPath(std::move(axiomFolderPath)),
                                    devsmapFolderPath(std::move(devsmapFolderPath)),
                                    modelnames(std::move(modelnames)),
                                    sep(std::move(sep)),
                                    file() {}

            // Starts the output file stream and retrieves model info
            void start() override {

                for (std::string modelname : modelnames){
                    std::string devsmapFilePath = devsmapFolderPath + modelname + ".json";

                    std::ifstream devsmapFile(devsmapFilePath);
                    json modelData = json::parse(devsmapFile).at(modelname);
                    modelInfo mi = modelData.get<modelInfo>();

                    for (const auto& [key, value]: mi.state_variables ){
                        mi.variable_values.emplace(key, std::make_pair("",""));
                    }
                    
                    for (const auto& [key, value]: mi.in_ports){
                        mi.in_port_values.emplace(key, "");
                    }

                    for (const auto& [key, value]: mi.out_ports){
                        mi.out_port_values.emplace(key, "");
                    }

                    modelTypeInfo.emplace(modelname, mi);
                }


                file.open(outputpath);
                file << "sep=" << sep << std::endl; //<! makes it easier to open in excel.
                file << "time" << sep << "model_id" << sep << "model_name" << sep << "port_name" << sep << "data" << std::endl;
            }

            // It closes the output file after the simulation.
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
                file << time << sep << modelId << sep << modelName << sep << portName << sep << output << std::endl;
            }

            /**
             * Virtual method to log atomic models' states.
             * @param time current simulation time.
             * @param modelId ID of the model that generated the output message.
             * @param modelName name of the model that generated the output message.
             * @param state string representation of the state.
             */
            void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {
                json next_state = json::parse(state);

                if (!activeModels.contains(modelId)){
                    if (modelTypeInfo.contains(modelName)){
                        std::pair modelTypeAndState = std::make_pair(modelName,modelTypeInfo[modelName].copy());
                        activeModels.emplace(modelId,modelTypeAndState);
                    } else {
                        assert(("The model name must match the model type",false));
                    }
                } else {
                    thisModelInfo = activeModels.at(modelId).second;
                    for (const auto& [key, value]: thisModelInfo.variable_values){
                        value.first = value.second;
                        value.second = "";
                        thisModelInfo.variable_values[key] = value;
                    }
                }
                



                file << time << sep << modelId << sep << modelName << sep << sep << state << std::endl;
            }

            /**
             * Method for converting a string statement to be Thousands of Problems for Theorem Provers (TPTP) compatible
             * @param value a string representing a variable's value (Ex: "true", "speed", "57")
             */
            std::string tptpConversion(std::string value){
                std::regex true_regex(R"^[t|T]rue$|^TRUE$");
                std::regex false_regex(R"^[f|F]alse$|^FALSE$");
                std::regex int_regex(R"^\d*$");
                if (std::regex_match(value,true_regex)){
                    return "$true";
                } else if (std::regex_match(value,false_regex)){
                    return "$false";
                } else if (std::regex_match(value,int_regex)){
                    return value + ".0";
                } 
            }

            /**
             * Method for building a conjecture string that is a conjunction of the given axioms
             * @param next_state_values is a list of strings representing the axioms that describe the next state's variables
             */
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
                /**
                
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
                */
            }
    };
}

#endif // CADMIUM_SIMULATION_LOGGER_AXIOM_HPP_