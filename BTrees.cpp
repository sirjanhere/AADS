//  B-TREE (order m)  --  implemented as described in Drozdek, Chapter 7.1.1
//
//  Book definition (order m = maximum number of children):
//    1. Root has at least 2 subtrees unless it is a leaf.
//    2. Each non-root, non-leaf node holds k-1 keys and k pointers, ceil(m/2) <= k <= m.
//    3. Each leaf holds k-1 keys, ceil(m/2) <= k <= m.
//    4. All leaves are on the same level.
//
//  NOTATION USED IN THE COMPLEXITY COMMENTS
//    n = number of keys in the tree      m = order (max children per node), m >= 3
//    t = ceil(m/2) (min children)        h = height = number of levels (root = level 1)
//    N = number of nodes                 k = number of keys in the output
//    w = maximum number of nodes on one level
//
//    Comparing / copying one key is counted as O(1).
//    Every node holds at least 1 key (m >= 3), so N <= n.
//    Worst-case height (book):  h <= log_t( (n+1)/2 ) + 1, hence h = O(log_t n)
//    (for n >= 2; a tree with one node has h = 1).
//    "Disk accesses" = number of nodes read/written (model: one node = one disk
//    block; this program itself keeps everything in RAM). "CPU time" counts key
//    comparisons; inside one node the book scans keys linearly, so one node
//    costs O(m) CPU. (Binary search inside a node would make it O(log m).)
//    "Space" means auxiliary space, not counting the tree itself. A std::vector
//    that reallocates while a key/pointer is inserted needs O(m) temporary space.
//
//  Key type T must be default-constructible, copyable, support operator<
//  and operator<< (the menu program uses BTree<int>).
//
//  Compile :  g++ -std=c++11 -Wall -o btree btree.cpp
#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <sstream>
#include <iomanip>
#include <string>
using namespace std;

template <class T>
class BTree {
private:
    // Node: same idea as the book's BTreeNode (leaf flag, keyTally, keys,
    // pointers). vectors are used so that the order m can be typed at run
    // time. keyTally() is simply keys.size().
    struct Node {
        bool leaf;
        vector<T> keys;          // at most m-1 keys (m during a temporary overflow)
        vector<Node*> pointers;  // keys.size()+1 children if not a leaf, else empty
        Node(bool isLeaf) : leaf(isLeaf) {}
        int keyTally() const { return (int)keys.size(); }
    };
    typedef pair<Node*, int> PathItem;   // (parent node, index of child we went to)

    int  m;          // order
    int  maxKeys;    // m - 1
    int  minKeys;    // ceil(m/2) - 1  (minimum keys of a non-root node)
    Node* root;
    bool verbose;    // print which insertion/deletion case happens

    void trace(const string& s) const { if (verbose) cout << "   [" << s << "]\n"; }

    // findPos: first index i such that keys[i] >= K  (linear scan like book)
    // Time: O(m) CPU, 0 disk accesses (node already in memory)
    int findPos(const Node* node, const T& K) const {
        int i = 0;
        while (i < node->keyTally() && node->keys[i] < K) i++;
        return i;
    }
    // isEqualAt: true if keys[i] exists and equals K (K is already known to be <= keys[i]).
    // Time: O(1), Space: O(1)
    bool isEqualAt(const Node* node, int i, const T& K) const {
        return i < node->keyTally() && !(K < node->keys[i]);   // keys[i] >= K and !(K<keys[i]) => equal
    }

    // BTreeSearch (book's recursive algorithm)
    // Returns the node containing K and sets idx to K's position, else NULL.
    // Time: O(h*m) CPU (linear scan in each of <= h nodes)
    //       O(h) = O(log_t n) disk accesses.
    //       (with binary search in a node: O(h*log m) CPU)
    // Space: O(h) recursion stack.
    Node* BTreeSearch(const T& K, Node* node, int& idx) const {
        if (node == NULL) return NULL;
        int i = findPos(node, K);
        if (isEqualAt(node, i, K)) { idx = i; return node; }
        if (node->leaf) return NULL;
        return BTreeSearch(K, node->pointers[i], idx);
    }

    // splitNode: splits 'node' around keys[mid]. The left part stays in node,
    // the right part goes to a NEW node which is returned; keys[mid] is
    // returned in 'up' (it is the key promoted to the parent).
    // Time: O(m) (copying about m/2 keys and pointers). Space: O(1) auxiliary
    // (the new node itself occupies O(m) and becomes part of the tree).
    Node* splitNode(Node* node, int mid, T& up) {
        Node* right = new Node(node->leaf);
        up = node->keys[mid];
        right->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
        node->keys.erase(node->keys.begin() + mid, node->keys.end());
        if (!node->leaf) {
            right->pointers.assign(node->pointers.begin() + mid + 1, node->pointers.end());
            node->pointers.erase(node->pointers.begin() + mid + 1, node->pointers.end());
        }
        return right;
    }

    // splitChild (used only by presplitting): splits the FULL child
    // parent->pointers[i] and inserts the promoted key into parent.
    // Time: O(m) (splitNode O(m) + two vector inserts into parent O(m)).
    // Space: O(m) temporary: an insert into the parent's vectors may reallocate them.
    void splitChild(Node* parent, int i) {
        Node* child = parent->pointers[i];
        int mid = (child->keyTally() - 1) / 2;          // child has m-1 keys
        T up;
        Node* right = splitNode(child, mid, up);
        parent->keys.insert(parent->keys.begin() + i, up);
        parent->pointers.insert(parent->pointers.begin() + i + 1, right);
    }

    // redistribute (book case 1.2.1): gather keys of the two neighbouring
    // children li and li+1 plus the separator parent->keys[li], then divide
    // them evenly; the middle key becomes the new separator in the parent.
    // Time: O(m) (at most 2(m-1)+1 keys and 2m pointers are copied).
    // Space: O(m) for the temporary vectors allK and allP.
    void redistribute(Node* parent, int li) {
        Node* L = parent->pointers[li];
        Node* R = parent->pointers[li + 1];
        vector<T> allK(L->keys);
        allK.push_back(parent->keys[li]);
        allK.insert(allK.end(), R->keys.begin(), R->keys.end());
        vector<Node*> allP;
        if (!L->leaf) {
            allP = L->pointers;
            allP.insert(allP.end(), R->pointers.begin(), R->pointers.end());
        }
        int total = (int)allK.size();
        int mid = (total - 1) / 2;
        L->keys.assign(allK.begin(), allK.begin() + mid);
        parent->keys[li] = allK[mid];
        R->keys.assign(allK.begin() + mid + 1, allK.end());
        if (!L->leaf) {
            L->pointers.assign(allP.begin(), allP.begin() + mid + 1);
            R->pointers.assign(allP.begin() + mid + 1, allP.end());
        }
    }

    // mergeNodes (book case 1.2.2): children li and li+1 and the separator
    // parent->keys[li] are put into child li; child li+1 is discarded and the
    // parent loses one key and one pointer (parent may underflow afterwards).
    // Time: O(m) (copy of R into L, erase from parent's vectors).
    // Space: O(m) temporary: inserting into L's vectors may reallocate them.
    void mergeNodes(Node* parent, int li) {
        Node* L = parent->pointers[li];
        Node* R = parent->pointers[li + 1];
        L->keys.push_back(parent->keys[li]);
        L->keys.insert(L->keys.end(), R->keys.begin(), R->keys.end());
        if (!L->leaf)
            L->pointers.insert(L->pointers.end(), R->pointers.begin(), R->pointers.end());
        parent->keys.erase(parent->keys.begin() + li);
        parent->pointers.erase(parent->pointers.begin() + li + 1);
        delete R;                       // shallow delete: its children now belong to L
    }

    // recursive helpers (their complexity is documented at the public wrappers)
    // destroy: frees every node of a subtree. Time O(n), Space O(h) (see ~BTree).
    void destroy(Node* n) {
        if (!n) return;
        if (!n->leaf)
            for (size_t i = 0; i < n->pointers.size(); i++) destroy(n->pointers[i]);
        delete n;
    }
    void inorderRec(Node* n) const {
        if (!n) return;
        for (int i = 0; i < n->keyTally(); i++) {
            if (!n->leaf) inorderRec(n->pointers[i]);
            cout << n->keys[i] << " ";
        }
        if (!n->leaf) inorderRec(n->pointers[n->keyTally()]);
    }
    void printRec(Node* n, int depth) const {
        if (!n) return;
        cout << string(depth * 4, ' ') << (n->leaf ? "(leaf) " : "(node) ") << keysStr(n) << "\n";
        if (!n->leaf)
            for (size_t i = 0; i < n->pointers.size(); i++) printRec(n->pointers[i], depth + 1);
    }
    // keysStr: text like "[3 8 22]". Time O(m), Space O(m) (the returned string).
    static string keysStr(const Node* n) {
        ostringstream os;
        os << "[";
        for (int i = 0; i < n->keyTally(); i++) os << (i ? " " : "") << n->keys[i];
        os << "]";
        return os.str();
    }
    void rangeRec(Node* n, const T& low, const T& high, vector<T>& out) const {
        if (!n) return;
        for (int i = 0; i < n->keyTally(); i++) {
            if (!n->leaf && !(n->keys[i] < low)) rangeRec(n->pointers[i], low, high, out);
            if (!(n->keys[i] < low) && !(high < n->keys[i])) out.push_back(n->keys[i]);
            if (high < n->keys[i]) return;          // everything further right is too big
        }
        if (!n->leaf) rangeRec(n->pointers[n->keyTally()], low, high, out);
    }
    int countKeysRec(Node* n) const {
        if (!n) return 0;
        int c = n->keyTally();
        if (!n->leaf)
            for (size_t i = 0; i < n->pointers.size(); i++) c += countKeysRec(n->pointers[i]);
        return c;
    }
    int countNodesRec(Node* n) const {
        if (!n) return 0;
        int c = 1;
        if (!n->leaf)
            for (size_t i = 0; i < n->pointers.size(); i++) c += countNodesRec(n->pointers[i]);
        return c;
    }
    int countLeavesRec(Node* n) const {
        if (!n) return 0;
        if (n->leaf) return 1;
        int c = 0;
        for (size_t i = 0; i < n->pointers.size(); i++) c += countLeavesRec(n->pointers[i]);
        return c;
    }
    bool validateRec(Node* n, const T* lo, const T* hi, int depth, int& leafDepth,
                     bool isRoot, string& err) const {
        int k = n->keyTally();
        if (k > maxKeys)                    { err = "node has more than m-1 keys"; return false; }
        if (!isRoot && k < minKeys)         { err = "non-root node has fewer than ceil(m/2)-1 keys"; return false; }
        if (isRoot && k < 1)                { err = "root is empty"; return false; }
        for (int i = 0; i < k; i++) {
            if (i > 0 && !(n->keys[i - 1] < n->keys[i])) { err = "keys not strictly increasing"; return false; }
            if (lo && !(*lo < n->keys[i]))               { err = "key violates lower bound from ancestor"; return false; }
            if (hi && !(n->keys[i] < *hi))               { err = "key violates upper bound from ancestor"; return false; }
        }
        if (n->leaf) {
            if (!n->pointers.empty()) { err = "leaf has pointers"; return false; }
            if (leafDepth == -1) leafDepth = depth;
            else if (leafDepth != depth) { err = "leaves are not all on the same level"; return false; }
            return true;
        }
        if ((int)n->pointers.size() != k + 1) { err = "non-leaf must have keys+1 pointers"; return false; }
        if (isRoot && n->pointers.size() < 2) { err = "root has fewer than 2 subtrees"; return false; }
        for (int i = 0; i <= k; i++) {
            const T* l = (i == 0) ? lo : &n->keys[i - 1];
            const T* h2 = (i == k) ? hi : &n->keys[i];
            if (!n->pointers[i]) { err = "NULL child in non-leaf"; return false; }
            if (!validateRec(n->pointers[i], l, h2, depth + 1, leafDepth, false, err)) return false;
        }
        return true;
    }

public:
    // Constructor / destructor
    // Constructor: Time O(1), Space O(1).
    // Destructor / clear: Time O(n) (each of the N <= n nodes is freed once and
    // every key / pointer slot is touched once), Space O(h) recursion.
    BTree(int order, bool verboseTrace = true) : root(NULL), verbose(verboseTrace) {
        m = (order < 3) ? 3 : order;           // a B-tree needs order >= 3
        maxKeys = m - 1;
        minKeys = (m + 1) / 2 - 1;             // ceil(m/2) - 1
    }
    ~BTree() { clear(); }

    // Copying is disabled: the tree owns its nodes through raw pointers, so a
    // shallow copy would make two objects free the same nodes (double free).
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;
    void clear() { destroy(root); root = NULL; }
    // setVerbose, order: Time O(1), Space O(1)
    void setVerbose(bool v) { verbose = v; }
    int  order() const { return m; }

    // isEmpty: Time O(1), Space O(1)
    bool isEmpty() const { return root == NULL; }

    //  SEARCH
    // contains / search: wrapper around the book's BTreeSearch.
    // Time: O(h*m) CPU, O(h)=O(log_t n) disk accesses. Space: O(h).
    bool contains(const T& K) const {
        int idx;
        return BTreeSearch(K, root, idx) != NULL;
    }

    // searchPrint: same search, but prints the path (node by node) so one can
    // see how many nodes (disk reads) were needed.
    // Time: O(h*m) CPU (printing a node is O(m) too), O(h) disk accesses.
    // Space: O(m) (the temporary string built for one node; the search itself is
    // iterative and needs no stack).
    bool searchPrint(const T& K) const {
        Node* node = root;
        int level = 1;
        while (node) {
            int i = findPos(node, K);
            cout << "   level " << level << ": " << keysStr(node) << "\n";
            if (isEqualAt(node, i, K)) {
                cout << "   found at position " << i << " of this node (" << level << " node(s) read)\n";
                return true;
            }
            if (node->leaf) break;
            node = node->pointers[i];
            level++;
        }
        cout << "   not found (" << level << " node(s) read)\n";
        return false;
    }

    //  INSERTION  -- way 1: BOTTOM-UP (book's BTreeInsert)
    //  Go to the leaf, put the key there; if the node overflows split it,
    //  promote the middle key and repeat on the parent.
    //  The 3 cases of the book:
    //    Case 1: leaf has room                -> just insert
    //    Case 2: leaf full -> split, promote  -> may propagate upward
    //    Case 3: root full -> new root        -> only case where height grows
    //  Returns false if K is already in the tree (duplicates rejected).
    //
    //  Time : descent O(h*m) + at most h splits * O(m) = O(h*m) = O(m*log_t n).
    //         Disk accesses: O(h) reads and at most 2h+1 node writes = O(h).
    //         Book: when a tree is built by insertions only, the average
    //         probability of a split is about 1/(ceil(m/2)-1) per inserted key,
    //         so amortized O(1) splits per insertion (h splits is the worst case).
    //  Space: O(h + m): O(h) for the stored path (no recursion) plus O(m)
    //         temporary if a node's vector reallocates when a key is inserted.
    //         Each split also creates one new O(m) node that becomes part of the tree.
    bool insertBottomUp(const T& K) {
        if (root == NULL) {                         // empty tree: new root is a leaf
            root = new Node(true);
            root->keys.push_back(K);
            trace("Case 1: tree was empty, new root leaf created");
            return true;
        }
        vector<PathItem> path;                      // remembers parents + child index
        Node* node = root;
        while (true) {                              // go down to the proper leaf
            int i = findPos(node, K);
            if (isEqualAt(node, i, K)) { return false; }   // duplicate
            if (node->leaf) break;
            path.push_back(PathItem(node, i));
            node = node->pointers[i];
        }

        T key = K;
        Node* rightChild = NULL;
        int pos = findPos(node, key);
        int splits = 0;
        bool rootSplit = false;
        while (true) {
            node->keys.insert(node->keys.begin() + pos, key);              // O(m)
            if (rightChild)
                node->pointers.insert(node->pointers.begin() + pos + 1, rightChild);
            if (node->keyTally() <= maxKeys) break;                        // node not overflowing: done

            // overflow: node has m keys -> split into two nodes, promote the middle key
            T up;
            Node* right = splitNode(node, m / 2, up);                      // O(m)
            splits++;
            if (path.empty()) {                                            // node was the root (Case 3)
                Node* newRoot = new Node(false);
                newRoot->keys.push_back(up);
                newRoot->pointers.push_back(node);
                newRoot->pointers.push_back(right);
                root = newRoot;
                rootSplit = true;
                break;
            }
            pos = path.back().second;                                      // continue with parent
            node = path.back().first;
            path.pop_back();
            key = up;
            rightChild = right;
        }
        if (splits == 0)       trace("Case 1: leaf had room, key placed in leaf");
        else if (!rootSplit)   { ostringstream os; os << "Case 2: leaf was full, " << splits
                                  << " split(s), middle key(s) promoted"; trace(os.str()); }
        else                   { ostringstream os; os << "Case 3: root was full too (" << splits
                                  << " split(s)), new root created, height increased"; trace(os.str()); }
        return true;
    }

    //  INSERTION  -- way 2: PRESPLITTING (top-down)
    //  While going down, every FULL node that is visited is split at once,
    //  so a split never has to be propagated upward and no parent stack
    //  is needed. A full node always has a parent with room (or is the root).
    //
    //  NOTE: a full node has m-1 keys; after promoting one key the remaining
    //  m-2 keys must be divisible into two halves of >= ceil(m/2)-1 keys.
    //  That works only when m is EVEN. For odd m this function falls back to
    //  the bottom-up algorithm (so the tree always stays a valid B-tree).
    //
    //  Time : O(h*m) for the duplicate check + O(h*m) for the descent with
    //         splits (each level: one scan O(m) + at most one split O(m); then
    //         one insert into the leaf O(m)) = O(h*m) = O(m*log_t n).
    //         Disk accesses: O(h) reads and O(h) writes (at most 3 nodes written
    //         per split).
    //  Space: O(h + m): O(h) for the recursive duplicate check contains(), plus
    //         O(m) temporary if a node's vector reallocates during a split or
    //         insert. The descent itself is iterative and needs no stack.
    //         (For odd m the call falls back to insertBottomUp, same bounds.)
    bool insertPresplit(const T& K) {
        if (m % 2 != 0) {
            trace("presplit needs even order m -> using bottom-up insertion instead");
            return insertBottomUp(K);
        }
        if (contains(K)) return false;     // check first so we do not split nodes for nothing

        if (root == NULL) {
            root = new Node(true);
            root->keys.push_back(K);
            trace("tree was empty, new root leaf created");
            return true;
        }
        int splits = 0;
        if (root->keyTally() == maxKeys) {              // root full: split it first (height grows)
            Node* newRoot = new Node(false);
            newRoot->pointers.push_back(root);
            root = newRoot;
            splitChild(root, 0);
            splits++;
            trace("root was full: presplit it, new root created, height increased");
        }
        Node* node = root;
        while (!node->leaf) {
            int i = findPos(node, K);
            Node* child = node->pointers[i];
            if (child->keyTally() == maxKeys) {         // full child on the way down: split it now
                splitChild(node, i);
                splits++;
                if (node->keys[i] < K) i++;             // decide which half to enter
            }
            node = node->pointers[i];
        }
        node->keys.insert(node->keys.begin() + findPos(node, K), K);   // leaf is guaranteed not full
        ostringstream os;
        os << "presplit insertion done, " << splits << " full node(s) split on the way down";
        trace(os.str());
        return true;
    }

    // default insert = book's BTreeInsert (bottom-up); same complexity as insertBottomUp
    bool insert(const T& K) { return insertBottomUp(K); }

    //  DELETION (book's BTreeDelete)
    //  Case 1: key in a leaf
    //    1.1   leaf still has >= ceil(m/2)-1 keys: just remove + shift
    //    1.2   underflow:
    //      1.2.1 a sibling has more than the minimum  -> REDISTRIBUTE
    //      1.2.2 siblings have only the minimum       -> MERGE (may chain upward)
    //      1.2.2.1 parent is the root with a single key -> merge into new root,
    //                height decreases by one
    //  Case 2: key in a non-leaf: replace it by its immediate PREDECESSOR (the
    //          largest key in the left subtree - always in a leaf), then delete
    //          that predecessor from the leaf (-> Case 1).
    //  Returns false if K is not in the tree.
    //
    //  Time : find O(h*m) + (predecessor walk O(h)) + at most h-1 merges and at
    //         most one redistribution, each O(m) = O(h*m) = O(m*log_t n).
    //         Disk accesses: O(h) reads (path plus at most 2 siblings per level)
    //         and O(h) writes in the worst case.
    //  Space: O(h) path stack, plus O(m) temporary vectors inside redistribute.
    bool BTreeDelete(const T& K) {
        if (root == NULL) return false;
        vector<PathItem> path;
        Node* node = root;
        int i;
        while (true) {                                   // locate K, remember the path
            i = findPos(node, K);
            if (isEqualAt(node, i, K)) break;
            if (node->leaf) return false;                // not found
            path.push_back(PathItem(node, i));
            node = node->pointers[i];
        }

        if (!node->leaf) {                               // Case 2: non-leaf
            Node* target = node;
            int ti = i;
            path.push_back(PathItem(node, i));
            Node* cur = node->pointers[i];               // go to left subtree ...
            while (!cur->leaf) {                         // ... then as far right as possible
                path.push_back(PathItem(cur, cur->keyTally()));
                cur = cur->pointers[cur->keyTally()];
            }
            target->keys[ti] = cur->keys.back();         // copy predecessor over K
            cur->keys.pop_back();                        // delete predecessor from the leaf
            node = cur;
            trace("Case 2: key was in a non-leaf, replaced by its predecessor from a leaf");
        } else {
            node->keys.erase(node->keys.begin() + i);    // Case 1: remove from leaf
        }

        bool underflow = (!path.empty() && node->keyTally() < minKeys);
        if (!underflow) {
            if (path.empty() && node->keyTally() == 0)
                trace("Case 1.1: the only key of the root leaf was removed, tree becomes empty");
            else if (path.empty())
                trace("Case 1.1: key removed from the root leaf (a root has no minimum), nothing else to do");
            else
                trace("Case 1.1: leaf still at least half full, nothing else to do");
        }

        bool done = false;
        while (!done && !path.empty() && node->keyTally() < minKeys) {
            Node* parent = path.back().first;
            int idx = path.back().second;
            path.pop_back();
            Node* leftS  = (idx > 0) ? parent->pointers[idx - 1] : NULL;
            Node* rightS = (idx < parent->keyTally()) ? parent->pointers[idx + 1] : NULL;

            if (leftS && leftS->keyTally() > minKeys) {                 // Case 1.2.1
                redistribute(parent, idx - 1);
                trace("Case 1.2.1: underflow fixed by redistributing keys with LEFT sibling");
                done = true;
            } else if (rightS && rightS->keyTally() > minKeys) {        // Case 1.2.1
                redistribute(parent, idx);
                trace("Case 1.2.1: underflow fixed by redistributing keys with RIGHT sibling");
                done = true;
            } else {                                                    // Case 1.2.2
                if (rightS) mergeNodes(parent, idx);         // prefer RIGHT sibling (as in Fig 7.9d)
                else        mergeNodes(parent, idx - 1);     // last child: no right sibling, use left
                trace("Case 1.2.2: siblings at minimum -> merged nodes, parent lost a key");
                node = parent;                                          // parent may underflow now
            }
        }
        if (root->keyTally() == 0) {                    // root became empty
            Node* old = root;
            if (root->leaf) { root = NULL; }
            else {
                root = root->pointers[0];
                trace("Case 1.2.2.1: root had one key -> merged into new root, height decreased");
            }
            delete old;
        }
        return true;
    }
    // remove: same complexity as BTreeDelete
    bool remove(const T& K) { return BTreeDelete(K); }

    //  TRAVERSALS
    // inorder: prints all keys in sorted order.
    // Time O(n) (every key printed once, every node entered once, N <= n),
    // Space O(h) recursion.
    void inorder() const { inorderRec(root); cout << "\n"; }

    // levelOrder (BFS): prints the tree level by level.
    // Time O(n) (every node is enqueued, dequeued and printed once, N <= n).
    // Space O(w + m): the queue holds nodes of at most two neighbouring levels
    // (<= 2w), plus the temporary string of one node (O(m)).
    void levelOrder() const {
        if (!root) { cout << "(empty tree)\n"; return; }
        queue<Node*> q;
        q.push(root);
        int level = 1;
        while (!q.empty()) {
            int cnt = (int)q.size();
            cout << "Level " << level++ << ": ";
            while (cnt--) {
                Node* n = q.front(); q.pop();
                cout << keysStr(n) << " ";
                if (!n->leaf)
                    for (size_t i = 0; i < n->pointers.size(); i++) q.push(n->pointers[i]);
            }
            cout << "\n";
        }
    }

    // printTree: preorder, indented by depth (shows parent/child structure).
    // Time O(n + N*h): every key is printed once, and every node first prints an
    // indent of 4*depth spaces (depth <= h), so the indents cost up to N*h.
    // Space O(h + m): recursion depth h, plus the indent string (O(h)) and the
    // key string of one node (O(m)).
    void printTree() const {
        if (!root) { cout << "(empty tree)\n"; return; }
        printRec(root, 0);
    }

    //  EXTRA UTILITY FUNCTIONS
    // height: number of levels (empty = 0, only root = 1). Time O(h), Space O(1).
    int height() const {
        int h = 0;
        for (Node* n = root; n; n = n->leaf ? NULL : n->pointers[0]) h++;
        return h;
    }

    // findMin: leftmost key of leftmost leaf. Time O(h), Space O(1).
    bool findMin(T& out) const {
        if (!root) return false;
        Node* n = root;
        while (!n->leaf) n = n->pointers[0];
        out = n->keys.front();
        return true;
    }
    // findMax: rightmost key of rightmost leaf. Time O(h), Space O(1).
    bool findMax(T& out) const {
        if (!root) return false;
        Node* n = root;
        while (!n->leaf) n = n->pointers[n->keyTally()];
        out = n->keys.back();
        return true;
    }

    // countKeys / countNodes / countLeaves: Time O(N) (visits every node), Space O(h).
    // (They could be O(1) if counters were maintained on every insert/delete.)
    int countKeys()   const { return countKeysRec(root); }
    int countNodes()  const { return countNodesRec(root); }
    int countLeaves() const { return countLeavesRec(root); }

    // utilization: percentage of key cells in use = n / (N*(m-1)) * 100.
    // The book says random insertions/deletions give about 69%.
    // Time O(N), Space O(h).
    double utilization() const {
        int N = countNodes();
        if (N == 0) return 0.0;
        return 100.0 * countKeys() / ((double)N * maxKeys);
    }

    // rangeSearch: all keys x with low <= x <= high, in sorted order.
    // Time O(h*m + k): only subtrees that overlap the range are entered. At most
    // 2 nodes per level contain low or high inside their interval (O(h) nodes,
    // each scanned in O(m)); every other visited node lies completely inside the
    // range, so all of its keys are output and its cost is charged to k.
    // Space O(h + k): recursion depth h plus the output vector of k keys.
    vector<T> rangeSearch(const T& low, const T& high) const {
        vector<T> out;
        rangeRec(root, low, high, out);
        return out;
    }

    // successor: smallest key strictly greater than K (K need not be in the tree).
    // Time O(h*m), Space O(1).
    bool successor(const T& K, T& out) const {
        Node* n = root;
        bool found = false;
        while (n) {
            int i = 0;
            while (i < n->keyTally() && !(K < n->keys[i])) i++;   // first key > K
            if (i < n->keyTally()) { out = n->keys[i]; found = true; }
            if (n->leaf) break;
            n = n->pointers[i];
        }
        return found;
    }
    // predecessor: largest key strictly smaller than K. Time O(h*m), Space O(1).
    bool predecessor(const T& K, T& out) const {
        Node* n = root;
        bool found = false;
        while (n) {
            int i = findPos(n, K);                                // first key >= K
            if (i > 0) { out = n->keys[i - 1]; found = true; }
            if (n->leaf) break;
            n = n->pointers[i];
        }
        return found;
    }

    // validate: checks every B-tree property of the book (key counts, sorted
    // order and ranges, all leaves on one level, root rule). Used for testing.
    // Time O(n), Space O(h).
    bool validate(string& err) const {
        if (!root) { err = "ok (empty)"; return true; }
        int leafDepth = -1;
        bool ok = validateRec(root, NULL, NULL, 1, leafDepth, true, err);
        if (ok) err = "ok";
        return ok;
    }
};

//                              MENU-DRIVEN MAIN
static bool readLine(string& line) { return (bool)getline(cin, line); }

// parseWholeInt: true only if s is one whole number (surrounding spaces allowed,
// nothing else). Rejects "12abc", "3.5", "abc", "" and numbers that overflow int.
static bool parseWholeInt(const string& s, int& v) {
    istringstream is(s);
    char extra;
    return (is >> v) && !(is >> extra);
}

static bool readInt(const string& prompt, int& v) {
    string line;
    while (true) {
        cout << prompt;
        if (!readLine(line)) return false;           // EOF
        if (parseWholeInt(line, v)) return true;
        cout << "  please type a whole number\n";
    }
}

static void printMenu() {
    cout << "\n================ B-TREE MENU ================\n"
         << " 1. Insert key (bottom-up, book's BTreeInsert)\n"
         << " 2. Insert key (presplitting, top-down)\n"
         << " 3. Insert many keys (one line, space separated)\n"
         << " 4. Delete key\n"
         << " 5. Search key (shows path)\n"
         << " 6. Display level-order\n"
         << " 7. Display indented tree\n"
         << " 8. Inorder traversal (sorted)\n"
         << " 9. Range search\n"
         << "10. Min / Max\n"
         << "11. Successor / Predecessor\n"
         << "12. Statistics (height, keys, nodes, leaves, utilization)\n"
         << "13. Validate B-tree properties\n"
         << "14. Clear tree\n"
         << "15. Turn case-trace ON/OFF\n"
         << " 0. Exit\n"
         << "Choice: ";
}

#ifndef BTREE_NO_MAIN
int main() {
    int order;
    cout << "B-Tree implementation (order m = max children per node)\n";
    if (!readInt("Enter order m (>= 3): ", order)) return 0;
    if (order < 3) { cout << "order too small, using 3\n"; order = 3; }
    BTree<int> tree(order, true);
    bool trace = true;
    cout << "Min keys per non-root node = " << ((order + 1) / 2 - 1)
         << ", max keys per node = " << (order - 1) << "\n";
    if (order % 2 != 0)
        cout << "(note: presplit insertion needs even m; for odd m option 2 uses bottom-up)\n";

    while (true) {
        printMenu();
        int ch;
        string line;
        if (!readLine(line)) break;
        if (!parseWholeInt(line, ch)) { cout << "invalid choice\n"; continue; }
        int x, y;
        if (ch == 0) break;
        switch (ch) {
        case 1:
            if (!readInt("Key to insert: ", x)) return 0;
            if (!tree.insertBottomUp(x)) cout << "  duplicate key, not inserted\n";
            break;
        case 2:
            if (!readInt("Key to insert: ", x)) return 0;
            if (!tree.insertPresplit(x)) cout << "  duplicate key, not inserted\n";
            break;
        case 3: {
            cout << "Keys: ";
            if (!readLine(line)) return 0;
            vector<int> keys;                        // validate every token before inserting anything
            string tok, badTok;
            bool bad = false;
            istringstream is(line);
            while (is >> tok) {
                int v;
                if (parseWholeInt(tok, v)) keys.push_back(v);
                else { bad = true; badTok = tok; break; }
            }
            if (bad) { cout << "  invalid key '" << badTok << "', nothing inserted\n"; break; }
            if (keys.empty()) { cout << "  no keys entered\n"; break; }
            int method;
            while (true) {
                if (!readInt("Method: 1 = bottom-up, 2 = presplit : ", method)) return 0;
                if (method == 1 || method == 2) break;
                cout << "  please type 1 or 2\n";
            }
            for (size_t i = 0; i < keys.size(); i++) {
                cout << "insert " << keys[i] << "\n";
                bool ok = (method == 2) ? tree.insertPresplit(keys[i]) : tree.insertBottomUp(keys[i]);
                if (!ok) cout << "  duplicate key " << keys[i] << ", skipped\n";
            }
            break;
        }
        case 4:
            if (!readInt("Key to delete: ", x)) return 0;
            if (!tree.BTreeDelete(x)) cout << "  key not found\n";
            else cout << "  deleted " << x << "\n";
            break;
        case 5:
            if (!readInt("Key to search: ", x)) return 0;
            tree.searchPrint(x);
            break;
        case 6: tree.levelOrder(); break;
        case 7: tree.printTree(); break;
        case 8: cout << "Inorder: "; tree.inorder(); break;
        case 9: {
            if (!readInt("Low : ", x)) return 0;
            if (!readInt("High: ", y)) return 0;
            vector<int> r = tree.rangeSearch(x, y);
            cout << "Keys in [" << x << ", " << y << "]: ";
            for (size_t i = 0; i < r.size(); i++) cout << r[i] << " ";
            cout << "(" << r.size() << " key(s))\n";
            break;
        }
        case 10: {
            int mn, mx;
            if (tree.findMin(mn) && tree.findMax(mx)) cout << "Min = " << mn << ", Max = " << mx << "\n";
            else cout << "tree is empty\n";
            break;
        }
        case 11: {
            if (!readInt("Key: ", x)) return 0;
            int s, p;
            if (tree.successor(x, s))   cout << "Successor   : " << s << "\n"; else cout << "No successor\n";
            if (tree.predecessor(x, p)) cout << "Predecessor : " << p << "\n"; else cout << "No predecessor\n";
            break;
        }
        case 12:
            cout << "Order        : " << tree.order() << "\n"
                 << "Height       : " << tree.height() << "\n"
                 << "Keys         : " << tree.countKeys() << "\n"
                 << "Nodes        : " << tree.countNodes() << "\n"
                 << "Leaves       : " << tree.countLeaves() << "\n"
                 << "Utilization  : " << fixed << setprecision(1) << tree.utilization() << " %\n";
            cout.unsetf(ios::floatfield);
            break;
        case 13: {
            string err;
            bool ok = tree.validate(err);
            cout << (ok ? "VALID B-tree: " : "INVALID B-tree: ") << err << "\n";
            break;
        }
        case 14: tree.clear(); cout << "tree cleared\n"; break;
        case 15: trace = !trace; tree.setVerbose(trace); cout << "case trace " << (trace ? "ON" : "OFF") << "\n"; break;
        default: cout << "invalid choice\n";
        }
    }
    cout << "Bye.\n";
    return 0;
}
#endif
