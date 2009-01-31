.SILENT:

all:
	cat !list | \
	awk '{ printf "printf \"%%10s     %%-20s\\n\" LINK %s;\n\
	g++ -L../testlib -L../testlib++ -I../testlib -I../testlib++ -ltestlib -ltestlib++ -o %s %s.cxx;\n",$$1,$$1,$$1; }' > tmp.sh;
	chmod 0755 tmp.sh; ./tmp.sh; rm -f tmp.sh

depend:	

configure:	

install:

clean:
	cat !list | \
	awk '{ printf "rm -f %s\n",$$1; }' > tmp.sh;
	chmod 0755 tmp.sh; ./tmp.sh;
	rm -f *.o

distclean:
	cat !list | \
	awk '{ printf "rm -f %s\n",$$1; }' > tmp.sh;
	chmod 0755 tmp.sh; ./tmp.sh;
	rm -f *.o
