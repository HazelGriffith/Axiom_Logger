#include <limits>
#include <vector>
#include <map>
#include <string>
#include "include/Counter_Top.hpp"
#include "include/axiom_logger.hpp"
#include "cadmium/simulation/root_coordinator.hpp"

using namespace cadmium;

int main(int argc, char* argv[]){
	auto counter_top = std::make_shared<Counter_Top> ("counter_top");

	auto rootCoordinator = RootCoordinator(counter_top);

	std::string outputPath = "../simulation_results/counter_test.csv";
	std::string atpPath = "../vampire";
	std::string axiomFolderPath = "../main/axiom_files/";
	std::string devsmapFolderPath = "../main/DEVSMap/";
	std::vector<std::string> modelnames;
	modelnames.push_back("counter");

	rootCoordinator.setLogger<AxiomLogger>(outputPath,atpPath,axiomFolderPath,devsmapFolderPath,modelnames);

	rootCoordinator.start();
	rootCoordinator.simulate(30.0);
	rootCoordinator.stop();	

	return 0;
}