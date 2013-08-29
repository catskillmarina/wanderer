# Makefile for wanderer - modified by Bill Randle 6/30/88
# modified again by play@cwi.nl
# and again by me.. maujp@uk.ac.warwick.cu
# and yet again by adb@bucsf.bu.edu
# Make clean - Marina Brown marina@surferz.net 2001,2

OBJ = monsters.o m.o save.o jump.o display.o icon.o game.o read.o help.o fall.o scores.o edit.o encrypt.o

#CFLAGS = -O -s
#CFLAGS = -g
LIBS = -lcurses
#CC = cc

all:	wanderer
	@echo DONE

wanderer:	$(OBJ)
	$(CC) $(CFLAGS) -o wanderer $(OBJ) $(LIBS)

convert: convert.c wand_head.h
	$(CC) $(CFLAGS) -o convert convert.c

$(OBJ): wand_head.h

install: 
	install -m 0755 -d -o root -g games ${DESTDIR}/usr/local/share/wanderer
	install -m 0755 -d -o root -g games ${DESTDIR}/usr/local/share/wanderer/screens
	install -m 0644 -o root -g games ./screens/* ${DESTDIR}/usr/local/share/wanderer/screens/
	install -m 2755 -o root -g games ./wanderer ${DESTDIR}/usr/local/bin/
	install -m 0644 -o root -g games wanderer.6 ${DESTDIR}/usr/local/man/man6/

clean:
	rm *.o

distclean:
	rm *.o
	rm wanderer
	rm convert

#uninstall:
#	rm /usr/games/wanderer
#	rm -rf /usr/games/wandscreens
#	rm /var/games/wandererscores
