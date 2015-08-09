SOURCES = syncReaders.cc syncClass.cc syncClass.hpp syncDbClass.cc \
syncDbClass.hpp jlog.cc jlog.hpp
OBJS = syncReaders.o syncClass.o syncDbClass.o jlog.o
LIBS = -lsqlite3
EXEC = syncReaders
CC = g++

CCFLAGS = -g  -Wall
docs = docs/html/index.html
pdf = docs/latex/refman.pdf

.PHONY : all clean

all : $(EXEC)

docs : $(docs)
pdf : $(pdf)

syncReaders.o : syncReaders.cc syncClass.o syncClass.hpp jlog.hpp
syncClass.o : syncClass.cc syncClass.hpp jlog.hpp
syncDbClass.o : syncDbClass.cc syncDbClass.hpp jlog.hpp
jlog.o : jlog.cc jlog.hpp

$(EXEC) : $(OBJS)
	$(CC) $(CCFLAGS) $(LIBS) -o $@ $^ 

.cc.o :
	$(CC) -c $(CCFLAGS) -o $@ $< 
.o.hpp :
	$(CC) -c $(CCFLAGS) -o $@ $< 

$(docs) : $(SOURCES) docs.cfg
	doxygen docs.cfg

$(pdf) : $(SOURCES) docs.cfg docs
	make -C docs/latex

clean :
	rm -f $(EXEC) *.o
	rm -rf ./docs/html
	rm -rf ./docs/latex
