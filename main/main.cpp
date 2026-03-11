#include <limits>
#include <map>
#include <string>
#include "include/Counter_Top.hpp"
#include "include/axiom_logger.hpp"
#include "cadmium/simulation/root_coordinator.hpp"
#include "cadmium/simulation/logger/stdout.hpp"
#include "cadmium/simulation/logger/csv.hpp"

using namespace cadmium;

int main(int argc, char* argv[]){
	auto counter_top = std::make_shared<Counter_Top> ("counter_top");

	auto rootCoordinator = RootCoordinator(counter_top);

	//std::map<std::string, std::string> modelAxiomPaths = {{"counter","../main/model_axioms/counter.p"}};
	rootCoordinator.setLogger<CSVLogger>("../simulation_results/counter_test.csv",",");

	rootCoordinator.start();
	rootCoordinator.simulate(30.0);
	rootCoordinator.stop();	

	return 0;
}