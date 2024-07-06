import networkx as nx;

def main():
    G = nx.navigable_small_world_graph(4)
    nx.write_adjlist(G, "out.adj")
    print(G);

    G2 = nx.turan_graph(4, 2);
    nx.write_adjlist(G2, "out2.adj")
    print(G2);
 
if __name__ == "__main__":
    main()
