CFLAGS ?= -W -Wall -Werror -Wno-deprecated -I.. -g -fsanitize=address,undefined -coverage -lm $(EXTRA)
CWD ?= $(realpath $(CURDIR))
ROOT ?= $(CWD)
DOCKER = docker run $(DA) --rm -e WINEDEBUG=-all -v $(ROOT):$(ROOT) -w $(CWD)

all: test cpp vc98 vc2017

test:
	$(CC) $(CFLAGS) unit_test.c -o ut
	./ut

cpp:
	$(CXX) $(CFLAGS) unit_test.c -o ut
	./ut

vc98:
	$(DOCKER) mdashnet/vc98 wine cl /nologo /W3 /O2 /I. unit_test.c /Feut.exe
	$(DOCKER) mdashnet/vc98 wine ut.exe

vc2017:
	$(DOCKER) mdashnet/vc2017 wine64 cl /nologo /W3 /O2 /I. unit_test.c /Feut.exe
	$(DOCKER) mdashnet/vc2017 wine64 ut.exe

clean:
	rm -rf unit_test ut *.exe *.o *.obj *.gc*
