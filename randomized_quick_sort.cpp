/*

Randomized Quick Sort works like Quick Sort, but the pivot is selected randomly.

Steps of Randomized Quick Sort:
1. Select a random index between low and high.
2. Swap the element at the random index with the last element.
3. Use the last element as the pivot for partitioning.
4. Compare each element with the pivot and place smaller/equal elements on the left side of the pivot.
5. Put the pivot at its correct position.
6. Recursively apply the same process to the left and right subarrays.
7. Count the comparisons made during partitioning.
8. Run the algorithm on 100 different randomly generated inputs with sizes varying from 30 to 1000.
9. Store n, number of comparisons, and nlogn in randomized_quick_sort.csv.

Difference between Quick Sort and Randomized Quick Sort:
- In normal Quick Sort, the pivot is usually selected using a fixed rule, such as the first element, last element, or middle element.
- In Randomized Quick Sort, the pivot index is selected randomly before partitioning.
- The line "uniform_int_distribution<int> dist(low, high)" together with "int randomIndex = dist(rng)" is what makes this Randomized Quick Sort.
- Randomized Quick Sort has expected time complexity O(n log n), while its worst-case time complexity is O(n^2), just like Quick Sort.
- Expected auxiliary recursion-stack space is O(log n), while worst-case recursion-stack space is O(n).

Comparison counting used in this program:
- Every evaluation of "arr[j] <= pivot" is counted as one comparison.
- Pivot selection and the internal random-number generation are not counted as element-to-pivot comparisons.

Complexities:
- Best case: O(n log n)
- Average/Expected case: O(n log n)
- Worst case: O(n^2)
- Expected auxiliary space: O(log n)
- Worst-case auxiliary space: O(n)
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <iomanip>

using namespace std;

// This function partitions the array around the selected pivot.
// It returns the final position of the pivot.
int partitionArray(vector<int>& arr, int low, int high, long long& comparisons)
{
    // Step 3 of Randomized Quick Sort:
    // After the random pivot is moved to the end, arr[high] is used as the pivot.
    int pivot = arr[high];

    int i = low - 1;

    // Step 4 of Randomized Quick Sort:
    // Compare every element with the pivot and move smaller/equal elements left.
    for (int j = low; j < high; j++)
    {
        // Count one element-to-pivot comparison.
        comparisons++;

        if (arr[j] <= pivot)
        {
            i++;
            swap(arr[i], arr[j]);
        }
    }

    // Step 5 of Randomized Quick Sort:
    // Place the pivot between the smaller/equal and larger elements.
    swap(arr[i + 1], arr[high]);

    return i + 1;
}

// This function performs Randomized Quick Sort recursively.
void randomizedQuickSort(vector<int>& arr, int low, int high,
                         long long& comparisons, mt19937& rng)
{
    // Step 6 of Randomized Quick Sort:
    // Continue recursively only when the current subarray has at least two elements.
    if (low >= high)
        return;

    // Step 1 of Randomized Quick Sort:
    // Select a random pivot index from the current subarray.
    // This random selection is the main difference from ordinary Quick Sort.
    uniform_int_distribution<int> dist(low, high);
    int randomIndex = dist(rng);

    // Step 2 of Randomized Quick Sort:
    // Move the randomly selected pivot to the last position so that partitionArray() can use arr[high] as the pivot.
    swap(arr[randomIndex], arr[high]);

    // Steps 3-5 are performed inside partitionArray().
    int pivotIndex = partitionArray(arr, low, high, comparisons);

    // Step 6 of Randomized Quick Sort:
    // Recursively sort the left and right parts of the array.
    randomizedQuickSort(arr, low, pivotIndex - 1, comparisons, rng);
    randomizedQuickSort(arr, pivotIndex + 1, high, comparisons, rng);
}

// Generates one random input array of the required size.
vector<int> generateInput(int n, mt19937& rng)
{
    vector<int> arr(n);

    // Generate random values for the input array.
    uniform_int_distribution<int> valueDist(1, 1000000);

    for (int i = 0; i < n; i++)
    {
        arr[i] = valueDist(rng);
    }

    return arr;
}

int main()
{
    // Fixed seed makes the experiment reproducible.
    mt19937 rng(123456);

    // Create the CSV file required for the practical.
    ofstream csvFile("randomized_quick_sort.csv");

    if (!csvFile)
    {
        cerr << "Error: Could not create randomized_quick_sort.csv" << endl;
        return 1;
    }

    // The CSV contains exactly the three columns required for the graph:
    // n, number of comparisons, and nlogn.
    csvFile << "n,comparisons,nlogn\n";

    /*
    Generate 100 different input sizes.

    The first size is 30 and the last size is 1000.
    The sizes are distributed approximately uniformly between 30 and 1000.
    This gives 100 different input sizes for the experiment.
    */
    for (int test = 0; test < 100; test++)
    {
        int n = static_cast<int>(
            round(30.0 + test * (1000.0 - 30.0) / 99.0)
        );

        // Generate a new random input for this test.
        vector<int> arr = generateInput(n, rng);

        long long comparisons = 0;

        // Run Randomized Quick Sort on the generated input.
        randomizedQuickSort(arr, 0, n - 1, comparisons, rng);

        // Calculate n log2(n) for comparison with the experimental result.
        double nlogn = n * log2(static_cast<double>(n));

        // Store the three required values in the CSV file.
        csvFile << n << ","
                << comparisons << ","
                << fixed << setprecision(4) << nlogn << "\n";
    }

    csvFile.close();

    cout << "Randomized Quick Sort experiment completed successfully." << endl;
    cout << "100 inputs were tested with sizes from 30 to 1000." << endl;
    cout << "CSV file generated: randomized_quick_sort.csv" << endl;

    return 0;
}
