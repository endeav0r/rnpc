import sys

fh1 = open(sys.argv[1], 'r')
fh2 = open(sys.argv[2], 'r')

lines1 = fh1.readlines()
for i in range(len(lines1)) :
	lines1[i] = lines1[i].split('\t')[0]

lines2 = fh2.readlines()
for i in range(len(lines2)) :
	lines2[i] = lines2[i].strip()
	
print 'not found'

for line2 in lines2 :
	if line2 not in lines1 :
		print line2

print 'false positives'

for line1 in lines1 :
	if line1 not in lines2 :
		print line1

fh1.close()
fh2.close()
