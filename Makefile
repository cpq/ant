CFLAGS ?= -W -Wall -Werror -Wno-deprecated -I.. -g -fsanitize=address,undefined -coverage $(EXTRA)
CWD ?= $(realpath $(CURDIR))
ROOT ?= $(CWD)
DOCKER = docker run $(DA) --rm -e WINEDEBUG=-all -v $(ROOT):$(ROOT) -w $(CWD)

all: test cpp vc98 vc2017 mingw

test:
	$(CC) $(CFLAGS) unit_test.c -o ut
	$(RUN) ./ut

cpp:
	$(CXX) $(CFLAGS) unit_test.c -o ut
	$(RUN) ./ut

vc98:
	$(DOCKER) mdashnet/vc98 wine cl /nologo /W3 /O2 /I. unit_test.c /Feut.exe
	$(DOCKER) mdashnet/vc98 wine ut.exe

vc2017:
	$(DOCKER) mdashnet/vc2017 wine64 cl /nologo /W3 /O2 /I. unit_test.c /Feut.exe
	$(DOCKER) mdashnet/vc2017 wine64 ut.exe

mingw:
	$(DOCKER) mdashnet/mingw i686-w64-mingw32-gcc -W -Wall $(EXTRA) unit_test.c -o ut.exe
	$(DOCKER) mdashnet/mingw wine ut.exe

coverage: test
	gcov -l -n *.gcno | sed '/^$$/d' | sed 'N;s/\n/ /'

upload-coverage: coverage
	curl -s https://codecov.io/bash | /bin/bash

clean:
	rm -rf unit_test ut *.exe *.o *.obj *.gc*
