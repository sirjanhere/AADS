#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
using namespace std;

/*
    Function: KMPFailureFunction

    Computes the failure function f for pattern P.
    f[i] is the length of the longest proper prefix of P[0..i]
    that is also a suffix of P[0..i].

    Time Complexity:
    Best Case: O(m)
    Average Case: O(m)
    Worst Case: O(m)

    Here, m is the length of pattern P.
*/
vector<size_t> KMPFailureFunction(const string& P)
{
    const size_t m = P.size();
    vector<size_t> f(m, 0);

    if (m == 0)
    {
        return f;
    }

    size_t i = 1;
    size_t j = 0;

    while (i < m)
    {
        if (P[j] == P[i])
        {
            f[i] = j + 1;
            ++i;
            ++j;
        }
        else if (j > 0)
        {
            j = f[j - 1];
        }
        else
        {
            f[i] = 0;
            ++i;
        }
    }

    return f;
}

/*
    Function: KMPMatch

    Finds the first occurrence of pattern P in text T.
    Returns the zero-based starting index if found,
    otherwise returns -1.

    Time Complexity:
    Best Case: O(m)
    Average Case: O(n + m)
    Worst Case: O(n + m)

    Here, n is the length of text T and m is the length
    of pattern P. The failure function takes O(m) time,
    and the matching phase takes O(n) time in the worst case.
    Best Case: O(1) when m > n (early return). Otherwise O(m), when the pattern matches at the beginning of the text.
*/
long long KMPMatch(const string& T, const string& P)
{
    const size_t n = T.size();
    const size_t m = P.size();

    if (m == 0)
    {
        return 0;
    }

    if (m > n)
    {
        return -1;
    }

    const vector<size_t> f = KMPFailureFunction(P);

    size_t i = 0;
    size_t j = 0;

    while (i < n)
    {
        if (P[j] == T[i])
        {
            if (j == m - 1)
            {
                return static_cast<long long>(i - m + 1);
            }

            ++i;
            ++j;
        }
        else if (j > 0)
        {
            j = f[j - 1];
        }
        else
        {
            ++i;
        }
    }

    return -1;
}

int main()
{
    string P, T;

    cout << "Enter the pattern: ";

    if (!getline(cin, P))
    {
        cerr << "Error: Unable to read the pattern.\n";
        return 1;
    }

    if (P.empty())
    {
        cerr << "Error: Pattern cannot be empty.\n";
        return 1;
    }

    cout << "Enter the text: ";

    if (!getline(cin, T))
    {
        cerr << "Error: Unable to read the text.\n";
        return 1;
    }

    vector<size_t> f = KMPFailureFunction(P);

    size_t maxValue = P.size() - 1;

    for (size_t i = 0; i < f.size(); ++i)
    {
        maxValue = max(maxValue, f[i]);
    }

    int columnWidth = max(
        5,
        static_cast<int>(to_string(maxValue).length()) + 2
    );

    const int labelWidth = 18;

    cout << "\nFailure Function Table\n";

    cout << left << setw(labelWidth) << "Pattern index i";

    for (size_t i = 0; i < P.size(); ++i)
    {
        cout << right << setw(columnWidth) << i;
    }

    cout << '\n';

    cout << left << setw(labelWidth) << "Failure value f(i)";

    for (size_t i = 0; i < f.size(); ++i)
    {
        cout << right << setw(columnWidth) << f[i];
    }

    cout << '\n';

    const long long result = KMPMatch(T, P);

    cout << "\nKMP Pattern Matching Result\n";

    if (result == -1)
    {
        cout << "Pattern not found in the text.\n";
    }
    else
    {
        cout << "Pattern found in the text.\n";
        cout << "Starting index (zero-based): "
             << result << '\n';
        cout << "Starting position (one-based): "
             << result + 1 << '\n';
    }

    return 0;
}
