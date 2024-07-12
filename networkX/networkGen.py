import networkx as nx
import random

def main():
    G = nx.navigable_small_world_graph(4)
    nx.write_adjlist(G, "small.adj")
    print(G);

    G2 = nx.turan_graph(4, 2);
    nx.write_adjlist(G2, "out2.adj")
    print(G2);


    # edgeWeights = {}
    # for n in G:
    #     for nbr in G[n]:
    #         G[n][nbr]["weight"] = random.random()
    #         edgeDict = {}
    #         edgeDict["weight"] = G[n][nbr]["weight"]
    #         edgeWeights[n, nbr] = edgeDict
    #

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
