#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

class SuffixTrie {
private:
    struct TrieNode {
        int id;
        int suffixStart;

        unordered_map<unsigned char, unique_ptr<TrieNode>> children;

        explicit TrieNode(int nodeId)
            : id(nodeId), suffixStart(-1) {}
    };

    unique_ptr<TrieNode> root;
    string text;
    int nextNodeId;

    static string displayChar(unsigned char ch) {
        switch (ch) {
            case '\n': return "\\n";
            case '\r': return "\\r";
            case '\t': return "\\t";
            case '\\': return "\\\\";
            case '\'': return "\\'";
            case ' ':  return "space";

            default:
                if (isprint(ch))
                    return string(1, static_cast<char>(ch));

                const char hex[] = "0123456789ABCDEF";
                string result = "\\x00";
                result[2] = hex[(ch >> 4) & 0x0F];
                result[3] = hex[ch & 0x0F];
                return result;
        }
    }

    vector<pair<unsigned char, const TrieNode*>>
    sortedChildren(const TrieNode* node) const {
        vector<pair<unsigned char, const TrieNode*>> result;

        for (const auto& entry : node->children)
            result.emplace_back(entry.first, entry.second.get());

        sort(result.begin(), result.end(),
             [](const auto& a, const auto& b) {
                 return a.first < b.first;
             });

        return result;
    }

public:
    explicit SuffixTrie(string input)
        : root(make_unique<TrieNode>(0)),
          text(move(input)),
          nextNodeId(1) {}

    /*
    METHOD 1: printSuffixes()

    Let n be the length of the input text, with n >= 1.

    TIME COMPLEXITY
    +---------+------------+-------------------------------------------+
    | Case    | Complexity | Reason                                    |
    +---------+------------+-------------------------------------------+
    | Best    | Theta(n^2) | All suffix characters are printed.        |
    +---------+------------+-------------------------------------------+
    | Average | Theta(n^2) | Total printed suffix length is            |
    |         |            | n*(n+1)/2 for every input.                |
    +---------+------------+-------------------------------------------+
    | Worst   | Theta(n^2) | Total printed suffix length is            |
    |         |            | n*(n+1)/2 for every input.                |
    +---------+------------+-------------------------------------------+

    AUXILIARY SPACE
    +---------+------------+-------------------------------------------+
    | Case    | Complexity | Reason                                    |
    +---------+------------+-------------------------------------------+
    | Best    | Theta(n)   | text.substr(0) copies all n characters,   |
    |         |            | so peak temporary storage is linear.      |
    +---------+------------+-------------------------------------------+
    | Average | Theta(n)   | text.substr(0) copies all n characters,   |
    |         |            | so peak temporary storage is linear.      |
    +---------+------------+-------------------------------------------+
    | Worst   | Theta(n)   | text.substr(0) copies all n characters,   |
    |         |            | so peak temporary storage is linear.      |
    +---------+------------+-------------------------------------------+

    The temporary suffix string is released after its use.
    Output storage is excluded from auxiliary-space analysis.
    For n = 0, the loop does not run.
    */

    void printSuffixes() const {
        cout << "\nAll suffixes of the input text:\n";
        cout << "------------------------------\n";

        for (size_t i = 0; i < text.size(); ++i) {
            cout << "Suffix " << i << ": "
                 << text.substr(i) << '\n';
        }

        if (text.empty())
            cout << "(Empty text has no non-empty suffixes.)\n";
    }

    /*
    METHOD 2: bruteForceSuffixTrie()

    Reference: Section 13.1.8, textbook pages 712-713.
    Suffixes are processed from right to left, as in the reference.

    Let n = text length.

    Assumption for runtime analysis:
    unordered_map child lookup and insertion take expected O(1).
    Each node has at most 256 distinct byte-valued child keys,
	so hash collisions do not change the asymptotic bound
	under the fixed-byte-alphabet assumption.

    TIME COMPLEXITY
    +---------+------------+-------------------------------------------+
    | Case    | Complexity | Reason                                    |
    +---------+------------+-------------------------------------------+
    | Best    | Theta(n^2) | Every suffix character is traversed,      |
    |         |            | even when its edge already exists.        |
    +---------+------------+-------------------------------------------+
    | Average | Theta(n^2) | Total suffix lengths equal                |
    |         |            | n*(n+1)/2 for every input.                |
    +---------+------------+-------------------------------------------+
    | Worst   | Theta(n^2) | Character processing totals               |
    |         |            | n*(n+1)/2; edge creation does not         |
    |         |            | change the asymptotic bound.              |
    +---------+------------+-------------------------------------------+

    SPACE COMPLEXITY: TRIE STORAGE
    +---------+------------+-------------------------------------------+
    | Case    | Complexity | Reason                                    |
    +---------+------------+-------------------------------------------+
    | Best    | Theta(n)   | Repetitive text such as "aaaa..." shares  |
    |         |            | a chain of n edges and n+1 nodes,         |
    |         |            | including the root.                       |
    +---------+------------+-------------------------------------------+
    | Average | Theta(n^2) | Expected value for independent,           |
    |         |            | uniformly random characters over a        |
    |         |            | fixed alphabet of at least two symbols.   |
    +---------+------------+-------------------------------------------+
    | Worst   | Theta(n^2) | At most 1+n*(n+1)/2 nodes are needed,     |
    |         |            | and quadratic growth is attainable,       |
    |         |            | e.g. a^(n/2)b^(n/2).                      |
    +---------+------------+-------------------------------------------+

    Assumptions:
    1. The alphabet consists of byte values 0-255.
    2. Average-case space assumes uniformly random characters over
       a fixed alphabet with at least two symbols.
    3. suffixStart marks suffix endpoints; no sentinel is appended.
    */

    void bruteForceSuffixTrie() {
        const size_t n = text.size();

        for (size_t i = n; i-- > 0;) {
            TrieNode* node = root.get();
            size_t j = i;

            // Follow the longest prefix already present in the trie.
            while (j < n) {
                unsigned char ch =
                    static_cast<unsigned char>(text[j]);

                auto it = node->children.find(ch);

                if (it == node->children.end())
                    break;

                node = it->second.get();
                ++j;
            }

            // Create nodes for the remaining characters of this suffix.
            for (size_t k = j; k < n; ++k) {
                unsigned char ch =
                    static_cast<unsigned char>(text[k]);

                auto newNode =
                    make_unique<TrieNode>(nextNodeId++);

                TrieNode* next = newNode.get();

                node->children.emplace(ch, move(newNode));
                node = next;
            }

            // Record the starting index of this complete suffix.
            // The reference algorithm does not store this; it is added
            // here so pattern searching can report occurrence positions.
            node->suffixStart = static_cast<int>(i);
        }
    }

    /*
    METHOD 3: searchOccurrences(pattern)

    Let:
        n    = text length
        m    = pattern length
        Vsub = number of nodes visited in the matching subtree
        k    = number of reported occurrences

    PATTERN-PATH TRAVERSAL ONLY (NON-EMPTY PATTERNS)
    +---------+------------------------+--------------------------------------+
    | Case    | Complexity             | Reason                               |
    +---------+------------------------+--------------------------------------+
    | Best    | Theta(1)               | The first required edge is absent.   |
    +---------+------------------------+--------------------------------------+
    | Average | Distribution-dependent | Depends on text and pattern          |
    |         |                        | distributions; no universal tight    |
    |         |                        | average bound is assumed.            |
    +---------+------------------------+--------------------------------------+
    | Worst   | Theta(min(m,n+1))      | At most min(m,n+1) edge lookups;     |
    |         |                        | attained by text a^n and pattern     |
    |         |                        | a^m.                                 |
    +---------+------------------------+--------------------------------------+

    FULL OCCURRENCE REPORTING
    Time: O(min(m,n+1) + Vsub + k log k).

    The algorithm follows the pattern path, traverses the matching
    subtree to collect suffix endpoints, and sorts the k positions.

    Worst-case time: Theta(n^2), because the matching subtree can
    contain Theta(n^2) nodes, e.g. text a^(n/2)b^(n/2) with pattern "a".

    AUXILIARY SPACE, excluding the trie (NON-EMPTY PATTERNS)
    +---------+------------+-------------------------------------------+
    | Case    | Complexity | Reason                                    |
    +---------+------------+-------------------------------------------+
    | Best    | Theta(1)   | A missing first edge returns an           |
    |         |            | empty vector immediately.                 |
    +---------+------------+-------------------------------------------+
    | Average | O(n)       | The DFS stack and occurrence vector       |
    |         |            | each need at most linear space; no        |
    |         |            | tighter average bound is assumed.         |
    +---------+------------+-------------------------------------------+
    | Worst   | Theta(n)   | The occurrence vector holds n entries     |
    |         |            | for text a^n and pattern "a"; the DFS     |
    |         |            | stack is O(n) for a fixed alphabet.       |
    +---------+------------+-------------------------------------------+

    Empty pattern:
    Time: Theta(n).
    Auxiliary space: Theta(n), because n+1 positions are returned.
    All boundaries from index 0 through n are included.
    */

    vector<size_t> searchOccurrences(const string& pattern) const {
        const size_t n = text.size();

        // An empty pattern matches all n+1 boundaries in the text.
        if (pattern.empty()) {
            vector<size_t> positions(n + 1);

            for (size_t i = 0; i <= n; ++i)
                positions[i] = i;

            return positions;
        }

        const TrieNode* node = root.get();

        // Follow the pattern path from the root.
        for (unsigned char ch : pattern) {
            auto it = node->children.find(ch);

            if (it == node->children.end())
                return {};

            node = it->second.get();
        }

        // Collect suffix endpoints below the matching pattern node.
        vector<size_t> positions;
        vector<const TrieNode*> stack{node};

        while (!stack.empty()) {
            const TrieNode* current = stack.back();
            stack.pop_back();

            if (current->suffixStart >= 0) {
                positions.push_back(
                    static_cast<size_t>(current->suffixStart)
                );
            }

            for (const auto& entry : current->children)
                stack.push_back(entry.second.get());
        }

        // Return occurrence positions in ascending order.
        sort(positions.begin(), positions.end());

        return positions;
    }

    /*
    METHOD 4: display()

    Let V be the total number of nodes and L be the total number
    of characters printed, including indentation, labels, IDs,
    and suffix-end markers.

    TIME COMPLEXITY
    +---------+----------------+-------------------------------------------+
    | Case    | Complexity     | Reason                                    |
    +---------+----------------+-------------------------------------------+
    | Best    | Theta(V + L)   | Every node is visited and printed once.   |
    |         |                | Sorting at most 256 children per node     |
    |         |                | costs O(1) per node.                      |
    +---------+----------------+-------------------------------------------+
    | Average | Theta(V + L)   | Every node is visited and printed once.   |
    |         |                | Sorting at most 256 children per node     |
    |         |                | costs O(1) per node.                      |
    +---------+----------------+-------------------------------------------+
    | Worst   | Theta(V + L)   | Every node is visited and printed once.   |
    |         |                | Sorting at most 256 children per node     |
    |         |                | costs O(1) per node.                      |
    +---------+----------------+-------------------------------------------+

    Every node prints at least one line, so L >= V.
    Therefore, Theta(V + L) is equivalent to Theta(L).

    Indentation grows with depth, so L can reach Theta(n^3)
    for texts such as a^(n/2)b^(n/2).

    For an alphabet of size sigma, sorting adds at most
    O(V * sigma log sigma), which is O(V) for the fixed byte alphabet.

    AUXILIARY SPACE
    O(n) for a fixed byte alphabet, as an upper bound over all inputs.
    The iterative DFS stack has O(n) levels and at most 256 child
    entries per active level.
    */

    void display() const {
        cout << "Root [node 0]\n";

        struct Frame {
            const TrieNode* node;
            vector<pair<unsigned char, const TrieNode*>> children;
            size_t nextChild;
            bool isLast;
        };

        vector<Frame> stack;
        stack.push_back({
            root.get(), sortedChildren(root.get()), 0, true
        });

        while (!stack.empty()) {
            Frame& frame = stack.back();

            if (frame.nextChild >= frame.children.size()) {
                stack.pop_back();
                continue;
            }

            size_t index = frame.nextChild++;
            bool childIsLast =
                (index + 1 == frame.children.size());

            auto edge = frame.children[index];
            const TrieNode* child = edge.second;

            for (size_t level = 1; level < stack.size(); ++level)
                cout << (stack[level].isLast ? "   " : "|  ");

            cout << (childIsLast ? "`-- " : "|-- ")
                 << "'" << displayChar(edge.first) << "'"
                 << " -> node " << child->id;

            if (child->suffixStart >= 0) {
                cout << " [END: suffix starts at index "
                     << child->suffixStart << "]";
            }

            cout << '\n';

            stack.push_back({
                child, sortedChildren(child), 0, childIsLast
            });
        }
    }

    /*
    METHOD 5: nodeCount()

    Let V be the number of nodes and n the text length.

    TIME COMPLEXITY
    +---------+------------+-------------------------------------------+
    | Case    | Complexity | Reason                                    |
    +---------+------------+-------------------------------------------+
    | Best    | Theta(V)   | Every node is visited exactly once.       |
    +---------+------------+-------------------------------------------+
    | Average | Theta(V)   | Every node is visited exactly once.       |
    +---------+------------+-------------------------------------------+
    | Worst   | Theta(V)   | Every node is visited exactly once.       |
    +---------+------------+-------------------------------------------+

    AUXILIARY SPACE
    O(n) for a fixed alphabet, as an upper bound over all inputs,
    because the iterative DFS stack contains at most O(n) node
    pointers.
    */

    size_t nodeCount() const {
        size_t count = 0;
        vector<const TrieNode*> stack{root.get()};

        while (!stack.empty()) {
            const TrieNode* node = stack.back();
            stack.pop_back();
            ++count;

            for (const auto& entry : node->children)
                stack.push_back(entry.second.get());
        }

        return count;
    }
};

int main() {
    string text;

    cout << "Enter text: ";

    if (!getline(cin, text)) {
        cerr << "Error: unable to read text.\n";
        return 1;
    }

    SuffixTrie trie(text);

    // Step 1: Print every suffix separately.
    trie.printSuffixes();

    // Step 2: Construct the suffix trie.
    trie.bruteForceSuffixTrie();

    // Step 3: Display the constructed trie.
    cout << "\nSuffix Trie (one character per edge)\n";
    cout << "------------------------------------\n";
    trie.display();

    cout << "\nText length: " << text.size() << '\n';
    cout << "Total nodes (including root): "
         << trie.nodeCount() << '\n';

    // Step 4: Accept a pattern and search for its occurrences.
    string pattern;

    cout << "\nEnter pattern to search: ";

    if (!getline(cin, pattern)) {
        cerr << "Error: unable to read pattern.\n";
        return 1;
    }

    vector<size_t> positions = trie.searchOccurrences(pattern);

    // Step 5: Display the search result.
    if (positions.empty()) {
        cout << "Pattern not found.\n";
    } else {
        cout << "Pattern found at index/indices (0-based): ";

        for (size_t i = 0; i < positions.size(); ++i) {
            if (i > 0)
                cout << ", ";

            cout << positions[i];
        }

        cout << "\nTotal occurrences: " << positions.size() << '\n';
    }

    return 0;
}
