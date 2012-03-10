import hashlib
import sys

file_in = sys.argv[1]
file_out = sys.argv[2]
print 'file_in : ' + file_in
print 'file_out: ' + file_out

fh_in = open(file_in, 'r')
fh_out = open(file_out, 'w')

for word in fh_in.xreadlines() :
	word = word.strip()
	try :
		h = hashlib.new('md5')
		h.update(word)
		fh_out.write(h.hexdigest() + '\n')
	except :
		print 'error with word: ' + word

fh_in.close()
fh_out.close()

