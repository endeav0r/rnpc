import sys

lower_chars = 'abcdefghijklmnopqrstuvwxyz'
upper_chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
replace_chars       = '4@3!1!105$7'
replace_chars_ascii = 'aaeiillosst'

fh = open(sys.argv[1], 'r')
lines = fh.readlines()
fh.close()

def dowork (base, all, i) :
	if i == len(all) :
		print base
		return
	if all[i] in lower_chars :
		#print base + all[i]
		dowork(base + all[i], all, i+1)
		for j in range(len(replace_chars)) :
			if all[i] == replace_chars[j] :
				#print base + replace_chars_ascii[j]
				dowork(base + replace_chars_ascii[j], all, i + 1)
	elif all[i] in upper_chars :
		#print base + lower_chars[upper_chars.index(all[i])]
		dowork(base + lower_chars[upper_chars.index(all[i])], all, i + 1)
		lower_char = lower_chars[upper_chars.index(all[i])]
		for j in range(len(replace_chars)) :
			if lower_char == replace_chars[j] :
				#print base + replace_chars_ascii[j]
				dowork(base + replace_chars_ascii[j], all, i + 1)

for line in lines :
	line = line.strip()
	dowork('', line, 0)
