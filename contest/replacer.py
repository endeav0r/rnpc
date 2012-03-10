lower = 'abcdefghijklmnopqrstuvwxyz'

import sys

fh = open(sys.argv[1], 'r')
words = fh.readlines()
fh.close()

for word in words :
	word = word.strip()
	for i in range(len(word)) :
		for c in lower :
			if c != word[i] :
				new_word = word[:i] + c + word[i+1:]
				print new_word
