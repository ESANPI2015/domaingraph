#include "SoftwareGraph.hpp"
#include "HypergraphYAML.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <getopt.h>

static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"uid", required_argument, 0, 'u'},
    {"label", required_argument, 0, 'l'},
    {"type-uid", required_argument, 0, 't'},
    {0,0,0,0}
};

void usage (const char *myName)
{
    std::cout << "Usage:\n";
    std::cout << myName << " --uid=<UID> --label=<label> <yaml-file-in> <yaml-file-out>\n\n";
    std::cout << "Options:\n";
    std::cout << "--help\t" << "Show usage\n";
    std::cout << "--uid=<uid>\t" << "Specify the algorithm to be used to generate code by UID\n";
    std::cout << "--label=<label>\t" << "Specify the algorithm(s) to be used to generate code by label\n";
    std::cout << "--type-uid=<uid>\t" << "Specify the datatype class which hosts compatible types\n";
    std::cout << "\nExample:\n";
    std::cout << myName << "--label=MyAlgorithm initial_model.yml new_model.yml\n";
}

std::string genTypeFromLabel(const std::string& label)
{
    return label+"_type";
}

std::string genPartIdentifier(const UniqueId& partUid)
{
    return "component_"+partUid;
}

std::string genInterfaceIdentifier(const std::string& label)
{
    return "interface_"+label;
}

std::string genInputIdentifier(const std::string& label)
{
    return "input_"+label;
}

std::string genOutputIdentifier(const std::string& label)
{
    return "output_"+label;
}

int main (int argc, char **argv)
{
    UniqueId uid, vhdlDatatypeUid;
    std::string label;
    // Parse command line
    int c;
    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "h", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case 't':
                vhdlDatatypeUid=std::string(optarg);
                break;
            case 'u':
                uid=std::string(optarg);
                break;
            case 'l':
                label=std::string(optarg);
                break;
            case 'h':
            case '?':
                break;
            default:
                std::cout << "W00t?!\n";
                return 1;
        }
    }

    if ((argc - optind) < 2)
    {
        usage(argv[0]);
        return 1;
    }

    // Set vars
    std::string fileNameIn(argv[optind]);
    std::string fileNameOut(argv[optind+1]);
    Software::Graph swgraph(YAML::LoadFile(fileNameIn).as<Hypergraph>());

    // Get all algorithm classes (without overall superclass)
    // and find the candidate(s)
    Hyperedges algorithms(swgraph.algorithmClasses(label));
    if (!uid.empty())
        algorithms = intersect(algorithms, Hyperedges{uid});

    if (!algorithms.size())
    {
        std::cout << "No algorithm found.\n";
        return 2;
    }

    // Set our language
    const UniqueId& vhdlImplementationUid("Software::Graph::Implementation::VHDL");
    swgraph.createImplementation(vhdlImplementationUid, "VHDLImplementation");

    // Find relevant datatypeClasses
    Hyperedges relevantTypeUids;
    if (!vhdlDatatypeUid.empty())
        relevantTypeUids = swgraph.datatypeClasses("",Hyperedges{vhdlDatatypeUid});
    else
        relevantTypeUids = swgraph.datatypeClasses();

    // Get some constants
    Hyperedges allInputUids(swgraph.inputs());
    Hyperedges allOutputUids(swgraph.outputs());

    // For each of these algorithms
    for (const UniqueId& algorithmId : algorithms)
    {
        Hyperedges myInterfaceClassIds;
        std::stringstream result;
        Hyperedge*  algorithm(swgraph.get(algorithmId));

        result << "-- Algorithm to VHDL entity generator --\n";
        result << "library IEEE;\n";
        result << "use IEEE.STD_LOGIC_1164.ALL;\n";
        result << "\nuse work." << algorithm->label() << "_types.all;\n";
        result << "\nentity " << algorithm->label() << " is\n";
        result << "port(\n";

        // Handle Inputs (input instances which are interfaces of algorithmId
        Hyperedges myInputIds(intersect(swgraph.inputs(), swgraph.interfacesOf(Hyperedges{algorithmId})));
        result << "\n\t-- Inputs --\n";
        for (const UniqueId& inputId : myInputIds)
        {
            // This input is of a certain type we have to find
            Hyperedges inputClassUids(swgraph.instancesOf(inputId,"",Hypergraph::TraversalDirection::FORWARD));
            for (const UniqueId& classUid : inputClassUids)
            {
                std::string typeOfInput(swgraph.get(classUid)->label());
                result << "\t" << genInputIdentifier(swgraph.get(inputId)->label()) << " : in " << genTypeFromLabel(typeOfInput) << ";\n";
            }
            // Put input classes to the interface classes we need later
            myInterfaceClassIds.insert(inputClassUids.begin(), inputClassUids.end());
        }
        // Handle Outputs
        Hyperedges myOutputIds(intersect(swgraph.outputs(), swgraph.interfacesOf(Hyperedges{algorithmId})));
        result << "\n\t-- Outputs --\n";
        for (const UniqueId& outputId : myOutputIds)
        {
            // This output is of a certain type we have to find
            Hyperedges outputClassUids(swgraph.instancesOf(outputId,"",Hypergraph::TraversalDirection::FORWARD));
            for (const UniqueId& classUid : outputClassUids)
            {
                std::string typeOfOutput(swgraph.get(classUid)->label());
                result << "\t" << genOutputIdentifier(swgraph.get(outputId)->label()) << " : out " << genTypeFromLabel(typeOfOutput) << ";\n";
            }
            // Put output classes to the interface classes we need later
            myInterfaceClassIds.insert(outputClassUids.begin(), outputClassUids.end());
        }
        result << "\n\t-- Standard Signals --\n";
        //result << "\tstart : in std_logic;\n";
        //result << "\tvalid : out std_logic;\n";
        result << "\tclk : in std_logic;\n";
        result << "\trst : in std_logic\n";
        result << ");\n";
        result << "end\n";

        // Find subcomponents
        Hyperedges parts(swgraph.componentsOf(Hyperedges{algorithmId}));
        // Handle architecture
        if (!parts.size())
        {
            // Architecture for atomic entity
            result << "\n-- Architecture def --\n";
            result << "architecture BEHAVIOURAL of " << algorithm->label() << " is\n";
            result << "-- signals here --\n";
            result << "\nbegin\n";
            result << "-- processes here --\n";
            result << "compute : process(clk)\n";
            result << "\t-- variables here --\n";
            result << "\tbegin\n";
            result << "\t\tif rising_edge(clk) then\n";
            result << "\t\t\tif (rst='1') then\n";
            result << "\t\t\t\t-- init here --\n";
            result << "\t\t\telse\n";
            result << "\t\t\t\t-- computation here --\n";
            result << "\t\t\tend if;\n";
            result << "\t\tend if;\n";
            result << "end process compute;\n";
            result << "end BEHAVIORAL;\n";
        } else {
            // FIXME: Handle output to multiple inputs!!!! That means that we have to create ONE common signal per output and assign signals
            // TODO:
            // * For each component, create ONE signal per input and ONE signal per OUTPUT!!!!
            result << "\n-- Architecture def --\n";
            result << "architecture BEHAVIOURAL of " << algorithm->label() << " is\n";
            result << "-- signals here --\n";
            // I: Create signals for each component
            result << "-- signals of parts --\n";
            for (const UniqueId& partUid : parts)
            {
                Hyperedges partInputUids(intersect(allInputUids, swgraph.interfacesOf(Hyperedges{partUid})));
                for (const UniqueId& partInputUid : partInputUids)
                {
                    Hyperedges interfaceClassUids(swgraph.instancesOf(partInputUid,"",Hypergraph::TraversalDirection::FORWARD));
                    for (const UniqueId& classUid : interfaceClassUids)
                    {
                        result << "signal ";
                        result << genPartIdentifier(partUid) << "_" << genInputIdentifier(swgraph.get(partInputUid)->label());
                        result << " : " << genTypeFromLabel(swgraph.get(classUid)->label()) << ";\n";
                    }
                }
                Hyperedges partOutputUids(intersect(allOutputUids, swgraph.interfacesOf(Hyperedges{partUid})));
                for (const UniqueId& partOutputUid : partOutputUids)
                {
                    Hyperedges interfaceClassUids(swgraph.instancesOf(partOutputUid,"",Hypergraph::TraversalDirection::FORWARD));
                    for (const UniqueId& classUid : interfaceClassUids)
                    {
                        result << "signal ";
                        result << genPartIdentifier(partUid) << "_" << genOutputIdentifier(swgraph.get(partOutputUid)->label());
                        result << " : " << genTypeFromLabel(swgraph.get(classUid)->label()) << ";\n";
                    }
                }
            }
            result << "\nbegin\n";
            // II. Wire toplvl inputs to internal inputs
            result << "-- assignment of toplvl inputs to internal inputs --\n";
            for (const UniqueId& inputId : myInputIds)
            {
                Hyperedges partUids(intersect(parts, swgraph.interfacesOf(Hyperedges{inputId}, "", Hypergraph::TraversalDirection::INVERSE)));
                for (const UniqueId& partUid : partUids)
                {
                    result << genPartIdentifier(partUid) << "_" << genInputIdentifier(swgraph.get(inputId)->label());
                    result << " <= ";
                    result << genInputIdentifier(swgraph.get(inputId)->label());
                    result << ";\n";
                }
            }
            // III. Wire internal outputs to toplvl outputs
            result << "-- assignment of internal outputs to toplvl outputs --\n";
            for (const UniqueId& outputId : myOutputIds)
            {
                Hyperedges partUids(intersect(parts, swgraph.interfacesOf(Hyperedges{outputId}, "", Hypergraph::TraversalDirection::INVERSE)));
                for (const UniqueId& partUid : partUids)
                {
                    result << genOutputIdentifier(swgraph.get(outputId)->label());
                    result << " <= ";
                    result << genPartIdentifier(partUid) << "_" << genOutputIdentifier(swgraph.get(outputId)->label());
                    result << ";\n";
                }
            }
            // IV. Instantiate and wire parts
            result << "-- part entity instantiation & wiring--\n";
            for (const UniqueId& partUid : parts)
            {
                Hyperedges partInterfaceUids(swgraph.interfacesOf(Hyperedges{partUid}));
                Hyperedges partInputUids(intersect(allInputUids, partInterfaceUids));
                Hyperedges partOutputUids(intersect(allOutputUids, partInterfaceUids));
                Hyperedges superclasses(swgraph.instancesOf(partUid,"",Hypergraph::TraversalDirection::FORWARD));
                // V. Assign component input signals to other components output signals
                result << "-- assignment of internal outputs to internal inputs --\n";
                for (const UniqueId& partInputUid : partInputUids)
                {
                    Hyperedges partOutputUids(swgraph.endpointsOf(Hyperedges{partInputUid},"",Hypergraph::TraversalDirection::INVERSE));
                    for (const UniqueId& partOutputUid : partOutputUids)
                    {
                        Hyperedges producerUids(intersect(parts, swgraph.interfacesOf(Hyperedges{partOutputUid}, "", Hypergraph::TraversalDirection::INVERSE)));
                        for (const UniqueId& producerUid : producerUids)
                        {
                            result << genPartIdentifier(partUid) << "_" << genInputIdentifier(swgraph.get(partInputUid)->label());
                            result << " <= ";
                            result << genPartIdentifier(producerUid) << "_" << genOutputIdentifier(swgraph.get(partOutputUid)->label());
                            result << ";\n";
                        }
                    }
                }
                result << "-- instantiate entity --\n";
                for (const UniqueId& superUid : superclasses)
                {
                    result << genPartIdentifier(partUid) << ": entity work." << swgraph.get(superUid)->label() << "\n";
                    result << "port map (\n";
                    result << "\t-- inputs --\n";
                    // VI. Wire to corresponding signals
                    for (const UniqueId& partInputUid : partInputUids)
                    {
                            result << "\t";
                            result << genInputIdentifier(swgraph.get(partInputUid)->label()) << " => ";
                            result << genPartIdentifier(partUid) << "_" << genInputIdentifier(swgraph.get(partInputUid)->label());
                            result << ",\n";
                    }
                    result << "\t-- outputs --\n";
                    for (const UniqueId& partOutputUid : partOutputUids)
                    {
                            result << "\t";
                            result << genOutputIdentifier(swgraph.get(partOutputUid)->label()) << " => ";
                            result << genPartIdentifier(partUid) << "_" << genOutputIdentifier(swgraph.get(partOutputUid)->label());
                            result << ",\n";
                    }
                    result << "\tclk => clk,\n";
                    result << "\trst => rst\n";
                    result << ");\n";
                }
            }
            result << "-- processes here --\n";
            result << "end BEHAVIORAL;\n";
        }

        // Handle the collected interface classes
        result << "\n-- Package def --\n";
        result << "package " << algorithm->label() << "_types is\n";
        for (const UniqueId& interfaceClassId : myInterfaceClassIds)
        {
            Hyperedge* interface(swgraph.get(interfaceClassId));
            Hyperedges typeUids(intersect(relevantTypeUids, swgraph.directSubclassesOf(Hyperedges{interfaceClassId},"",Hypergraph::TraversalDirection::INVERSE)));
            for (const UniqueId& typeUid : typeUids)
            {
                std::string datatypeName(swgraph.get(typeUid)->label());
                result << "\ttype " << genTypeFromLabel(interface->label()) << " is " << datatypeName << ";\n";
            }
        }
        result << "end " << algorithm->label() << "_types;\n";

        // Create implementation for algorithm with this result
        const UniqueId& implId(vhdlImplementationUid+"::"+algorithm->label());
        swgraph.createImplementation(implId, result.str(), Hyperedges{vhdlImplementationUid});
        swgraph.isA(Hyperedges{implId}, Hyperedges{algorithmId});
    }

    // Store graph
    std::ofstream fout;
    fout.open(fileNameOut);
    if(fout.good()) {
        fout << YAML::StringFrom(swgraph) << std::endl;
    } else {
        std::cout << "FAILED\n";
    }
    fout.close();

    return 0;
}
