#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>

using namespace std;


/*
    Structure used to store one edge of the graph.

    Each edge contains:
    u      -> internal index of the starting vertex
    v      -> internal index of the ending vertex
    weight -> weight of the edge
*/
struct Edge
{
    int u;
    int v;
    int weight;
};


/*
    Disjoint Set Union (DSU) is used to detect cycles.

    parent[i] stores the parent of vertex i.
    rankValue[i] stores the approximate height of the set.

    Initially, every vertex belongs to its own separate set.
*/
class DSU
{
private:
    vector<int> parent;
    vector<int> rankValue;

public:

    /*
        Create V separate sets.

        Initially, every vertex is its own parent because
        every vertex belongs to a separate component.
    */
    DSU(int V)
    {
        parent.resize(V);
        rankValue.resize(V, 0);

        for (int i = 0; i < V; i++)
        {
            parent[i] = i;
        }
    }


    /*
        Find the representative of the set containing vertex x.

        Path compression is used to make future find operations faster.
    */
    int find(int x)
    {
        if (parent[x] == x)
        {
            return x;
        }

        parent[x] = find(parent[x]);

        return parent[x];
    }


    /*
        Combine the sets containing vertices x and y.

        Union by rank is used so that the trees of the
        disjoint sets remain small.
    */
    void unite(int x, int y)
    {
        int rootX = find(x);
        int rootY = find(y);

        if (rootX == rootY)
        {
            return;
        }

        if (rankValue[rootX] < rankValue[rootY])
        {
            parent[rootX] = rootY;
        }
        else if (rankValue[rootX] > rankValue[rootY])
        {
            parent[rootY] = rootX;
        }
        else
        {
            parent[rootY] = rootX;
            rankValue[rootX]++;
        }
    }
};


/*
    Comparator used to sort edges in non-decreasing order
    according to their weights.
*/
bool compareEdges(const Edge& a, const Edge& b)
{
    return a.weight < b.weight;
}


int main()
{
    int V, E;


    /*
        Ask the user for the number of vertices in the graph.
    */
    cout << "Enter the number of vertices: ";
    cin >> V;


    /*
        Ask the user to enter a name or label for each vertex.

        The labels can be numbers, letters, or strings.
        Each label is internally mapped to an integer index
        for DSU operations.
    */
    vector<string> vertexNames(V);
    map<string, int> vertexIndex;

    cout << "\nEnter the vertex names:\n";

    for (int i = 0; i < V; i++)
    {
        cin >> vertexNames[i];

        /*
            Check that the same vertex label is not entered twice.
        */
        if (vertexIndex.find(vertexNames[i]) != vertexIndex.end())
        {
            cout << "Error: Duplicate vertex name entered.\n";
            return 0;
        }

        vertexIndex[vertexNames[i]] = i;
    }


    /*
        Ask the user for the total number of edges in the graph.

        The graph can have more than V-1 edges because Kruskal's
        algorithm chooses V-1 edges from the complete set of edges.
    */
    cout << "\nEnter the number of edges: ";
    cin >> E;


    /*
        Store all the edges entered by the user.
    */
    vector<Edge> edges(E);


    /*
        Ask the user to enter the two endpoint labels and weight
        of every edge.

        The entered labels are converted into integer indices
        using the vertexIndex map so that DSU can process them.
    */
    cout << "\nEnter each edge as: source destination weight\n";

    for (int i = 0; i < E; i++)
    {
        string source, destination;

        cout << "Edge " << i + 1 << ": ";
        cin >> source >> destination >> edges[i].weight;


        /*
            Check whether both endpoint labels exist in the graph.
        */
        if (vertexIndex.find(source) == vertexIndex.end() ||
            vertexIndex.find(destination) == vertexIndex.end())
        {
            cout << "Error: Invalid vertex name entered.\n";
            return 0;
        }


        /*
            Convert the user-provided vertex labels into
            their internal integer indices.
        */
        edges[i].u = vertexIndex[source];
        edges[i].v = vertexIndex[destination];
    }


    /*
        Sort all graph edges in non-decreasing order of their weights.

        This is the first major step of Kruskal's algorithm:
        Kruskal's algorithm considers edges in this order and the smallest edge is picked.
    */
    sort(edges.begin(), edges.end(), compareEdges);


    /*
        Display the edges after sorting so that the user can
        compare the program's execution with a hand calculation.
    */
    cout << "\nEdges in non-decreasing order of weight:\n";

    cout << "Edge\tWeight\n";

    for (const Edge& edge : edges)
    {
        cout << vertexNames[edge.u] << " - "
             << vertexNames[edge.v] << "\t"
             << edge.weight << "\n";
    }


    /*
        Create a Disjoint Set containing all vertices.

        Initially, every vertex is in a separate component.
    */
    DSU dsu(V);


    /*
        Store the edges selected for the Minimum Spanning Tree.
    */
    vector<Edge> mst;


    /*
        Store the total cost of all selected MST edges.
    */
    int totalCost = 0;


    /*
        Keep track of the number of edges currently present in the MST.

        A spanning tree containing V vertices must contain exactly V-1 edges.
    */
    int mstEdgeCount = 0;


    /*
        Process the sorted edges from smallest weight to largest weight.

        This directly follows Kruskal's algorithm:
        pick the smallest remaining edge and check whether
        it creates a cycle with the edges selected so far.
    */
    cout << "\nKruskal's Algorithm Steps:\n";

    for (const Edge& edge : edges)
    {
        /*
            Stop processing when V-1 edges have already been
            selected because a spanning tree with V vertices
            contains exactly V-1 edges.
        */
        if (mstEdgeCount == V - 1)
        {
            break;
        }


        /*
            Find the representative of the component containing
            the source vertex.
        */
        int rootU = dsu.find(edge.u);


        /*
            Find the representative of the component containing
            the destination vertex.
        */
        int rootV = dsu.find(edge.v);


        /*
            Display the edge currently being considered.
        */
        cout << "\nConsidering edge "
             << vertexNames[edge.u] << " - "
             << vertexNames[edge.v]
             << " with weight " << edge.weight << ".\n";


        /*
            If both endpoints have the same representative,
            they already belong to the same connected component.

            Therefore, adding this edge would create a cycle.
        */
        if (rootU == rootV)
        {
            cout << "Cycle formed. Edge discarded.\n";
        }
        else
        {
            /*
                Since the endpoints belong to different components,
                adding this edge does not create a cycle.

                Therefore, include this edge in the MST.
            */
            mst.push_back(edge);


            /*
                Add the weight of the selected edge to the
                total cost of the MST.
            */
            totalCost += edge.weight;


            /*
                Increase the number of edges selected in the MST.
            */
            mstEdgeCount++;


            /*
                Merge the two components because the selected edge
                connects them.
            */
            dsu.unite(edge.u, edge.v);


            /*
                Display that the edge has been accepted.
            */
            cout << "No cycle formed. Edge included in MST.\n";
        }


        /*
            Display the current number of edges selected in the MST.
        */
        cout << "Edges selected so far: "
             << mstEdgeCount
             << " out of "
             << V - 1
             << ".\n";
    }


    /*
        If fewer than V-1 edges were selected, the graph is disconnected.

        A disconnected graph does not have a Minimum Spanning Tree.
        Kruskal's algorithm would instead produce a minimum spanning forest.
    */
    if (mstEdgeCount != V - 1)
    {
        cout << "\nThe graph is disconnected.\n";
        cout << "Minimum Spanning Tree does not exist.\n";

        return 0;
    }


    /*
        Display the final Minimum Spanning Tree.
    */
    cout << "\nMinimum Spanning Tree:\n";

    cout << "Edge\tWeight\n";

    for (const Edge& edge : mst)
    {
        cout << vertexNames[edge.u] << " - "
             << vertexNames[edge.v]
             << "\t"
             << edge.weight << "\n";
    }


    /*
        Display the total weight of all edges in the MST.
    */
    cout << "\nTotal cost of MST = " << totalCost << "\n";


    /*
        Time Complexity Analysis:

        Let V be the number of vertices and E be the number of edges.

        1. Reading V vertex labels and storing them in the map:
           O(V log V)

        2. Reading and converting E edge labels using the map:
           O(E log V)

        3. Sorting all edges in non-decreasing order of weight:
           O(E log E)

        4. Creating V separate sets in DSU:
           O(V)

        5. Processing the sorted edges:
           At most E edges are checked.
           Each edge requires Find and Union operations.

           With Path Compression and Union by Rank,
           each DSU operation takes O(alpha(V)) amortized time,
           where alpha(V) is the inverse Ackermann function.

           Therefore, processing all edges takes:
           O(E alpha(V))

        6. Printing the sorted edges:
           O(E)

        7. Printing the MST edges:
           O(V)

        Therefore, the total time complexity is:

           O(V log V) + O(E log V) + O(E log E)
           + O(V) + O(E alpha(V)) + O(E) + O(V)

        Since O(E log E) dominates the other terms
        for the graph processing performed by Kruskal's algorithm:

           Overall Time Complexity = O(E log E)


        Space Complexity:

        1. Storing all E edges:
           O(E)

        2. DSU parent and rank arrays for V vertices:
           O(V)

        3. Storing the MST containing V-1 edges:
           O(V)

        4. Storing vertex names and their mappings:
           O(V)

        Therefore:

           Overall Space Complexity = O(V + E)
    */


    return 0;
}
