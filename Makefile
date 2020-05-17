
CC=g++
NAME=src/icp
BASEFILES=src/
ZIPNAME=xkoprd00-xmudry01.zip
ZIPFILES=doc/ examples/ src/ Makefile README.txt
CXXFLAGS= -std=c++17 -Wall -Wextra

all: 
	qmake src/icp.pro -o src/Makefile
	$(MAKE) CXX=$(CC) -C src/ -o $(NAME)

run: all
	src/icp

doxygen:
	doxygen doc/Doxyfile
	
pack:
	zip -r $(ZIPNAME) $(ZIPFILES) -x src/.git\* src/\*.pro.\*

clean:
	rm -rf icp src/*.o src/Makefile src/icp src/moc_* src/ui_mainwindow.h doc/html doc/latex