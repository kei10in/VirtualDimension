# 
# Virtual Dimension -  a free, fast, and feature-full virtual desktop manager 
# for the Microsoft Windows platform.
# Copyright (C) 2003 Francois Ferrand
#
# This program is free software; you can redistribute it and/or modify it under 
# the terms of the GNU General Public License as published by the Free Software 
# Foundation; either version 2 of the License, or (at your option) any later 
# version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with 
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
# Place, Suite 330, Boston, MA 02111-1307 USA
#

TARGET = VirtualDimension.exe
BUILDDIR = mingw
SRC_FILE = ConfigBox.cpp Desktop.cpp DesktopManager.cpp HotKeyManager.cpp Settings.cpp \
VirtualDimension.cpp deskPropsDlg.cpp stdafx.cpp Transparency.cpp AlwaysOnTop.cpp \
TrayIcon.cpp ShellHook.cpp WindowsManager.cpp Window.cpp movewindow.cpp guids.c
RES_FILE = VirtualDimension.res
OBJ_FILE_TMP = $(SRC_FILE:cpp=o)
OBJ_FILE = $(OBJ_FILE_TMP:c=o) libtransp.a

CXXFLAGS = -fexpensive-optimizations -O3
CFLAGS = -fexpensive-optimizations -O3


.PHONY: all clean precomp


vpath %.o ${BUILDDIR}
vpath %.a ${BUILDDIR}
vpath %.res ${BUILDDIR}

all: pre_comp ${TARGET}

clean:
	-rm -r ${BUILDDIR}
	-rm $(TARGET)

pre_comp:
	-mkdir ${BUILDDIR}

VirtualDimension.exe: ${OBJ_FILE} ${RES_FILE}
	g++ $^ -o $@ -mwindows -lcomctl32 -lole32 $(CXXFLAGS)
	strip $@

libtransp.a: transp.def
	dlltool --def $< --dllname user32.dll  --output-lib ${BUILDDIR}/$@

VirtualDimension.res: VirtualDimension.rc
	windres -i $< -I rc -o ${BUILDDIR}/$@ -O coff 

%.o: %.cpp
	g++ -c -o ${BUILDDIR}/$@ $< $(CXXFLAGS)

%.o: %.c
	gcc -c -o ${BUILDDIR}/$@ $< $(CFLAGS)
