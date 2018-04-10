# Examples
Here are some Nandlang examples.

## array.nand
Demonstrates the usage of an array to simulate a block of memory. Included are
indexing functions. The indexN(array, index : ret) function takes in an array
that is 2^N bytes and an index that is N bytes, and returns the value at that
position. The setindexN(array, index, value : ret) function takes in an array
that is 2^N bytes, an index that is N bytes, and a 1 byte value, and returns
the array after the byte at the given index is set to value. There is also a
zero function that initializes an block of memory to zeroes.

The example itself demonstrates how to ask the user to input a string, and then
prints that string but reversed (which is why an array is required).

## basic.nand
Demonstrates basic variable assignment and output.

## fibonacci.nand
Prints out the Fibonacci sequence. Demonstrates recursive functions.

## fizzbuzz.nand
Basic FuzzBuzz example. Also demonstrates recursion.

## helloworld.md
Very basic "Hello, World!" example. Shows how to print characters using putc().

## loop.md
Demonstrates how to take user input and print it back out using functions such
as iogood(), getc(), and putc().

## mathtest.md
Shows how to implement basic math functions, such as subtraction, division,
modulo, multiplication, and addition.

## stacktest.md
Simple test that I use to make sure I didn't mess up any stack code while
programming. 
