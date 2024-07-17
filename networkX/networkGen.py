import networkx as nx
import random

def main():
    G = nx.navigable_small_world_graph(10, seed=1)
    print(G);
    print(type(nx.to_dict_of_dicts(G)));
    print(nx.to_dict_of_dicts(G));

    edgeWeights = {}
    for n in G:
        for nbr in G[n]:
            G[n][nbr]["weight"] = random.random()
            edgeDict = {}
            edgeDict["weight"] = G[n][nbr]["weight"]
            edgeWeights[n, nbr] = edgeDict

    dod = nx.to_dict_of_dicts(G);
    print(type(dod))
    print(dod);
    firstLevel = dod[0,0]
    print(type(firstLevel))
    print(firstLevel)
    secondLevel = firstLevel[0,1];
    print(type(secondLevel))
    print(secondLevel)
    thirdLevel = secondLevel["weight"];
    print(type(thirdLevel))
    print(thirdLevel)
    # for key, val in edgeWeights.items():
    #     edgeWeights[key]["weight"] = val["weight"]+ 10;
    #
    # nx.write_weighted_edgelist(G, "weight.adj")
    # nx.set_edge_attributes(G, edgeWeights);
    #
    #
    # print(nx.get_edge_attributes(G, "weight"))
 
if __name__ == "__main__":
    main()
