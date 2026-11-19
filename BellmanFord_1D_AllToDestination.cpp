#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <new>
#include <stdexcept>

using namespace std;

/*
    Bellman-Ford Algorithm using a One-Dimensional Array
    to find shortest paths from every vertex to a given destination.

    dist[v] stores the shortest distance from vertex v
    to the specified destination in the original graph.

    nextEdge[v] stores the index of the exact original edge
    to follow from vertex v towards the destination.

    INF represents a vertex for which no path to the destination
    has been found. After the algorithm terminates, such a vertex
    cannot reach the specified destination.

    The algorithm relaxes every original edge in the reverse direction
    (equivalent to running Bellman-Ford on the reversed graph).

    For an original edge u -> v with weight w:

    dist[u] = min(dist[u], w + dist[v])

    Time Complexity:
    Best Case    : O(V + E), including initialization
                   O(E), ignoring O(V) initialization
    Average Case : O(VE) in general
    Worst Case   : O(VE) for Bellman-Ford

    The complete program has O(VE + V^2) worst-case time complexity
    because it prints the shortest paths for all vertices.

    Space Complexity:
    Graph Storage  : O(V + E)
    Distance Array : O(V)
    Next Edge Array: O(V)
    Path Storage   : O(V)

    Total Auxiliary Space: O(V)
    Total Space: O(V + E)

    Numeric Range:
    INF = LLONG_MAX / 4 represents an unreachable vertex.
    Finite distances must remain strictly between -INF and INF.
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
    Returns -1 if the label does not exist.
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
    Adds two values after checking the supported distance range.

    Both operands lie strictly between -INF and INF.
    Since 2 * INF is smaller than LLONG_MAX, their sum
    cannot overflow long long.

    Returns false if the result falls outside the supported range.
*/
bool safeAdd(long long a, long long b, long long& result)
{
    long long sum = a + b;

    if (sum <= -INF || sum >= INF)
    {
        return false;
    }

    result = sum;
    return true;
}

/*
    Prints a path from a starting vertex to the destination.
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
    Reconstructs the original-graph path by following nextEdge
    forwards from the starting vertex until the destination
    is reached.

    Each stored edge identifies the next vertex towards
    the specified destination.
*/
vector<int> reconstructPath(
    int start,
    int destination,
    const vector<Edge>& edges,
    const vector<int>& nextEdge,
    int V)
{
    vector<int> path;
    int current = start;

    for (int count = 0; count < V; ++count)
    {
        path.push_back(current);

        if (current == destination)
        {
            return path;
        }

        int edgeIndex = nextEdge[current];

        if (edgeIndex < 0 ||
            edgeIndex >= static_cast<int>(edges.size()))
        {
            return {};
        }

        const Edge& edge = edges[edgeIndex];

        if (edge.source != current)
        {
            return {};
        }

        current = edge.destination;
    }

    return {};
}

/*
    Prints the exact selected edges and cumulative path cost.
    Stored edge indices handle parallel edges correctly.
*/
void printPathCalculation(
    const vector<int>& path,
    const vector<Edge>& edges,
    const vector<int>& nextEdge,
    const vector<string>& vertices)
{
    if (path.size() == 1)
    {
        cout << "The starting vertex is the destination vertex.\n";
        cout << "Total path cost: 0\n";
        return;
    }

    long long totalCost = 0;

    cout << "Path calculation:\n";

    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        int u = path[i];
        int v = path[i + 1];
        int edgeIndex = nextEdge[u];

        if (edgeIndex < 0 ||
            edgeIndex >= static_cast<int>(edges.size()))
        {
            cout << "The shortest path could not be verified.\n";
            return;
        }

        const Edge& edge = edges[edgeIndex];

        if (edge.source != u || edge.destination != v)
        {
            cout << "The shortest path could not be verified.\n";
            return;
        }

        long long updatedCost;

        if (!safeAdd(totalCost, edge.weight, updatedCost))
        {
            cout << "Path cost exceeds the supported numeric range.\n";
            return;
        }

        totalCost = updatedCost;

        cout << vertices[u] << " to " << vertices[v]
             << " with edge weight " << edge.weight
             << ", cumulative cost: " << totalCost << '\n';
    }

    cout << "Total path cost: " << totalCost << '\n';
}

int main()
{
    try
    {
        int V, E;

        cout << "Enter the number of vertices: ";

        if (!(cin >> V) || V <= 0)
        {
            cerr << "The number of vertices must be a positive integer.\n";
            return 1;
        }

        cout << "Enter the number of edges: ";

        if (!(cin >> E) || E < 0)
        {
            cerr << "The number of edges must be a non-negative integer.\n";
            return 1;
        }

        vector<string> vertices(V);
        unordered_map<string, int> vertexIndex;

        cout << "\nEnter the vertex labels.\n";
        cout << "Labels can be numbers, letters, or words.\n";

        for (int i = 0; i < V; ++i)
        {
            cout << "Vertex " << i + 1 << ": ";

            if (!(cin >> vertices[i]))
            {
                cerr << "Invalid vertex input.\n";
                return 1;
            }

            if (vertexIndex.find(vertices[i]) != vertexIndex.end())
            {
                cerr << "Vertex labels must be unique.\n";
                return 1;
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
            string sourceLabel, destinationLabel;
            long long weight;

            cout << "Edge " << i + 1 << ": ";

            if (!(cin >> sourceLabel >> destinationLabel >> weight))
            {
                cerr << "Invalid edge input.\n";
                return 1;
            }

            int source = getVertexIndex(sourceLabel, vertexIndex);
            int destination =
                getVertexIndex(destinationLabel, vertexIndex);

            if (source == -1 || destination == -1)
            {
                cerr << "Both edge endpoints must be valid vertex labels.\n";
                return 1;
            }

            if (weight <= -INF || weight >= INF)
            {
                cerr << "Edge weight is outside the supported numeric range.\n";
                return 1;
            }

            edges.push_back({source, destination, weight});
        }

        string destinationLabel;

        cout << "\nEnter the destination vertex: ";

        if (!(cin >> destinationLabel))
        {
            cerr << "Invalid destination input.\n";
            return 1;
        }

        int destination =
            getVertexIndex(destinationLabel, vertexIndex);

        if (destination == -1)
        {
            cerr << "The destination vertex does not exist.\n";
            return 1;
        }

        /*
            Initialize all distances to infinity.
            The destination has distance zero from itself.
        */
        vector<long long> dist(V, INF);
        vector<int> nextEdge(V, -1);

        dist[destination] = 0;

        /*
            Relax each original edge in the reverse direction.

            For an original edge u -> v with weight w,
            if v can reach the destination, test whether
            w + dist[v] improves dist[u].

            Record the exact edge index whenever a shorter
            distance is found.
        */
        for (int i = 1; i <= V - 1; ++i)
        {
            bool updated = false;

            for (int j = 0; j < static_cast<int>(edges.size()); ++j)
            {
                const Edge& edge = edges[j];

                int u = edge.source;
                int v = edge.destination;

                // v cannot reach the destination, so it cannot extend a path.
                if (dist[v] == INF)
                {
                    continue;
                }

                long long candidate;

                if (!safeAdd(dist[v], edge.weight, candidate))
                {
                    cerr << "A path distance exceeds the supported "
                         << "numeric range. Shortest paths cannot "
                         << "be determined reliably.\n";
                    return 1;
                }

                if (candidate < dist[u])
                {
                    dist[u] = candidate;
                    nextEdge[u] = j;
                    updated = true;
                }
            }

            // Stop when a complete pass makes no distance changes.
            if (!updated)
            {
                break;
            }
        }

        /*
            Check for a negative-weight cycle that can reach
            the specified destination in the original graph.

            Such a cycle makes shortest paths undefined for
            vertices that can reach the cycle and then the destination.
        */
        for (const Edge& edge : edges)
        {
            int u = edge.source;
            int v = edge.destination;

            if (dist[v] == INF)
            {
                continue;
            }

            long long candidate;

            if (!safeAdd(dist[v], edge.weight, candidate))
            {
                cerr << "Numeric range exceeded during negative-cycle "
                     << "detection. The result cannot be determined safely.\n";
                return 1;
            }

            if (candidate < dist[u])
            {
                cout << "\nA negative-weight cycle that can reach the "
                     << "destination was detected.\n";
                cout << "Shortest paths are undefined for vertices "
                     << "affected by this cycle.\n";
                return 2;
            }
        }

        cout << "\nShortest Paths to Destination Vertex "
             << destinationLabel << '\n';

        for (int start = 0; start < V; ++start)
        {
            cout << "\nStarting Vertex: " << vertices[start] << '\n';

            if (dist[start] == INF)
            {
                cout << "No path exists from " << vertices[start]
                     << " to " << destinationLabel << ".\n";
                cout << "Shortest distance: Unreachable\n";
                continue;
            }

            vector<int> path = reconstructPath(
                start,
                destination,
                edges,
                nextEdge,
                V);

            if (path.empty())
            {
                cout << "The shortest path could not be reconstructed.\n";
                continue;
            }

            cout << "Shortest path: ";
            printPath(path, vertices);

            cout << "Shortest distance: " << dist[start] << '\n';

            printPathCalculation(
                path,
                edges,
                nextEdge,
                vertices);
        }

        cout << "\nFinal Shortest Distances to Destination\n";

        cout << left
             << setw(20) << "Vertex"
             << setw(20) << "Distance" << '\n';

        for (int v = 0; v < V; ++v)
        {
            cout << left << setw(20) << vertices[v];

            if (dist[v] == INF)
            {
                cout << setw(20) << "Unreachable";
            }
            else
            {
                cout << setw(20) << dist[v];
            }

            cout << '\n';
        }

        return 0;
    }
    catch (const bad_alloc&)
    {
        cerr << "Insufficient memory to process the graph.\n";
        return 1;
    }
    catch (const length_error&)
    {
        cerr << "The requested graph size exceeds supported limits.\n";
        return 1;
    }
}
