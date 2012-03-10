import sys
import random
import hashtypes

print 'make_salted_passwords.py <salts_file> <passwords_file>'

fhs = open(sys.argv[1], 'r')
fhp = open(sys.argv[2], 'r')
passwords = fhp.readlines()
fhp.close()

plaintexts = []

for salt in fhs.xreadlines() :
	salt = salt.strip()
	password = passwords[int(random.random() * len(passwords)) % len(passwords)]
	password = password.strip()
	
	print salt + ':' + hashtypes.mscache(salt, password)
	plaintexts.append(salt + password)
