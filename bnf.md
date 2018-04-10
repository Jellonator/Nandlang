Complete syntax for Nandlang:

```BNF
expr       ::= nandexpr | funccall | literal | identifier
nandexpr   ::= expr '!' expr
funccall   ::= identifier '(' [exprlist] ')'
literal    ::= 0 | 1 | character
character  ::= ''' printable character ''' | ''' '\' escape '''
escape     ::= '0' | 't' | 'n' | 'v' | 'f' | 'r'
exprlist   ::= expr { ',' expr}
idlist     ::= identifier { ',' idlist }
funcparam  ::= [idlist] [':' [idlist]]
funcdef    ::= 'function' '(' funcparam ')' block
block      ::= '{' {statement} '}'
statement  ::= ifstmt | whilestmt | varstmt | expr ';'
ifstmt     ::= 'if' expr block {'else' block}
whilestmt  ::= 'while' expr block
varstmt    ::= {'var'} idlist '=' exprlist ';'
identifier ::= (alphabetic character | '_') {alphanumeric character | '_'}
```

alphabetic character is any character that is true for std::isalpha,
alphanumeric character is any character that is true for std::isalpha or
std::isdigit, and printable character is any character that is true for
std::isprint.
