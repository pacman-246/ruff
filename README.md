# ruff

## 目標
構文を単純化させた関数型言語

## コード例
Hello world! 
```
println -> "Hello world!";
```

fizzbuzz 
```
let -> $fizzbuzz -> (
    fn -> $x
    -> $(
        if
        -> (equal -> (mod -> x -> 15) -> 0)
        -> "fizzbuzz"
        -> (
            if
            -> (equal -> (mod -> x -> 3) -> 0)
            -> "fizz"
            -> (
                if
                -> (equal -> (mod -> x -> 5) -> 0)
                -> "buzz"
                -> (toString -> x)
            )
        )
    )
);

foreach -> println -> (map -> fizzbuzz -> (range -> 1 -> 100));
```