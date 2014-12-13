## Solution to Project Euler Problem 67 in C++

This is my solution to Problem 67. It is heavily annotated because I am
submitting it for a code evaluation.

### Problem

The problem is described [here](https://projecteuler.net/problem=67).

### Solution

My solution is implemented in euler67.cpp.

* First I define a Triangle datatype which represents a triangular cascade of
  numbers as described in the Problem 67. It is designed to statically
  guarantee the proper structure for solving the problem.

* I then define a generic higher-order function that performs a bottom-up
  traversal of the Triangle.

* Finally, I utilize that function to write a solution to the specific problem.
  The solution can be defined in terms of a recursive function over the Triangle
  structure (but because this is C++ I don't use actual function recursion).

### Compiling

The code requires a C++11 compiler. It has no other dependencies. On my linux
system, I run it from the project directory with:

```shell
make && ./euler67

```
