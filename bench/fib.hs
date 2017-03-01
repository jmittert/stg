fib :: Int -> Int -> Int -> Int
fib a b 0 = a
fib a b n = fib b (a + b) (n-1)

main = print $ fib 0 1 70
