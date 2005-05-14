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

TARGETS = $(BUILDDIR)/VirtualDimension.exe $(BUILDDIR)/HookDLL.dll $(BUILDDIR)/langEN.dll
SRC_FILE = ConfigBox.cpp Desktop.cpp DesktopManager.cpp HotKeyManager.cpp Settings.cpp \
VirtualDimension.cpp deskPropsDlg.cpp stdafx.cpp Transparency.cpp AlwaysOnTop.cpp \
TrayIcon.cpp ShellHook.cpp WindowsManager.cpp Window.cpp movewindow.cpp ToolTip.cpp \
FastWindow.cpp TrayIconsManager.cpp WindowDialogs.cpp HotKeyControl.cpp \
OnScreenDisplay.cpp PlatformHelper.cpp SubclassWindow.cpp WindowsList.cpp  \
WallPaper.cpp BackgroundDisplayMode.cpp BackgroundColor.cpp TaskPool.cpp \
LinkControl.cpp HotkeyConfig.cpp guids.c ExplorerWrapper.cpp HidingMethod.cpp \
SharedMenuBuffer.cpp MouseWarp.cpp Config.cpp ApplicationListDlg.cpp Locale.cpp
RES_FILE = $(BUILDDIR)/VirtualDimension.res
DEP_FILE = $(addprefix $(BUILDDIR)/,$(addsuffix .d,$(basename $(SRC_FILE))))
OBJ_FILE = $(DEP_FILE:.d=.o) $(BUILDDIR)/libtransp.a

ifdef DEBUG
CXXFLAGS = -g -O3 -DDEBUG
CFLAGS = -g -O3 -DDEBUG
else
CXXFLAGS = -fexpensive-optimizations -O3 -ffast-math -DNDEBUG
CFLAGS = -fexpensive-optimizations -O3 -ffast-math -DNDEBUG
endif

.PHONY: all recall clean

all: ${BUILDDIR} $(TARGETS)

install: install-script.nsi all
	/c/Program\ Files/NSIS/makeNSIS.exe $<

clean:
	@if ( [ -d ${BUILDDIR} ] ) then \
	   echo rm -r ${BUILDDIR};      \
	   rm -r ${BUILDDIR};           \
	fi

${BUILDDIR}:
	@if (! [ -d ${BUILDDIR} ] ) then   \
	   echo mkdir ${BUILDDIR};         \
	   mkdir ${BUILDDIR};              \
	fi

$(BUILDDIR)/VirtualDimension.exe: ${OBJ_FILE} ${RES_FILE}
	g++ $^ -o $@ -mwindows -mthreads -lcomctl32 -lole32 -lolepro32 -lversion -luuid $(CXXFLAGS)
ifndef DEBUG
	strip --strip-all $@
endif

$(BUILDDIR)/HookDLL.dll: $(BUILDDIR)/HookDLL.o $(BUILDDIR)/SharedMenuBuffer.o
	g++ -shared -mwindows -mthreads -o $@ $^ $(CXXFLAGS)
ifndef DEBUG
	strip --strip-all $@
endif

$(BUILDDIR)/%.res: %.rc ${BUILDDIR}
	windres -i $< -J rc -o $@ -O coff --include-dir=..

$(BUILDDIR)/%.dll: $(BUILDDIR)/%.res
	g++ -shared -mwindows -mthreads -o $@ $^ $(CXXFLAGS)
ifndef DEBUG
	strip --strip-all $@
endif	

$(BUILDDIR)/libtransp.a: Transp.def
	dlltool --def $< --dllname user32.dll  --output-lib $@

$(BUILDDIR)/%.d: %.cpp $(BUILDDIR)
	@-g++ -MM $(CPPFLAGS) $< \
                      | sed 's|\($*\)\.o[ :]*|$(BUILDDIR)/\1.o $@ : |g' > $@;

$(BUILDDIR)/%.d: %.c $(BUILDDIR)
	@-gcc -MM $(CPPFLAGS) $< \
                      | sed 's|\($*\)\.o[ :]*|$(BUILDDIR)/\1.o $@ : |g' > $@;

$(BUILDDIR)/%.o: %.cpp $(BUILDDIR)
	g++ -c -o $@ $< $(CXXFLAGS)

$(BUILDDIR)/%.o: %.c $(BUILDDIR)
	gcc -c -o $@ $< $(CFLAGS)

-include $(DEP_FILE)
