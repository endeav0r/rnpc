numbers = ['one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten']

for number1 in numbers :
	print number1
	for number2 in numbers :
		n = number1 + number2
		print n
		for number3 in numbers :
			n2 = n+ number3
			print n2
