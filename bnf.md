Complete syntax for Nandlang in EBNF notation:

```EBNF
expr       ::= nandexpr | funccall | literal | variable
nandexpr   ::= expr '!' expr
funccall   ::= identifier '(' [exprlist] ')'
literal    ::= digit [index] | character
character  ::= '\'' printable character '\'' | '\'' '\\' escape '\''
escape     ::= '0' | 't' | 'n' | 'v' | 'f' | 'r'
exprlist   ::= expr { ',' expr}
idlist     ::= variable { ',' idlist }
funcparam  ::= [idlist] [':' [idlist]]
funcdef    ::= 'function' identifier '(' funcparam ')' block
block      ::= '{' {statement} '}'
statement  ::= ifstmt | whilestmt | varstmt | expr ';'
ifstmt     ::= 'if' expr block {'else' block}
whilestmt  ::= 'while' expr block
varstmt    ::= {'var'} idlist '=' exprlist ';'
forstmt    ::= 'for' '(' ':' variable {',' ':' variable } ')' block
identifier ::= (alphabetic character | '_') {alphanumeric character | '_'}
variable   ::= identifier [index]
index      ::= '[' digit { digit } ']' | '[' 'ptr' ']'
```

alphabetic character is any character that is true for std::isalpha, digit is
any character that is true for std::isdigit, alphanumeric character is any
character that is true for std::isalpha or std::isdigit, and printable
character is any character that is true for std::isprint. Refer to C++
documentation for more information about these functions.
