#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

/*
    Randomized Select:
    Finds the k-th smallest element in an array.

    Time Complexity:
    Best Case      : O(n)
    Expected Case  : O(n)
    Worst Case     : O(n^2)

    Space Complexity:
    Average / Expected Case   : O(log n)  - recursive call stack
    Worst Case                : O(n)      - recursive call stack
*/


// Partition the array around the pivot
int partitionArray(int arr[], int low, int high)
{
    /*
        Step 1:
        Select the last element as the pivot.

        The pivot has already been randomly selected and moved to the last position by randomizedPartition().
    */
    int pivot = arr[high];

    // i stores the position where the next smaller element will go.
    int i = low - 1;

    /*
        Step 2:
        Compare every element with the pivot.

        Time Complexity of this loop: O(n)
        because each element in the current subarray is checked once.
    */
    for (int j = low; j < high; j++)
    {
        if (arr[j] <= pivot)
        {
            i++;

            // Move the smaller/equal element to the left side.
            swap(arr[i], arr[j]);
        }
    }

    /*
        Step 3:
        Place the pivot at its correct sorted position.

        After this:
        Elements on the left  <= pivot
        Elements on the right > pivot
    */
    swap(arr[i + 1], arr[high]);

    // Return the final position of the pivot.
    return i + 1;
}


// Performs partitioning using a randomly selected pivot
int randomizedPartition(int arr[], int low, int high)
{
    /*
        Step 1:
        Generate a random index between low and high.

        This is what makes the algorithm randomized.
    */
    int randomIndex = low + rand() % (high - low + 1);

    /*
        Step 2:
        Move the randomly selected pivot to the last position.
    */
    swap(arr[randomIndex], arr[high]);

    /*
        Step 3:
        Partition the array using the random pivot.

        Time Complexity: O(n)
    */
    return partitionArray(arr, low, high);
}


// Randomized Select algorithm
int randomizedSelect(int arr[], int low, int high, int k)
{
    /*
        Base Case:
        If the subarray contains only one element, that element is the answer.

        The operation itself takes O(1) time.
    */
    if (low == high)
        return arr[low];

    /*
        Step 1:
        Choose a random pivot and partition the array.

        Time Complexity of partitioning: O(n)
        where n is the size of the current subarray.
    */
    int pivotIndex = randomizedPartition(arr, low, high);

    /*
        rank gives the position of the pivot
        within the current subarray.

        Example:
        If rank = 3, the pivot is the 3rd smallest element in the current subarray.
    */
    int rank = pivotIndex - low + 1;

    /*
        Step 2:
        If the pivot is exactly the k-th smallest element, return it.
    */
    if (k == rank)
        return arr[pivotIndex];

    /*
        Step 3:
        If k is smaller than the pivot's rank, search only in the left partition.

        We do NOT process the right partition.

        This is the main reason Randomized Select is expected O(n), unlike Quick Sort which recursively processes both partitions.
    */
    else if (k < rank)
        return randomizedSelect(arr, low, pivotIndex - 1, k);

    /*
        Step 4:
        If k is greater than the pivot's rank, search only in the right partition.

        k is adjusted because the left partition and the pivot have already been excluded.
    */
    else
        return randomizedSelect(
            arr,
            pivotIndex + 1,
            high,
            k - rank
        );
}


int main()
{
    /*
        Step 1:
        Take the size of the array from the user.
    */
    int n;
    cout << "Enter the number of elements: ";
    cin >> n;

    // Validate array size.
    if (n <= 0)
    {
        cout << "Array size must be positive." << endl;
        return 0;
    }

    /*
        Step 2:
        Create the array and take its elements from the user.
    */
    int* arr = new int[n];

    cout << "Enter " << n << " elements:" << endl;

    for (int i = 0; i < n; i++)
    {
        cin >> arr[i];
    }

    /*
        Step 3:
        Take k from the user.

        k = 1 means smallest element
        k = 2 means second smallest element
        ...
        k = n means largest element
    */
    int k;
    cout << "Enter k (1 to " << n << "): ";
    cin >> k;

    // Validate k.
    if (k < 1 || k > n)
    {
        cout << "Invalid value of k." << endl;

        delete[] arr;
        return 0;
    }

    /*
        Step 4:
        Initialize the random number generator.

        This ensures that different executions can choose different random pivots.
    */
    srand(time(0));

    /*
        Step 5:
        Call Randomized Select.

        It returns the k-th smallest element.
    */
    int result = randomizedSelect(arr, 0, n - 1, k);

    /*
        Step 6:
        Display the result.
    */
    cout << "\nThe " << k << "-th smallest element is: "
         << result << endl;

    // Free dynamically allocated memory.
    delete[] arr;

    return 0;
}
