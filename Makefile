C_FLAGS = -Wall -static -O5
CC = gcc

BUILD_DIR = o/
SRC_DIR = src/

all : rnpc


rnpc : brute crypt dict hashdb mangle md4 md5 miscc mscache nt salt sha1 ssha
	$(CC) $(C_FLAGS) -o rnpc $(SRC_DIR)rnpc.c \
	$(BUILD_DIR)md4.o $(BUILD_DIR)md5.o $(BUILD_DIR)nt.o $(BUILD_DIR)sha1.o \
	$(BUILD_DIR)dict.o $(BUILD_DIR)hashdb.o $(BUILD_DIR)mangle.o $(BUILD_DIR)misc.o \
	$(BUILD_DIR)salt.o $(BUILD_DIR)brute.o $(BUILD_DIR)mscache.o $(BUILD_DIR)crypt.o \
	$(BUILD_DIR)ssha.o -lcrypt -pthread



test_md5 : md5 misc
	$(CC) $(C_FLAGS) -o tests/test_md5 $(SRC_DIR)test_md5.c $(BUILD_DIR)md5.o \
	$(BUILD_DIR)misc.o

test_sha1 : sha1 misc
	$(CC) $(C_FLAGS) -o tests/test_sha1 $(SRC_DIR)test_sha1.c $(BUILD_DIR)sha1.o \
	$(BUILD_DIR)misc.o

test_dict : dict
	$(CC) $(C_FLAGS) -o tests/test_dict $(SRC_DIR)test_dict.c $(BUILD_DIR)dict.o

test_hashdb : hashdb
	$(CC) $(C_FLAGS) -o tests/test_hashdb $(SRC_DIR)test_hashdb.c \
	$(BUILD_DIR)hashdb.o $(BUILD_DIR)md5.o $(BUILD_DIR)misc.o

test_mangle : mangle
	$(CC) $(C_FLAGS) -o tests/test_mangle $(SRC_DIR)test_mangle.c \
	$(BUILD_DIR)mangle.o

test_vokram : vokram dict
	$(CC) $(C_FLAGS) -o tests/test_vokram $(SRC_DIR)test_vokram.c \
	$(BUILD_DIR)vokram.o $(BUILD_DIR)dict.o

test_crypt : crypt
	$(CC) $(C_FLAGS) -o tests/test_crypt $(SRC_DIR)test_crypt.c $(BUILD_DIR)crypt.o \
	 -lcrypt

test_mscache : mscache md4 miscc nt
	$(CC) $(C_FLAGS) -o tests/test_mscache $(SRC_DIR)test_mscache.c \
	$(BUILD_DIR)mscache.o $(BUILD_DIR)md4.o $(BUILD_DIR)misc.o $(BUILD_DIR)nt.o \

test_ssha : sha1 ssha miscc hashdb
	$(CC) $(C_FLAGS) -o tests/test_ssha $(SRC_DIR)test_ssha.c -pthread -g \
	$(BUILD_DIR)sha1.o $(BUILD_DIR)ssha.o $(BUILD_DIR)misc.o $(BUILD_DIR)hashdb.o

	

brute : $(SRC_DIR)brute.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)brute.c -o $(BUILD_DIR)brute.o

crypt :$(SRC_DIR)crypt.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)crypt.c -o $(BUILD_DIR)crypt.o

dict : $(SRC_DIR)dict.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)dict.c -o $(BUILD_DIR)dict.o

hashdb : $(SRC_DIR)hashdb.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)hashdb.c -o $(BUILD_DIR)hashdb.o

mangle : $(SRC_DIR)mangle.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)mangle.c -o $(BUILD_DIR)mangle.o

md4 : $(SRC_DIR)md4.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)md4.c -o $(BUILD_DIR)md4.o

md5 : $(SRC_DIR)md5.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)md5.c -o $(BUILD_DIR)md5.o

miscc : $(SRC_DIR)misc.o
	$(CC) $(C_FLAGS) -c $(SRC_DIR)misc.c -o $(BUILD_DIR)misc.o

mscache : $(SRC_DIR)mscache.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)mscache.c -o $(BUILD_DIR)mscache.o

nt : $(SRC_DIR)nt.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)nt.c -o $(BUILD_DIR)nt.o

salt : $(SRC_DIR)salt.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)salt.c -o $(BUILD_DIR)salt.o

sha1 : $(SRC_DIR)sha1.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)sha1.c -o $(BUILD_DIR)sha1.o

ssha : $(SRC_DIR)ssha.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)ssha.c -o $(BUILD_DIR)ssha.o

vokram : $(SRC_DIR)vokram.c
	$(CC) $(C_FLAGS) -c $(SRC_DIR)vokram.c -o $(BUILD_DIR)vokram.o


clean :
	rm $(BUILD_DIR)*
	rm rnpc
	rm tests/*

