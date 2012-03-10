import smbpasswd
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
		h = smbpasswd.nthash('rainbowsandpwnies' + word)
		fh_out.write(h + '\n')
	except :
		print 'error with word: ' + word

fh_in.close()
fh_out.close()

