# -*- Makefile -*-

# makefile embeds dependcy tree
# recipe

# target: dependencies
# 	action						

CC := gcc
CFLAGS_RELEASE := -Wall -Wextra  -O2 -std=c11
CFLAGS_DEBUG   := -Wall -Wextra  -O0 -g3 -std=c11

LIBSODIUM_DIR := libsodium-stable
LIBSODIUM_CONFIGURE := ./configure \
      --enable-static \
      --disable-shared \
      --prefix=$(abspath $(LIBSODIUM_DIR)/build) \
      CFLAGS="-Os -fPIC"

all: macabee

macabee: CFLAGS := $(CFLAGS_RELEASE)
macabee: mongoose.o sqlite3.o main.o $(LIBSODIUM_DIR)/build/lib/libsodium.a
	$(CC) -o $@ $^ -I$(LIBSODIUM_DIR)/build/include


macabee-debug: CFLAGS := $(CFLAGS_DEBUG)
macabee-debug: mongoose.o sqlite3.o main.o $(LIBSODIUM_DIR)/build/lib/libsodium.a
	$(CC) -o $@ $^ -I$(LIBSODIUM_DIR)/build/include


$(LIBSODIUM_DIR)/build/lib/libsodium.a:
	@echo "=== Building Libsodium ==="
	cd $(LIBSODIUM_DIR) && $(LIBSODIUM_CONFIGURE) && make -j$(nproc) && make install

mongoose.o: mongoose.c
	$(CC) $(CFLAGS) -c $< -o $@

sqlite3.o: sqlite3.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c 
	$(CC) $(CFLAGS) -c $< -o $@

debug: macabee-debug

clean:
	rm -f *.o macabee macabee-debug




