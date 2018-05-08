#include "ComponentNetwork.hpp"
#include "HypergraphYAML.hpp"

#include <fstream>
#include <iostream>
#include <cassert>

int main(void)
{
    std::cout << "*** COMPONENT NETWORK TEST ***\n";
    Component::Network cnd;

    std::cout << "> Create two component classes\n";
    std::cout << cnd.createComponent("MyFirstComponent", "A") << "\n";
    std::cout << cnd.createComponent("MySecondComponent", "B") << "\n";

    std::cout << "> Create common interface class\n";
    std::cout << cnd.createInterface("CommonInterface", "Interface") << "\n";

    std::cout << "> Each component shall get two common interfaces\n";
    std::cout << cnd.hasInterface(Hyperedges{"MyFirstComponent"},  cnd.instantiateFrom(Hyperedges{"CommonInterface"}, "x")) << "\n";
    std::cout << cnd.hasInterface(Hyperedges{"MyFirstComponent"},  cnd.instantiateFrom(Hyperedges{"CommonInterface"}, "y")) << "\n";
    std::cout << cnd.hasInterface(Hyperedges{"MySecondComponent"}, cnd.instantiateFrom(Hyperedges{"CommonInterface"}, "u")) << "\n";
    std::cout << cnd.hasInterface(Hyperedges{"MySecondComponent"}, cnd.instantiateFrom(Hyperedges{"CommonInterface"}, "v")) << "\n";

    std::cout << "> Instantiate components and connect their interfaces\n";
    std::cout << cnd.instantiateComponent(Hyperedges{"MyFirstComponent", "MySecondComponent"}) << "\n";

    std::cout << "> Query their interfaces\n";
    std::cout << cnd.interfacesOf(Hyperedges{"MyFirstComponent1","MySecondComponent1"}) << "\n";

    std::cout << "> Connect interface y of A to interface u of B\n";
    std::cout << cnd.connectInterface(cnd.interfacesOf(Hyperedges{"MyFirstComponent1"},"y"), cnd.interfacesOf(Hyperedges{"MySecondComponent1"},"u")) << "\n";

    std::cout << "> Make the components part of a network\n";
    std::cout << cnd.partOfNetwork(Hyperedges{"MyFirstComponent1", "MySecondComponent1"}, cnd.createNetwork("MyFirstNetwork","C")) << "\n";

    // TODO: Make this a convenient function
    std::cout << "> Export interfaces to network level\n";
    std::cout << cnd.hasInterface(Hyperedges{"MyFirstNetwork"}, unite(cnd.interfacesOf(Hyperedges{"MyFirstComponent1"},"x"), cnd.interfacesOf(Hyperedges{"MySecondComponent1"},"v"))) << "\n";

    std::cout << "> Store component network using YAML" << std::endl;
    std::ofstream fout;
    fout.open("cnd.yml");
    if(fout.good()) {
        fout << YAML::StringFrom(cnd) << std::endl;
    } else {
        std::cout << "FAILED\n";
    }
    fout.close();

    std::cout << "*** TEST DONE ***\n";
}
