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

ifndef BUILDDIR
ifdef DEBUG
BUILDDIR = mingw-debug
else
BUILDDIR = mingw-release
endif
endif

TARGET = VirtualDimension.exe HookDLL.dll
SRC_FILE = ConfigBox.cpp Desktop.cpp DesktopManager.cpp HotKeyManager.cpp Settings.cpp \
VirtualDimension.cpp deskPropsDlg.cpp stdafx.cpp Transparency.cpp AlwaysOnTop.cpp \
TrayIcon.cpp ShellHook.cpp WindowsManager.cpp Window.cpp movewindow.cpp ToolTip.cpp \
FastWindow.cpp TrayIconsManager.cpp WindowDialogs.cpp HotKeyControl.cpp \
OnScreenDisplay.cpp PlatformHelper.cpp SubclassWindow.cpp WindowsList.cpp  \
WallPaper.c BackgroundDisplayMode.cpp guids.c
RES_FILE = VirtualDimension.res
OBJ_FILE_TMP = $(SRC_FILE:cpp=o)
OBJ_FILE = $(OBJ_FILE_TMP:c=o) libtransp.a

ifdef DEBUG
CXXFLAGS = -g
CFLAGS = -g
else
CXXFLAGS = -fexpensive-optimizations -O3
CFLAGS = -fexpensive-optimizations -O3
endif

MAKEDEPEND = g++ -MM $(CPPFLAGS) -o $*.d $<


.PHONY: all recall clean pre_comp

VPATH = ..

ifdef INCDEP
DEP_FILE_TMP = $(SRC_FILE:cpp=P)
DEP_FILE = $(DEP_FILE_TMP:c=P)
-include $(DEP_FILE)
endif

all: pre_comp
	@cd $(BUILDDIR); make allrec -f ../Makefile INCDEP=1

allrec: $(TARGET)

clean:
	@if ( [ -d ${BUILDDIR} ] ) then \
	   echo rm -r ${BUILDDIR};      \
	   rm -r ${BUILDDIR};           \
	fi

pre_comp:
	@if (! [ -d ${BUILDDIR} ] ) then   \
	   echo mkdir ${BUILDDIR};         \
	   mkdir ${BUILDDIR};              \
	fi

VirtualDimension.exe: ${OBJ_FILE} ${RES_FILE}
	g++ $^ -o $@ -mwindows -lcomctl32 -lole32 -lolepro32 -lmsimg32 $(CXXFLAGS)
ifndef DEBUG
	strip $@
endif

HookDLL.dll: HookDLL.o
	g++ -shared -o $@ $^ $(CXXFLAGS)
ifndef DEBUG
	strip $@
endif

libtransp.a: Transp.def
	dlltool --def $< --dllname user32.dll  --output-lib $@

VirtualDimension.res: VirtualDimension.rc
	windres -i $< -I rc -o $@ -O coff --include-dir=..

%.o: %.cpp
	@-$(MAKEDEPEND); \
            cp $*.d $*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
            rm -f $*.d
	g++ -c -o $@ $< $(CXXFLAGS)

%.o: %.c
	@-$(MAKEDEPEND); \
            cp $*.d $*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
            rm -f $*.d
	gcc -c -o $@ $< $(CFLAGS)
