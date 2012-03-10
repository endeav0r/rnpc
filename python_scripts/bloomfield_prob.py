import math

m = 2**24
k = 16
n = 10000

prob = (1 - math.e ** ((k * -1) * n / m)) ** k

print prob
