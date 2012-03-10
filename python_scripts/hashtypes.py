import hashlib

	
def NT (plaintext) :
	u = ''
	for char in plaintext :
		u += char + chr(0)
	h = hashlib.new('md4')
	h.update(u)
	return h.hexdigest()

def mscache (username, password) :

	def unicodify (string) :
		u = ''
		for char in string :
			u += char + chr(0)
		return u

	def md4 (plaintext) :
		h = hashlib.new('md4')
		h.update(plaintext)
		return h.digest()
	
	h = hashlib.new('md4')
	h.update(md4(unicodify(password)) + unicodify(username.lower()))
	return h.hexdigest()


def phpBB3 (phpbb3_hash, plaintext) :

	itoa64 = './0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'

	def md5 (string) :
		h = hashlib.new('md5')
		h.update(string)
		return h.digest()

	def phpbb3_base64 (string, count) :
		result = ''
		i = 0
		while True :
			value = ord(string[i])
			i += 1
			result += itoa64[value & 0x3f]
			if i < count :
				value |= ord(string[i]) << 8
			result += itoa64[(value >> 6) & 0x3f]
		
			if i >= count :
				break
			i += 1
		
			if i < count :
				value |= ord(string[i]) << 16
		
			result += itoa64[(value >> 12) & 0x3f]
		
			if i >= count :
				break
			i+= 1
		
			result += itoa64[(value >> 18) & 0x3f]
		
			if i >= count :
				break

		return result

	count_log2 = itoa64.index(phpbb3_hash[3])
	
	if count_log2 < 7 or count_log2 > 30 :
		return False

	count = 1 << count_log2
	salt = phpbb3_hash[4:12]

	h = md5(salt + plaintext)

	while count > 0 :
		h = md5(h + plaintext)
		count -= 1

	result = phpbb3_hash[0:12]
	result += phpbb3_base64(h, 16)

	return result
