# NANDLANG
Nandlang version 1.2

A programming language based on NAND completeness.

Created by Jocelyn Beedie

## About
Do you hate modern programming languages? All pretentious with fancy mumbo
jumbo such as "zero cost abstractions", "RAII principles", and
"object oriented programming"? What the heck does that stuff even mean? Do you
wish that there was a language that so simple, it only had one type and just a
few operators? Well, look no further, because Nandlang is the perfect choice
for you!

Nandlang is a programming language that is based on the idea of NAND
completeness. All common operations and logic gates can be derived from the
NAND gate. As such, the only operators in this programming language are the NAND
operator, function calls, and variable assignment. Nandlang is an imperative
programming language.

### What this is
This is a simple, esoteric language designed mostly as a fun project. It was
also designed to get me a good grade for my computer science final project
assignment.

### What this is not
This is not a serious language with any practical value whatsoever. I highly
doubt that anyone actually wants to design an entire program using just bits.
This implementation of the language uses an interpreter, and is not
very performant, which limits its applications.

## Downloading
You can download the latest release for this program from
https://github.com/Jellonator/Nandlang/releases.

## Building
To build this program, you will need to install scons.

### Ubuntu
Installing dependencies:

```
sudo apt install scons
```

Building the program:

```
scons
```

Running:
```
./nandlang <nandlang script file>
```

### Other platforms
Download scons for your platform from https://scons.org/pages/download.html

I'm not sure what the steps for building the program with scons is for other
platforms, but it's probably very similar as it is for on Ubuntu.

Alternatively if you're using an IDE such as Visual Studio, you can just create
a new project, add all of the source files from the /src/ directory, then
compile it that way. Make sure to set it to use c++14 if you do this however.

## Syntax highlighting
Since this language shares many keywords and syntax with the Javascript
programming language (which is odd since I've barely even touched it), I would
recommend setting Javascript as your text editor's language when editing
Nandlang script files.

## Syntax
Like many programming languages, Nandlang borrows a lot of its syntax from C;
however, there are still many differences. This section will go over the basic
syntax of Nandlang.

You can view the complete syntax definition [here](./bnf.md).

### Types
In Nandlang, there exists only one type: the bit. A bit has a value of either 0,
or 1. When used in a condition, a bit value of 1 acts as true, and a bit value
of 0 acts as false.

You could also consider functions as a type; however, functions in Nandlang are
not currently a first-class type in Nandlang like bits are.

### Identifier names
All identifiers are case-sensitive. So the identifier "asdf" is different from
"AsDf". An identifier may contain any alphanumeric character or an underscore,
and may not start with a digit. An identifier can not be a reserved word, such
as `if`, `while`, or `function`. This rule applies to both variable names and
function names.

### Comments
Anything after a "//" will be ignored for the rest of the line. This allows for
code to be commented. This is the same as in C.

### Declaring functions
Declaring functions is fairly simple:

```Javascript
function myFunctionName() {
    // statements
}
```

Anything inside of the function's name is the block of statements that will be
executed when that function is called.

A function can then be called like so:

```Javascript
myFunctionName();
```

### Statements
A statement can only occur in a function. In the below examples that do not
declare functions, it will be assumed that a they are occurring in a function.

All statements must end with a semicolon.

### Variables and assignment
A variable can be declared using the var statement:

```Javascript
var x = 0;
```

Multiple variables can be declared on the same line:

```Javascript
var y, z = 0, 0;
```

All variables that are declared must be assigned to.
The following will not work:

```Javascript
var a, b, c = 1, 0;
```

Once a variable has been declared, it can then be assigned to:

```Javascript
a, b = 0, 0;
```

A variable that has not yet been declared can not be used. A variable can not
be re-declared once it has already been declared.

### Inputs and outputs
Unlike most C-like languages, Nandlang handles arguments and return values a
little bit differently. A function can have any number of arguments, called
inputs, and any number of return values, called outputs. Inputs and outputs for
a function can be declared like so:

```Javascript
function foo(input1, input2 : output1, output2) {}
```

Note that inputs are separated from outputs using a colon, and that inputs are
separated from other inputs and outputs are separated from other outputs using
commas.

Inputs and outputs will behave just like any other variable inside of the
function call, and can be assigned to or read from. When a function is called,
all of the parameters will be assigned to the inputs, and once a function ends,
it will return all of its outputs.

When a function is called, the parameters given to the function call must match
up exactly to the number of inputs that the function expects. All of the outputs
of the function must also be used. For example, the following examples will
not work:

```Javascript
foo(0);       // not enough inputs given
foo(0, 1, 1); // too many inputs given
foo(0, 1);    // will not work since its outputs are not used.
var x, y, z = foo(0, 1); // foo does not have enough outputs
```

The following, however, will work:

```Javascript
var x, y = foo(0, 1); // OK
```

### If statements
An if statement is how conditions can be used. The basic construct of an if
statement is like so:

```Javascript
if condition {
    // statements
}
```

Note that if statements do NOT end with a semicolon.

If the condition resolves to 1, then the block of statements will execute.
Otherwise, they will not. As such, an if statement expects exactly one output
for its condition. Anything else is a compilation error. For example, the
following will not work:

```Javascript
function foo() {}
function bar(: out1, out2) {}

function main() {
    if foo() {} // foo does not have any outputs
    if bar() {} // bar has too many outputs
}
```

An if statement can also have an else block, which will execute if the given
condition resolves to 0.

```Javascript
if condition {
    // executes when condition is 1
} else {
    // executes when condition is 0
}
```

### While statements
While statements have almost the same syntax and functionality as an if
statement, however it uses the keyword `while` instead of `if`, and it will
repeatedly run its block of statements until the condition resolves to 0.

```Javascript
while condition {
    // statements
}
```

### NAND operator
The only operator in Nandlang is the NAND operator. Due to Nand-completeness,
any other logic gate can be implemented using it. The Nand operator is the `!`
symbol, and will NAND the value of the left side with the value on the right
side of the symbol. For example, here is a NOT logic gate function:

```Javascript
function not(input : output) {
    output = input ! input;
}
```

### Character literals
Character literals, e.g. `'A'`, are automatically expanded into eight
bit literals. For example, `'A'` becomes `0, 1, 0, 0, 0, 0, 0, 1`.

### Ignoring outputs
Sometimes, you may want to ignore the outputs of a function. This can be done
using a single underscore as an identifier:

```Javascript
// full adder
function add(a, b, cin : v, cout) {
    var nab = a ! b;
    v = (a ! nab) ! (b ! nab);
    var nvc = v ! cin;
    cout = nvc ! nab;
    v = (v ! nvc) ! (cin ! nvc);
}

function main() {
    var value, _ = add(1, 1, 0);
    putb(value);
    // putb(_) will cause a compilation error, since _ can't be used as a real
    // identifier.
}
```

### Arrays
An array in Nandlang can be declared using square brackets:

```Javascript
var foo[8] = 0,0,0,0,0,0,0,0;
```

Once you declare an array, you can assign to or retrieve individual elements of
the array also by using square brackets:

```Javascript
foo[0] = foo[3];
```

Attempting to use an out of bounds index in an array is a compilation error:

```Javascript
foo[3] = foo[10];// cannot index foo[10], only has size of 8
```

You can also use the ignore character as an array to ignore multiple values:

```Javascript
foo[1], _[6], foo[2] = foo;
```

It should be noted that arrays in Nandlang are really just syntactic sugar.
There is no difference between declaring `b[4]` and `b0, b1, b2, b3`.

A special size exists called `[ptr]` which represents the system's pointer size.
On 32 bit systems `[ptr]` is equivalent to `[32]`, and on 64 bit systems `[ptr]`
is equivalent `[64]`.

### For statements
A for statement can be used to shorten long, repetitive code and is necessary
for operating on pointer types. A for statement essentially breaks a value up
into multiple smaller types.

The syntax for a for statement is as such:

```Javascript
for (name1, :name2[8], name3[16]) {
    ...
}
```

A for statement can have any number of names, and each name in the list can have
any size. Each name in the list of names must match an existing variable name,
and the size of the variable must be divisible by the size given in the
for loop's list of names. If a name begins with a colon, then it will be
iterated in reverse order. The number of iterations for each name should match.

Here is an example for outputing a number as binary:

```Javascript
function putb8(input) {
    for (input) {
        putb(input);
    }
}
```

Here is another example for NANDing two variables together:
```Javascript
function nand8(a[8], b[8] : o[8]) {
    for (a, b, o) {
        o = a!b;
    }
}
```

Using for loops is also useful for addition. Here is an example addptr function
for adding two pointers together:

```Javascript
function add(a, b, cin : v, cout) {
    var nab = a ! b;
    v = (a ! nab) ! (b ! nab);
    var nvc = v ! cin;
    cout = nvc ! nab;
    v = (v ! nvc) ! (cin ! nvc);
}

function addptr(a[ptr], b[ptr] : o[ptr]) {
    var c = 0;
    for (:a, :b, :o) {
        o, c = add(a, b, c);
    }
}
```

Unlike previous examples, the iteration is performed in reverse order. This is
because the carry bit must carry from the least significant bit to the most
significant bit.

## Standard library
The Nandlang standard library defines many functions, mostly for I/O.

### putb
Put a single boolean value into standard output.

```Javascript
putb(1);
// prints 1
```

### puti8
Put an 8-bit integer value into standard output

```Javascript
puti8(0, 0, 1, 0, 0, 1, 1, 1);
// prints 39
```

### endl
Prints a newline character

```Javascript
endl();
// inserts a new line
```

### putc
Prints the given character

```Javascript
putc('B');
// prints B
```

### getc
Get a single character value from standard input as eight bit values.

```Javascript
putc(getc());
// User inputs a character that is then printed
```

### iogood
Returns 1 if standard input can be read from. This is useful for piping files
into standard input, as `iogood()` will resolve to false once the file can no
longer be read from. A user can also exit the program early by inserting an end
of file using ctrl+D on unix-based systems.

```Javascript
while iogood() {
    putc(getc());
}
// Prints anything that the user inputs until the standard input can no longer
// be read.
```

### malloc
Allocates memory. The input is a pointer sized number specifying the number of
bits to allocate in dynamic memory. It outputs a pointer pointing to a memory
location that can contain the given number of bits. A pointer should be freed
later.

```Javascript
var memory[ptr] = malloc(64[ptr]);
```

### assign
Assign a value to a pointer. It takes a pointer and a bit and assigns the
pointer to the value.

```Javascript
assign(memory, 1);
```

### deref
Dereferences a pointer. It takes a pointer and returns a bit indicating the bit
value at that pointer.

```Javascript
var value = deref(memory);
```

### free
Frees the memory at the given pointer. Should be used whenever memory allocated
with malloc is no longer needed or owned.

```Javascript
free(memory);
```

## Example Fibonacci program
Here is an example of a Fibonacci sequence generator. Note that even the most
basic operations, such as addition and subtraction, must be written from the
ground up.

The program should (correctly) output the following Fibonacci numbers
from 0 to 12 (anything larger wouldn't fit in an 8-bit integer):

```
Fib 0 = 0
Fib 1 = 1
Fib 2 = 1
Fib 3 = 2
Fib 4 = 3
Fib 5 = 5
Fib 6 = 8
Fib 7 = 13
Fib 8 = 21
Fib 9 = 34
Fib 10 = 55
Fib 11 = 89
Fib 12 = 144
Fib 13 = 233
```

And here is the program itself:

```Javascript
// not logic gate
function not(in : out) {
    out = in ! in;
}

// and logic gate
function and(a, b : out) {
    out = not(a ! b);
}

// or logic gate
function or(a, b : out) {
    out = not(a) ! not(b);
}

// xor logic gate
function xor(a, b : out) {
    out = or(and(a, not(b)), and(not(a), b));
}

// returns true if a and b are equal
function eq(a, b : out) {
    out = not(xor(a, b));
}

// full adder
function add(a, b, cin : v, cout) {
    v = xor(cin, xor(a, b));
    cout = or(and(a, b), and(xor(a, b), cin));
}

// 8 bit adder
function add8(a[8], b[8] : o[8]) {
    var c = 0;
    o[7], c = add(a[7], b[7], c);
    o[6], c = add(a[6], b[6], c);
    o[5], c = add(a[5], b[5], c);
    o[4], c = add(a[4], b[4], c);
    o[3], c = add(a[3], b[3], c);
    o[2], c = add(a[2], b[2], c);
    o[1], c = add(a[1], b[1], c);
    o[0], c = add(a[0], b[0], c);
}

// returns the two's complement of the given integer
function complement8(i[8] : o[8]) {
    o = add8(
        not(i[0]), not(i[1]), not(i[2]), not(i[3]),
        not(i[4]), not(i[5]), not(i[6]), not(i[7]),
        0, 0, 0, 0, 0, 0, 0, 1);
}

// 8 bit subtraction
function sub8(a[8], b[8] : o[8]) {
    o = add8(a, complement8(b));
}

// 8 bit equality
function equal8(a[8], b[8] : out) {
    out = and(
        and(and(eq(a[1], b[1]), eq(a[2], b[2])),
            and(eq(a[3], b[3]), eq(a[4], b[4]))),
        and(and(eq(a[5], b[5]), eq(a[6], b[6])),
            and(eq(a[7], b[7]), eq(a[0], b[0]))));
}

// returns the Fibonacci number for the given input
function fibonacci(i[8] : o[8]) {
    var check[7], _ = i;
    var is_equal = equal8(check,0, 0,0,0,0,0,0,0,0);
    if is_equal {
        // return input if equal to 1 or 0
        o = i;
    }
    if not(is_equal) {
        // o = fibonacci(i - 1) + fibonacci(i - 2);
        o = add8(
            fibonacci(sub8(i, 0,0,0,0,0,0,0,1)),
            fibonacci(sub8(i, 0,0,0,0,0,0,1,0))
        );
    }
}

function main()
{
    var value[8] = 0,0,0,0,0,0,0,0;
    while not(equal8(value, 0,0,0,0,1,1,1,0)) {
        // to output strings multiple individual putc calls are needed
        putc('F');
        putc('i');
        putc('b');
        putc(' ');
        puti8(value);
        putc(' ');
        putc('=');
        putc(' ');
        puti8(fibonacci(value));
        endl();
        // increment
        value = add8(value, 0,0,0,0,0,0,0,1);
    }
}
```

## Other examples
More examples can be found in the [examples subfolder](./example).
