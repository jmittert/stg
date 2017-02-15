# STG

Implements the STG as specified by https://www.dcc.fc.up.pt/~pbv/aulas/linguagens/peytonjones92implementing.pdf

## Example Code
Consider
```
add3 = {} @n {x} -> + {x 3}
main = {} @n {} -> add3 {3}
```

All STG programs consist of top level global bindings, one of which is the main
function which takes zero arguments and is executed first.

Every binding binds a name to a closure, where a closure species it's free
variables, if it's updatable, it's parameters and an expression which it
evaluates to in that order.

```
map = {} @n {f xs} ->
  case xs{} of {
    Nil {} -> Nil {}
    Cons {y ys} -> let fy = {f y} @u {} -> f {y}
                       mfy = {f ys} @u {} -> map {f ys}
                   in Cons {fy mfy}
  }
sum = {} @n {xs} ->
  case xs{} of {
    Nil {} -> Int {0}
    Cons {y ys} -> let sys = {ys} @u {} -> sum {ys}
                   in case sys{} of {
                       Int {x1} -> case y {} of {
                           Int {x2} -> case' +{x1 x2} of {
                               t -> Int {t}
                           }
                       }
                   }
    }

add3 = {} @n {x} -> case x{} of {
    Int {x1} -> case' + {x1 3} of {
        t -> Int {t}
    }
    }

main = {} @n {} ->
    letrec empty = {} @u {} -> Nil {}
           three = {} @u {} -> Int {3}
           lst = {} @u {} -> Cons {three empty}
           mapped = {lst} @u {} -> map {add3 lst}
           in sum {mapped}
```
In this more complex example, we demonstrate case and patter matching, let
bindings, letrec bindings and higher order functions

Running adds 3 to each element in [3] and sums the result, giving 6. This is
appropriately equivalent to the Haskell code `sum $ map (+3) [3]`.


## Build
Standard cmake
```
mkdir build
cd build
cmake ..
make

# Optionally run the tests
make test
```
