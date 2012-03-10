import sys

fh = open(sys.argv[1], 'r')

words = fh.readlines()

fh.close()

for w1 in words :
	for w2 in words :
		w1 = w1.strip()
		w2 = w2.strip()
		w = w1 + w2
		print w
