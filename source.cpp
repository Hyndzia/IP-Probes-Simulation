#include "json.hpp"
#include "simulate.h"

using json = nlohmann::json;

int main() {
    Simulate simulation;

    simulation.loadConfigs();

    simulation.startAllProbes();
    simulation.startAllReceivers();

    simulation.stopAllProbes();
    simulation.stopAllReceivers();

    system("pause");
    return 0;
}
