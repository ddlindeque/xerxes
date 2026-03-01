# Xerxes - math repository

## Basic syntx

### Syntax

```bison

DOCUMENT:
    "axiom": EXPRESSION // An axiom asserts that EXPRESSION will always be TRUE

EXPRESSION:
    identifier
  | EXPRESSION ':' TYPE
  | "identifier" "=>" EXPRESSION
  | identifier '(' EXPRESSIONS ')'

EXPRESSIONS:
    EXPRESSION
  | EXPRESSIONS ',' EXPRESSION

VARIABLE:
    identifier ':' TYPE

```

### Types

* `OBJECT`:  Any object. It must exist, i.e.: it cannot represent 'nothing'.
* `SET`: A collection of `OBJECT` items. A set can be empty, i.e.: it contains no objects. A set can have an infinite number of objects. A `SET` is also an `OBJECT`.
* `bool`: An `OBJECT` with only two possible values, `TRUE` and `FALSE`.

### Logical predicates

#### `or(x, y): bool->bool->bool`

#### Truth table
  | x | y | or(x, y)
  |-- |-- |--
  | F | F | F
  | F | T | T
  | T | F | T
  | T | T | T

* `and(x, y): bool->bool->bool`
  | x | y | and(x, y)
  |-- |-- |--
  | F | F | F
  | F | T | F
  | T | F | F
  | T | T | T
* `not(x): bool->bool`
  | x | not(x)
  |-- |--
  | F | T
  | T | F
* `if(x, y): bool->bool->bool`
  | x | y | if(x, y)
  |-- |-- |--
  | F | F | T
  | F | T | T
  | T | F | F
  | T | T | T
* `isMember(x, y): OBJECT->SET->bool`
  Return `TRUE` when the `OBJECT` `x` appear at least once in the `SET` `y`.
  Return `FALSE` when the `OBJECT` `x` does not appear in the `SET` `y`.
* 

### Defined

* `iff(x, y)): bool->bool->bool = and(if(x, y), if(y, x))`

# Axioms

## Zermelo-Fraenkel set theory

```
axiom "Axiom of extensionality": x => y => z => if(iff(isMember(z, x), isMember(z, y)), x=y)
```

```
I: SET->OBJECT->OBJECT := X => x => if(isMember(X), x)
```
