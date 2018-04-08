# NANDLANG
Nandlang version 1.0

A programming language based on NAND completeness.

Created by James Beedie

## About
Nandlang is a programming language that is based on the idea of NAND
completeness. All common operations and logic gates can be derived from the
NAND gate. As such, the only operators in this programming language are the NAND
operator, function calls, and variable assignment. Nandlang is an imperative
programming language.

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

## Basic syntax
Like many programming languages, Nandlang borrows a lot of its syntax from C;
however, there are still many differences. This section will go over the basic
syntax of Nandlang.

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
    if bar() {} // bar has too many outputss
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
    v = xor(cin, xor(a, b));
    cout = or(and(a, b), and(xor(a, b), cin));
}

function main() {
    var value, _ = add(1, 1, 0);
    putb(value);
    // putb(_) will cause a compilation error, since _ can't be used as a real
    // identifier.
}
```

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
function add8(
    a1, a2, a3, a4, a5, a6, a7, a8,
    b1, b2, b3, b4, b5, b6, b7, b8
  : o1, o2, o3, o4, o5, o6, o7, o8) {
    var c = 0;
    o8, c = add(a8, b8, c);
    o7, c = add(a7, b7, c);
    o6, c = add(a6, b6, c);
    o5, c = add(a5, b5, c);
    o4, c = add(a4, b4, c);
    o3, c = add(a3, b3, c);
    o2, c = add(a2, b2, c);
    o1, c = add(a1, b1, c);
}

// returns the two's complement of the given integer
function complement8(
    i1, i2, i3, i4, i5, i6, i7, i8
  : o1, o2, o3, o4, o5, o6, o7, o8) {
    o1, o2, o3, o4, o5, o6, o7, o8 = add8(
    not(i1), not(i2), not(i3), not(i4), not(i5), not(i6), not(i7), not(i8),
    0, 0, 0, 0, 0, 0, 0, 1);
}

// 8 bit subtraction
function sub8(
    a1, a2, a3, a4, a5, a6, a7, a8,
    b1, b2, b3, b4, b5, b6, b7, b8
  : o1, o2, o3, o4, o5, o6, o7, o8) {
    o1, o2, o3, o4, o5, o6, o7, o8 = add8(
    a1, a2, a3, a4, a5, a6, a7, a8,
    complement8(b1, b2, b3, b4, b5, b6, b7, b8));
}

// 8 bit equality
function equal8(
    a1, a2, a3, a4, a5, a6, a7, a8,
    b1, b2, b3, b4, b5, b6, b7, b8
  : out) {
    out = and(
        and(and(eq(a1, b1), eq(a2, b2)), and(eq(a3, b3), eq(a4, b4))),
        and(and(eq(a5, b5), eq(a6, b6)), and(eq(a7, b7), eq(a8, b8))));
}

// returns the Fibonacci number for the given input
function fibonacci(
    i1, i2, i3, i4, i5, i6, i7, i8
  : o1, o2, o3, o4, o5, o6, o7, o8) {
    var is_equal = equal8(i1, i2, i3, i4, i5, i6, i7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if is_equal {
        // return input if equal to 1 or 0
        o1 = i1;
        o2 = i2;
        o3 = i3;
        o4 = i4;
        o5 = i5;
        o6 = i6;
        o7 = i7;
        o8 = i8;
    }
    if not(is_equal) {
        // o = fibonacci(i - 1) + fibonacci(i - 2);
        o1, o2, o3, o4, o5, o6, o7, o8 = add8(
            fibonacci(sub8(i1, i2, i3, i4, i5, i6, i7, i8, 0, 0, 0, 0, 0, 0, 0, 1)),
            fibonacci(sub8(i1, i2, i3, i4, i5, i6, i7, i8, 0, 0, 0, 0, 0, 0, 1, 0))
        );
    }
}

function main()
{
    var v1, v2, v3, v4, v5, v6, v7, v8 = 0, 0, 0, 0, 0, 0, 0, 0;
    while not(equal8(v1, v2, v3, v4, v5, v6, v7, v8, 0, 0, 0, 0, 1, 1, 1, 0)) {
        // to output strings multiple individual putc calls are needed
        putc('F');
        putc('i');
        putc('b');
        putc(' ');
        puti8(v1, v2, v3, v4, v5, v6, v7, v8);
        putc(' ');
        putc('=');
        putc(' ');
        puti8(fibonacci(v1, v2, v3, v4, v5, v6, v7, v8));
        endl();
        // increment
        v1, v2, v3, v4, v5, v6, v7, v8 = add8(
            v1, v2, v3, v4, v5, v6, v7, v8, 0, 0, 0, 0, 0, 0, 0, 1);
    }
}
```

## Other examples
More examples can be found in the "examples" subfolder.
