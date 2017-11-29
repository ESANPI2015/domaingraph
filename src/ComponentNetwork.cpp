#include "ComponentNetwork.hpp"

namespace Component {

const UniqueId Network::ComponentId            = "Component::Network::Component";
const UniqueId Network::InterfaceId            = "Component::Network::Interface";
const UniqueId Network::NetworkId              = "Component::Network::Network";
const UniqueId Network::HasAInterfaceId        = "Component::Network::HasAInterface";
const UniqueId Network::ConnectedToInterfaceId = "Component::Network::ConnectedToInterface";
const UniqueId Network::PartOfNetworkId        = "Component::Network::PartOfNetwork";

        // Constructor/Destructor
Network::Network()
{
    createMainConcepts();
}

Network::Network(CommonConceptGraph& A)
: CommonConceptGraph(A)
{
    createMainConcepts();
}
Network::~Network()
{
}

void Network::createMainConcepts()
{
    create(Network::ComponentId, "COMPONENT");
    create(Network::InterfaceId, "INTERFACE");
    create(Network::NetworkId, "NETWORK");

    relate(Network::HasAInterfaceId, Hyperedges{Network::ComponentId, Network::NetworkId}, Hyperedges{Network::InterfaceId}, "HAS-A");
    subrelationOf(Hyperedges{Network::HasAInterfaceId}, Hyperedges{CommonConceptGraph::HasAId});
    relate(Network::ConnectedToInterfaceId, Hyperedges{Network::InterfaceId}, Hyperedges{Network::InterfaceId}, "CONNECTED-TO");
    subrelationOf(Hyperedges{Network::ConnectedToInterfaceId}, Hyperedges{CommonConceptGraph::ConnectsId});
    relate(Network::PartOfNetworkId, Hyperedges{Network::ComponentId}, Hyperedges{Network::NetworkId}, "PART-OF");
    subrelationOf(Hyperedges{Network::PartOfNetworkId}, Hyperedges{CommonConceptGraph::PartOfId});
    isA(Hyperedges{Network::NetworkId},Hyperedges{Network::ComponentId});
}

Hyperedges Network::createComponent(const UniqueId& uid, const std::string& name, const Hyperedges& suids)
{
    if(!isA(create(uid, name), intersect(unite(Hyperedges{Network::ComponentId}, suids), componentClasses())).empty())
        return Hyperedges{uid};
    return Hyperedges();
}

Hyperedges Network::createInterface(const UniqueId& uid, const std::string& name, const Hyperedges& suids)
{
    if (!isA(create(uid, name), intersect(unite(Hyperedges{Network::InterfaceId}, suids), interfaceClasses())).empty())
        return Hyperedges{uid};
    return Hyperedges();
}

Hyperedges Network::createNetwork(const UniqueId& uid, const std::string& name, const Hyperedges& suids)
{
    if (!isA(create(uid, name), intersect(unite(Hyperedges{Network::NetworkId}, suids), networkClasses())).empty())
        return Hyperedges{uid};
    return Hyperedges();
}
Hyperedges Network::componentClasses(const std::string& name, const Hyperedges& suids)
{
    Hyperedges all(subclassesOf(Hyperedges{Network::ComponentId}, name));
    if (!suids.empty())
        all = intersect(all, subclassesOf(suids, name));
    return all;
}
Hyperedges Network::interfaceClasses(const std::string& name, const Hyperedges& suids)
{
    Hyperedges all(subclassesOf(Hyperedges{Network::InterfaceId}, name));
    if (!suids.empty())
        all = intersect(all, subclassesOf(suids, name));
    return all;
}
Hyperedges Network::networkClasses(const std::string& name, const Hyperedges& suids)
{
    Hyperedges all(subclassesOf(Hyperedges{Network::NetworkId}, name));
    if (!suids.empty())
        all = intersect(all, subclassesOf(suids, name));
    return all;
}

Hyperedges Network::instantiateComponent(const Hyperedges& componentIds)
{
    return instantiateDeepFrom(componentIds);
}

Hyperedges Network::components(const std::string& name, const std::string& className)
{
    // Get all super classes
    Hyperedges classIds = componentClasses(className);
    // ... and then the instances of them
    return instancesOf(classIds, name);
}
Hyperedges Network::interfaces(const std::string& name, const std::string& className)
{
    // Get all super classes
    Hyperedges classIds = interfaceClasses(className);
    // ... and then the instances of them
    return instancesOf(classIds, name);
}
Hyperedges Network::networks(const std::string& name, const std::string& className)
{
    // Get all super classes
    Hyperedges classIds = networkClasses(className);
    // ... and then the instances of them
    return instancesOf(classIds, name);
}
Hyperedges Network::hasInterface(const Hyperedges& componentIds, const Hyperedges& interfaceIds)
{
    Hyperedges fromIds(intersect(componentIds, unite(componentClasses(), components())));
    Hyperedges toIds(intersect(interfaceIds, interfaces()));
    if (fromIds.size() && toIds.size())
        return CommonConceptGraph::relateFrom(fromIds, toIds, Network::HasAInterfaceId);
    return Hyperedges();
}
Hyperedges Network::connectInterface(const Hyperedges& fromInterfaceIds, const Hyperedges& toInterfaceIds)
{
    Hyperedges valid(interfaces());
    Hyperedges fromIds(intersect(fromInterfaceIds, valid));
    Hyperedges toIds(intersect(toInterfaceIds, valid));
    if (fromIds.size() && toIds.size())
        return CommonConceptGraph::relateFrom(fromIds, toIds, Network::ConnectedToInterfaceId);
    return Hyperedges();
}

Hyperedges Network::partOfNetwork(const Hyperedges& componentIds, const Hyperedges& networkIds)
{
    Hyperedges fromIds(intersect(componentIds, components()));
    Hyperedges toIds(intersect(networkIds, unite(networks(), networkClasses())));
    if (fromIds.size() && toIds.size())
        return CommonConceptGraph::relateFrom(fromIds, toIds, Network::PartOfNetworkId);
    return Hyperedges();
}
Hyperedges Network::interfacesOf(const Hyperedges& componentIds, const std::string& name)
{
    Hyperedges children(childrenOf(componentIds,name));
    Hyperedges ifs(interfaces(name));
    return intersect(children, ifs);
}

}
