CC = clang
CFLAGS = -Wall -Wextra -O3 -I./src

.PHONY: all
all: bin/qrcg \
     bin/test_analysis \
     bin/test_encoding \
     bin/test_eccoding \
     bin/test_message \
     bin/test_masking

bin/qrcg: bin/encode.o bin/gf256.o bin/message.o bin/module.o bin/mask.o bin/image.o bin/main.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_analysis: bin/encode.o bin/test_analysis.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_encoding: bin/encode.o bin/test_encoding.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_eccoding: bin/gf256.o bin/test_eccoding.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_message: bin/message.o bin/test_message.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_masking: bin/mask.o bin/test_masking.o
	${CC} $(LDFLAGS) -o $@ $^

bin/%.o: src/%.c
	${CC} ${CFLAGS} -c $< -o $@

bin/%.o: test/%.c
	${CC} ${CFLAGS} -c $< -o $@

.PHONY: clean
clean:
	rm -f ./bin/*
