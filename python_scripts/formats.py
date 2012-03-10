import sys

try :
	import psyco
	psyco.full()
except :
	print 'no psyco'

UPPERS   = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
LOWERS   = 'abcdefghijklmnopqrstuvwxyz'
DIGITS   = '0123456789'
SPECIALS = '!$@#%^&*?.+\\-_=`~()|'

formats = {}

fh = open(sys.argv[1], 'r')

print 'reading formats'
formats_read = 0

for password in fh.xreadlines() :
	pattern = ''
	for i in range(len(password)) :
		if password[i] in UPPERS :
			pattern += 'U'
		elif password[i] in LOWERS :
			pattern += 'L'
		elif password[i] in DIGITS :
			pattern += 'D'
		elif password[i] in SPECIALS :
			pattern += 'S'
	if formats.has_key(pattern) :
		formats[pattern] += 1
	else :
		formats[pattern] = 1
	formats_read += 1
	if formats_read % 1000000 == 0 :
		print str(formats_read) + ' formats read'

print 'sorting'
formats_sorted = sorted(formats.items(), key=lambda(k,v):(v,k))
formats_sorted.reverse()

for format in formats_sorted :
	print format[1], format[0]

