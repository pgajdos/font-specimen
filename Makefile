# 
# font-specimen
#
# Display Font Specimen
#
# Copyright (C) 2014 Petr Gajdos (pgajdos at suse)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

VERSION		 = 20141201
LIBRARY_NAME	 = font-specimen
LIBRARY_MAJOR    = 0
LIBRARY_VERSION  = 0.0.0
LIBRARY_LINK     = lib$(LIBRARY_NAME).so
LIBRARY_FILE     = lib$(LIBRARY_NAME).so.$(LIBRARY_VERSION)
LIBPNG_CFLAGS	 = $(shell pkg-config --cflags libpng)
LIBPNG_LIBS	 = $(shell pkg-config --libs libpng)
FT2_CFLAGS	 = $(shell pkg-config --cflags freetype2)
FT2_LIBS	 = $(shell pkg-config --libs freetype2)
HB_CFLAGS	 = $(shell pkg-config --cflags harfbuzz)
HB_LIBS		 = $(shell pkg-config --libs harfbuzz)
FC_CFLAGS	 = $(shell pkg-config --cflags fontconfig)
FC_LIBS		 = $(shell pkg-config --libs fontconfig)
MYCFLAGS	 = -DFONT_SPECIMEN_VERSION=$(VERSION) $(LIBPNG_CFLAGS) $(FT2_CFLAGS) $(HB_CFLAGS) $(FC_CFLAGS) -Wall -g
MYLIBS		 = $(FC_LIBS) $(LIBPNG_LIBS) $(FT2_LIBS) $(HB_LIBS)

OBJS		 = fc.o unicode.o hbz.o ft.o specimen.o img_png.o error.o
UNICODE_SOURCES  = blocks-map.txt blocks.sh blocks.txt Blocks.txt Scripts.txt sentences.txt SOURCES UnicodeData.txt unicode.txt 
UNICODE_SCRIPTS  = collections-map.sh collections.sh scripts-map.sh scripts.sh  unicode.sh

font-specimen:			font-specimen.c .libs/$(LIBRARY_FILE)
				gcc -L.libs $(MYCFLAGS) $(CFLAGS) $(MYLDFLAGS) $(LDLAGS) -o font-specimen font-specimen.c -l$(LIBRARY_NAME)
.libs/$(LIBRARY_FILE):		$(OBJS)
				mkdir -p .libs
				gcc $(MYCFLAGS) $(CFLAGS) -shared -Wl,-soname,${LIBRARY_LINK}.$(LIBRARY_MAJOR) -o .libs/$(LIBRARY_FILE) $(OBJS) $(MYLIBS)
				ln -sf $(LIBRARY_FILE) .libs/$(LIBRARY_LINK)
				ln -sf $(LIBRARY_FILE) .libs/$(LIBRARY_LINK).$(LIBRARY_MAJOR)
specimen.o:			specimen.c specimen.h unicode.h fc.h ft.h img_png.h error.h
				gcc -c -fPIC $(MYCFLAGS) $(CFLAGS) specimen.c
fc.o:				fc.c fc.h unicode.h error.h
				gcc -c -fPIC $(MYCFLAGS) $(CFLAGS) fc.c
unicode.o:			unicode.c unicode.h fc.h error.h unicode/scripts.txt unicode/scripts-map.txt unicode/blocks-map.txt
				gcc -c -fPIC $(MYCFLAGS) $(CFLAGS) unicode.c
unicode.h:			unicode/sentences.txt
				touch unicode.h
hbz.o:				hbz.c hbz.h error.h
				gcc -c -fPIC $(MYCFLAGS) $(CFLAGS) hbz.c
ft.o:				ft.c ft.h fc.h error.h
				gcc -c -fPIC $(MYCFLAGS) $(CFLAGS) ft.c
img_png.o:			img_png.c img_png.h error.h
				gcc -c -fPIC $(MYCFLAGS) $(CFLAGS) img_png.c
error.o:			error.c error.h
				gcc -c -fPIC $(MYCFLAGS) $(CFLAGS) error.c
unicode/scripts.txt:		unicode/Scripts.txt unicode/scripts.sh unicode/collections.sh
				cd unicode; cat Scripts.txt | sh scripts.sh > scripts.txt; sh collections.sh >> scripts.txt
unicode/scripts-map.txt:	unicode/Scripts.txt unicode/scripts-map.sh unicode/collections-map.sh
				cd unicode; cat Scripts.txt | sh scripts-map.sh > scripts-map.txt; cat Scripts.txt Blocks.txt | sh collections-map.sh >> scripts-map.txt;
unicode/blocks-map.txt:		unicode/Blocks.txt unicode/blocks.sh
				cd unicode; cat Blocks.txt | sh blocks.sh > blocks-map.txt


clean:
				rm -rf *.o font-specimen unicode/scripts.txt unicode/scripts-map.txt .libs

install:			font-specimen
				mkdir -p $(DESTDIR)/$(INCLUDEDIR)
				install -m 0644 specimen.h $(DESTDIR)/$(INCLUDEDIR)/font-specimen.h
				mkdir -p $(DESTDIR)/$(LIBDIR)
				install -m 0644 .libs/$(LIBRARY_FILE) $(DESTDIR)/$(LIBDIR)
				ln -sf $(LIBRARY_FILE) $(DESTDIR)/$(LIBDIR)/$(LIBRARY_LINK).${LIBRARY_MAJOR}
				ln -sf $(LIBRARY_FILE) $(DESTDIR)/$(LIBDIR)/$(LIBRARY_LINK)
				mkdir -p -m 0755 $(DESTDIR)/$(BINDIR)
				install -m 0755 font-specimen $(DESTDIR)/$(BINDIR)

release:			font-specimen
				mkdir -p $(LIBRARY_NAME)-$(VERSION)
				cp -r *.c *.h Makefile $(LIBRARY_NAME)-$(VERSION)
				mkdir -p $(LIBRARY_NAME)-$(VERSION)/unicode
				for f in $(UNICODE_SOURCES) $(UNICODE_SCRIPTS); do \
				  cp unicode/$$f $(LIBRARY_NAME)-$(VERSION)/unicode; \
				done
				cp ChangeLog INSTALL README.md $(LIBRARY_NAME)-$(VERSION)
				tar cvjf $(LIBRARY_NAME)-$(VERSION).tar.bz2 $(LIBRARY_NAME)-$(VERSION)
				rm -r $(LIBRARY_NAME)-$(VERSION)

