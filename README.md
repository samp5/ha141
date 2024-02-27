## Honors Supplement for CS 141
Project for CS 141 Honors Supplement

### Folder Structure

```
├── src
│   ├── main.cpp
│   └── node.hpp
├── pthred_ex //practice pthread examples)
│   └── ...
└── .gitignore
```

### Update 2/28

<details>
<summary> Example Output </summary>
<br>

```
Node 1 added
Node 2 added
Node 3 added
Adding Random Neighbors
Edge from Node 1 to Node 2 added
Edge from Node 2 to Node 3 added
Node 1 is waiting
Node 3 is waiting
Node 1 is activated, setting accumulated to 1
Node 1 is running
Node 1 is sending a message to Node2
Accumulated value for Node 1 is 1
Weight for Node 1 to Node 2 is 4
Message is 4
Node 2 is activated, setting accumulated to 4
Node 2 is running
Node 2 is sending a message to Node3
Accumulated value for Node 2 is 4
Weight for Node 2 to Node 3 is 3
Message is 12
Node 3 is activated, setting accumulated to 12
Node 3 is running
Total Value is 12
```
</details>

<details>
<summary> Random neighbor function </summary>
<br>

- This funciton still needs adjustments to avoid reflexive edges and repeat edges

```cpp
void random_neighbors(vector<Node *> nodes, int number_neighbors) {
  cout << "Adding Random Neighbors\n";
  int size = nodes.size();
  int i = 0;
  while (i < number_neighbors) {
    int from = rand() % size;
    int to = rand() % size;
    if (from == to) {
      continue;
    }

    nodes[from]->add_neighbor(nodes[to], rand() % 5 + 1);
    cout << "Edge from Node " << from + 1 << " to Node " << to + 1
         << " added\n";
    i++;
  }
}
```
</details>
<details>
<summary>Basic Node class that can send and recieve messages from other nodes:</summary>
<br>

```
class Node{
    Node(int _id);
    ~Node();

    void add_neighbor(Node *neighbor, double weight);
    void* run();
    void start_thread();
    void join_thread();
    void activate();
    void deactivate();
    static void* thread_helper(void* instance);
}
```
</details>


