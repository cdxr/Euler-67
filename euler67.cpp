/******************************************************
 *
 *  A solution to Project Euler Problem 67
 *
 *    -  see https://projecteuler.net/problem=67
 *
 *  Written by Craig Roche
 *
*******************************************************
*/


#include <vector>

#include <iostream>
#include <fstream>
#include <sstream>

#include <functional>     // std::function

#include <stdexcept>      // std::argument_error



/* A Triangle consists of multiple rows of ints where the first row
 * contains one element and each other row has precisely one more element
 * than the row preceeding it. An example Triangle can be depicted like this:
 *
 *     3
 *    7 4     (taken from the description of Project Euler Problem 67)
 *   2 4 6
 *  8 5 9 3
 *
 * It is important that the row-length property is not violated because each
 * number in the Triangle is considered adjacent to the two below it.
 * Ensuring correctness is much simpler if we can guarantee this property
 * statically.
 *
 * This class only allows you to construct Triangles that satisfy this
 * property. You can create Triangles by:
 *   - Constructing an empty Triangle with the default constructor
 *   - Copying, moving, or assigning (from an existing correct Triangle)
 *   - Adding rows to a Triangle one at a time. If the user tries to add
 *     a row with the wrong number of elements, the Triangle is not modified
 *     and std::invalid_argument exception is thrown.
 */
class Triangle {
public:
    using Row = std::vector<int>;

private:
    /*  Class Invariant:    rows_[i].length = i + 1;  */

    std::vector<Row> rows_;

public:
    /* Access the rows of the `Triangle`. This provides a const reference
     * because the user cannot be permitted to change the length of the rows.
     */
    std::vector<Row> const& rows() const
    {
        return rows_;
    }

    /* `Triangle::at(r,n)` returns the n'th value of the r'th row.
     *
     *  The non-const version of this function is the only way for the user
     *  to modify the contents of the Triangle. If the user does not satisfy
     *  the preconditions the result is undefined.
     *
     *  Preconditions:
     *
     *      r < triangle.height()
     *      n <= r + 1
     */
    int  at(size_t row, size_t n) const { return rows_[row][n]; }
    int& at(size_t row, size_t n)       { return rows_[row][n]; }


    /* The height of a Triangle is the number of rows.
     */
    Row::size_type height() const
    {
        return rows_.size();
    }

    /* The width is the size of the bottom-most row. This is equal to the
     * height, but is defined seperately to improve clarity.
     */
    size_t width() const
    {
        return height();
    }

    /* Add a row to the tree. It must have a size equal to the new height of
     * the tree (the current height plus one).
     */
    void append_row(Row&& row)
    {
        if (row.size() != this->height() + 1)
        {
            throw std::invalid_argument(
                "Triangle::append_row requires that input row has a size equal"
                " to the height of the triangle plus one");
        }

        rows_.emplace_back(std::move(row));
    }

    /*  Note: we depend on the default constructors here and let
     *  std::vector do all the work of memory management.
     *
     *  This class is solely responsible for maintaining the
     *  row-length invariant.
     */
};


/* fold_triangle<T>(tri, make_t, combine_t)
 *     - reduces the entire triangle to a single value of type T
 *       by traversing each row from the bottom up
 *     - uses `make_t` to produce a T from every value in the bottom row
 *     - uses `combine_t` to aggregate each number with the two T's below it.
 *     - does not modify the original triangle
 *
 * First a `T` is produced from every `int` in the bottom row of the triangle 
 * using the function `make_t`.
 *
 *     3
 *    7 4       ==>    ts = { make_t(2), make_t(4), make_t(6) }
 *   2 4 6 <-
 *
 *
 * Then, `combine_t` is used to combine each `int` in the next-highest row
 * with the two `T`s that were just produced in the corresponding positions
 * in the row below it. 
 *
 *     3
 *    7 4  <-   ==>    ts' = { combine_t(7, ts[0], ts[1]),
 *   2 4 6                     combine_t(4, ts[1], ts[2])
 *
 *
 * This is applied iteratively to each row until only the top row remains,
 * with a single value of `T`.
 *
 *     3   <-
 *    7 4       ==>    ts'' = { combine_t(3, ts'[0], ts'[1]) }
 *   2 4 6                      
 *
 */
template <typename T>
T fold_triangle(Triangle const& triangle,
       std::function<T(int)> make_t,
       std::function<T(int,T,T)> combine_t)
{
    if (triangle.height() == 0) {
        throw std::invalid_argument(
            "fold_triangle expects a non-empty triangle");
    }

    /* We're going to use this vector to compute the values of each row.
     * With each row iteration, the values of `accum` will be overwritten.
     *
     * This is a vector instead of an array because in general we won't
     * know the size of the Triangle until runtime.
     */
    std::vector<T> accum;
    accum.reserve(triangle.width());

    // We're going to traverse the rows in reverse order.
    // Note: these are const_iterators because rows() returns a const vector
    auto row_iter = triangle.rows().rbegin();
    auto row_end  = triangle.rows().rend();


    // First we fill `accum` with the results of mapping the function
    // `make_t` over the values of the bottom row.
    for (int value: *row_iter)
    {
        accum.emplace_back(make_t(value));
    }

    // Traverse all the rows from the bottom up
    while(++row_iter != row_end)
    {
        // For each row...

        // Start at the beginning of the list of T's
        auto accum_iter = accum.begin();

        /* For each value `n` in the row,
         * apply the user-provided function `combine_t` to `n` and
         * the two below-and-adjacent T's that were previously computed.
         *
         * The bidirectional iterator `accum_iter` is used to read the
         * value from the accumulator and then overwrite it with a new value.
         */
        for (size_t i = 0; i != row_iter->size(); ++i)
        {
            int value = (*row_iter)[i];
            *accum_iter =
                combine_t(value, *accum_iter, *std::next(accum_iter));
            ++accum_iter;
        }

        /* Note:
         *   At this point we could erase the remaining values which were not
         *   overwritten like this:
         *   
         *       accum.erase(accum_iter, accum.end());
         *
         *   However, there is no harm in leaving them there so long as we
         *   don't access them again.
         */
    }

    // All the rows have been processed, and the final reduction is at the
    // front of `accum`.
    return accum.front();
}

/* This function uses `fold_triangle<int>` to compute the solution to
 * Project Euler Number 67.
 *
 * It works by starting at the bottom row and working upwards, eliminating
 * the lesser of adjacent paths until it reaching the top.
 *
 * Note that by using the `fold_triangle` to handle the traversal, this
 * function has been distilled down to its basic components.
 */
int max_path(Triangle const& triangle)
{
    // For the bottom row, the result of each number is simply that number.
    auto leaf = [](int i) -> int { return i; };

    // For every other row, the result of each number is that number plus the
    // greater of the two results immediately under it.
    auto combine = [](int i, int left, int right) -> int {
        return i + std::max(left, right);
    };

    return fold_triangle<int>(triangle, leaf, combine);
}

/* For fun, I added more rules to the problem.
 * As before, we must find the path of maximum value. However, we add the rule
 * that the path may only turn left onto an odd number, and may only go right
 * onto an even number.
 *
 * If the path reaches a point where it cannot continue, it has reached
 * the maximum value of that path.
 *
 * The change is relatively small, because fold_triangle<T> does all the
 * work in traversing the triangle and combining adjacent values.
 */
int max_odd_even_path(Triangle const& triangle)
{
    auto leaf = [](int i) -> int { return i; };

    auto is_even = [](int n) -> bool { return n % 2 == 0; };

    auto combine = [is_even](int i, int left, int right) -> int {
        return i +
            std::max(is_even(left) ? 0 : left,
                     is_even(right) ? right : 0);
    };

    return fold_triangle<int>(triangle, leaf, combine);
}


// Parse a file containing a Triangle in the format provided by
// Project Euler Problem 67.
Triangle parse_triangle(std::ifstream& stream)
{
    Triangle triangle;
    std::string row_string;

    Triangle::Row::size_type expected_row_size = 0;

    // Each line corresponds to a row
    while (std::getline(stream, row_string))
    {
        ++expected_row_size;

        // Triangle::Row is an alias for std::vector<int>
        Triangle::Row row;
        row.reserve(expected_row_size);

        int value;
        std::istringstream row_stream {row_string};

        // Each row contains values seperated by whitespace
        while (row_stream >> value)
        {
            row.push_back(value);
        }

        // C++11 helps us avoid copying the entire row
        triangle.append_row(std::move(row));
    }

    // Hopefully return value optimization will kick in here and avoid
    // copying the entire Triangle.
    return triangle;
}


/* The file containing the triangle located at
 *   https://projecteuler.net/project/resources/p067_triangle.txt
 */
char const* filepath = "p067_triangle.txt";


int main()
{
    std::ifstream file {filepath};
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return 1;
    }

    Triangle const triangle = parse_triangle(file);

    std::cout
        << "Loaded triangle with "
        << triangle.height() << " rows. " << std::endl

        << "The maximum path value is "
        << max_path(triangle) << "." << std::endl

        << "If you may only move left onto an odd number or right onto an even"
            " number, the\nmaximum path value is "
        << max_odd_even_path(triangle) << "." << std::endl;
}

/*  That's it!
 *
 *  Some possible improvements:
 *
 *   - The Triangle class is currently implemented as a container of
 *     `int`s. It would be easy to introduce a template parameter
 *     to make it more generic.
 *
 *   - The parse_triangle function could probably be more terse if I used
 *     some of the fancier stdlib functions. I'm still learning them all.
 *
 *   - At some point I would like to play around with more interesting
 *     applications of the fold_triangle function.
 *
 *   - I haven't written any tests! For the specific input provided by the
 *     problem, the program appears to run correctly.
 */
