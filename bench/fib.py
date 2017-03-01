#!/usr/bin/python
base1 = 0
base2 = 1
nth = 70
temp = 0

while nth > 0:
    nth -= 1
    temp = base2
    base2 = base1 + base2
    base1 = temp

print(base1)
