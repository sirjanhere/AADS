#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

/*
    Bellman-Ford Algorithm using Dynamic Programming with a 2D array.

    dp[k][v] represents the shortest distance from the source vertex to vertex v using at most k edges.

    Recurrence:
    dp[k][v] = min(dp[k-1][v],
                   dp[k-1][u] + weight(u, v))

    for every edge (u, v).

    Time Complexity:
    Best Case    : O(VE)
    Average Case : O(VE)
    Worst Case   : O(VE)

    This implementation performs V-1 iterations and examines all E edges in every iteration, so all three cases have O(VE) time.

    Space Complexity:
    Graph Storage : O(V + E)
    2D DP Array   : O(V^2)

    Total Space:
    O(V + E) + O(V^2)
    = O(V^2 + E)
*/

struct Edge
{
    int source;
    int destination;
    long long weight;
};

const long long INF = numeric_limits<long long>::max() / 4;

/*
    Returns the internal index of a vertex label.
*/
int getVertexIndex(
    const string& label,
    const unordered_map<string, int>& vertexIndex)
{
    auto it = vertexIndex.find(label);

    if (it == vertexIndex.end())
    {
        return -1;
    }

    return it->second;
}

/*
    Prints the complete 2D dynamic programming table.
*/
void printDPTable(
    const vector<vector<long long>>& dp,
    const vector<string>& vertices)
{
    cout << "\nDynamic Programming Table\n";
    cout << "Each row shows the shortest distance using at most the given number of edges.\n\n";

    cout << left << setw(18) << "Edges Allowed";

    for (const string& vertex : vertices)
    {
        cout << setw(18) << vertex;
    }

    cout << '\n';

    for (int k = 0; k < static_cast<int>(dp.size()); ++k)
    {
        cout << left << setw(18) << k;

        for (int v = 0; v < static_cast<int>(vertices.size()); ++v)
        {
            if (dp[k][v] == INF)
            {
                cout << setw(18) << "Unreachable";
            }
            else
            {
                cout << setw(18) << dp[k][v];
            }
        }

        cout << '\n';
    }
}

/*
    Finds a predecessor of the current vertex by comparing consecutive rows of the DP table.

    If dp[k][current] is unchanged from dp[k-1][current], no edge was required at this step, so reconstruction continues with k-1.

    Otherwise, an incoming edge that produces dp[k][current] is used as the previous edge of the shortest path.
*/
bool findPredecessor(
    int current,
    int k,
    const vector<vector<long long>>& dp,
    const vector<Edge>& edges,
    int& previousVertex,
    long long& selectedWeight)
{
    if (dp[k][current] == dp[k - 1][current])
    {
        previousVertex = -1;
        selectedWeight = 0;
        return true;
    }

    for (const Edge& edge : edges)
    {
        if (edge.destination != current)
        {
            continue;
        }

        if (dp[k - 1][edge.source] == INF)
        {
            continue;
        }

        long long candidate =
            dp[k - 1][edge.source] + edge.weight;

        if (candidate == dp[k][current])
        {
            previousVertex = edge.source;
            selectedWeight = edge.weight;
            return true;
        }
    }

    return false;
}

/*
    Reconstructs the shortest path from the final DP row.

    The reconstruction starts from the destination and moves backwards through the DP table until the source vertex is reached.
*/
vector<int> reconstructPath(
    int source,
    int destination,
    const vector<vector<long long>>& dp,
    const vector<Edge>& edges,
    vector<long long>& pathWeights)
{
    vector<int> reversedPath;

    int current = destination;

    reversedPath.push_back(current);

    for (int k = static_cast<int>(dp.size()) - 1; k > 0; --k)
    {
        int previousVertex;
        long long selectedWeight;

        bool found =
            findPredecessor(
                current,
                k,
                dp,
                edges,
                previousVertex,
                selectedWeight);

        if (!found)
        {
            return {};
        }

        if (previousVertex == -1)
        {
            continue;
        }

        pathWeights.push_back(selectedWeight);

        current = previousVertex;
        reversedPath.push_back(current);

        if (current == source)
        {
            break;
        }
    }

    if (current != source)
    {
        return {};
    }

    reverse(reversedPath.begin(), reversedPath.end());
    reverse(pathWeights.begin(), pathWeights.end());

    return reversedPath;
}

/*
    Prints the shortest path in forward order.
*/
void printPath(
    const vector<int>& path,
    const vector<string>& vertices)
{
    for (size_t i = 0; i < path.size(); ++i)
    {
        cout << vertices[path[i]];

        if (i + 1 < path.size())
        {
            cout << " -> ";
        }
    }

    cout << '\n';
}

/*
    Prints the step-by-step cost calculation for a shortest path.
*/
void printPathCalculation(
    const vector<int>& path,
    const vector<long long>& pathWeights,
    const vector<string>& vertices)
{
    if (path.size() == 1)
    {
        cout << "The source vertex is the destination vertex.\n";
        cout << "Path cost: 0\n";
        return;
    }

    long long totalCost = 0;

    cout << "Path calculation:\n";

    for (size_t i = 0; i < pathWeights.size(); ++i)
    {
        totalCost += pathWeights[i];

        cout << vertices[path[i]]
             << " to "
             << vertices[path[i + 1]]
             << " with edge weight "
             << pathWeights[i];

        cout << ", cumulative cost: "
             << totalCost
             << '\n';
    }

    cout << "Total path cost: "
         << totalCost
         << '\n';
}

int main()
{
    int V, E;

    cout << "Enter the number of vertices: ";
    cin >> V;

    if (V <= 0)
    {
        cout << "The number of vertices must be positive.\n";
        return 0;
    }

    cout << "Enter the number of edges: ";
    cin >> E;

    if (E < 0)
    {
        cout << "The number of edges cannot be negative.\n";
        return 0;
    }

    vector<string> vertices(V);
    unordered_map<string, int> vertexIndex;

    cout << "\nEnter the vertex labels.\n";
    cout << "Labels can be numbers, letters, or words.\n";

    for (int i = 0; i < V; ++i)
    {
        cout << "Vertex " << i + 1 << ": ";
        cin >> vertices[i];

        if (vertexIndex.find(vertices[i]) != vertexIndex.end())
        {
            cout << "Vertex labels must be unique.\n";
            return 0;
        }

        vertexIndex[vertices[i]] = i;
    }

    vector<Edge> edges;
    edges.reserve(E);

    cout << "\nEnter each directed edge as:\n";
    cout << "Source Destination Weight\n";
    cout << "Negative edge weights are allowed.\n\n";

    for (int i = 0; i < E; ++i)
    {
        string sourceLabel;
        string destinationLabel;
        long long weight;

        cout << "Edge " << i + 1 << ": ";
        cin >> sourceLabel >> destinationLabel >> weight;

        int source =
            getVertexIndex(sourceLabel, vertexIndex);

        int destination =
            getVertexIndex(destinationLabel, vertexIndex);

        if (source == -1 || destination == -1)
        {
            cout << "Both edge endpoints must be valid vertex labels.\n";
            --i;
            continue;
        }

        edges.push_back(
            {source, destination, weight});
    }

    string sourceLabel;

    cout << "\nEnter the source vertex: ";
    cin >> sourceLabel;

    int source =
        getVertexIndex(sourceLabel, vertexIndex);

    if (source == -1)
    {
        cout << "The source vertex does not exist.\n";
        return 0;
    }

    /*
        dp[k][v] stores the shortest distance from the source to v using at most k edges.

        There are V rows because k ranges from 0 to V-1.
    */
    vector<vector<long long>> dp(
        V,
        vector<long long>(V, INF));

    dp[0][source] = 0;

    /*
        Compute the DP table for paths containing at most 1, 2, ..., V-1 edges.
    */
    for (int k = 1; k <= V - 1; ++k)
    {
        /*
            Carry forward the distances from the previous row. This represents paths that use fewer than k edges.
        */
        for (int v = 0; v < V; ++v)
        {
            dp[k][v] = dp[k - 1][v];
        }

        /*
            Try every edge as the final edge of a path.
        */
        for (const Edge& edge : edges)
        {
            if (dp[k - 1][edge.source] == INF)
            {
                continue;
            }

            long long candidate =
                dp[k - 1][edge.source] + edge.weight;

            if (candidate < dp[k][edge.destination])
            {
                dp[k][edge.destination] = candidate;
            }
        }
    }

    printDPTable(dp, vertices);

    /*
        Check whether one more relaxation can improve a reachable distance. Such an improvement means that a negative-weight cycle is reachable from the source.
    */
    bool negativeCycle = false;

    for (const Edge& edge : edges)
    {
        if (dp[V - 1][edge.source] == INF)
        {
            continue;
        }

        long long candidate =
            dp[V - 1][edge.source] + edge.weight;

        if (candidate < dp[V - 1][edge.destination])
        {
            negativeCycle = true;
            break;
        }
    }

    if (negativeCycle)
    {
        cout << "\nA reachable negative-weight cycle was detected.\n";
        cout << "Shortest paths cannot be determined for vertices affected by this cycle.\n";

        return 0;
    }

    cout << "\nShortest Paths from Source Vertex "
         << sourceLabel
         << '\n';

    for (int destination = 0; destination < V; ++destination)
    {
        cout << "\nDestination Vertex: "
             << vertices[destination]
             << '\n';

        if (dp[V - 1][destination] == INF)
        {
            cout << "No path exists from "
                 << sourceLabel
                 << " to "
                 << vertices[destination]
                 << ".\n";

            continue;
        }

        vector<long long> pathWeights;

        vector<int> path =
            reconstructPath(
                source,
                destination,
                dp,
                edges,
                pathWeights);

        if (path.empty())
        {
            cout << "The shortest path could not be reconstructed.\n";
            continue;
        }

        cout << "Shortest path: ";
        printPath(path, vertices);

        cout << "Shortest distance: "
             << dp[V - 1][destination]
             << '\n';

        printPathCalculation(
            path,
            pathWeights,
            vertices);
    }

    cout << "\nFinal Shortest Distances\n";

    cout << left
         << setw(20)
         << "Vertex"
         << setw(20)
         << "Distance"
         << '\n';

    for (int v = 0; v < V; ++v)
    {
        cout << left
             << setw(20)
             << vertices[v];

        if (dp[V - 1][v] == INF)
        {
            cout << setw(20)
                 << "Unreachable";
        }
        else
        {
            cout << setw(20)
                 << dp[V - 1][v];
        }

        cout << '\n';
    }

    return 0;
}
