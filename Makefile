#
# MAKEFILE FOR SIRCOND
#
CPPFLAGS = -Wall -g -std=c++11

.cpp.o:
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

sircond:	sircond.o log.o sircon.o timetrax.o serial_unix.o \
	ctimer.o ctask.o client.o server.o sirclient.o sirserver.o \
	sobuf.o util.o scevents.o
	$(CXX) -o sircond sircond.o sircon.o log.o timetrax.o \
	serial_unix.o ctimer.o ctask.o client.o server.o sirclient.o \
	sirserver.o sobuf.o util.o scevents.o -pthread

clean:
	rm *.o sircond

install:	sircond
	cp -f sircond /usr/local/bin


