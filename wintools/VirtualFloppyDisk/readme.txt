
    This is a Virtual Floppy Drive (VFD) for Windows NT platform.
    Copyright (C) 2003-2008 Ken Kato (chitchat.vdk@gmail.com)
    http://chitchat.at.infoseek.co.jp/vmware/vfd.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    The GNU General Public License is also available from:
    http://www.gnu.org/copyleft/gpl.html


PROGRAM FILES

    vfd.sys
	VFD kernel-mode driver.

    vfd.exe
	VFD control program (console version).

    vfd.dll
        VFD device control and shell extension library

    vfdwin.exe
	VFD control program (GUI version).


INSTALL

    If you have an older version of the VFD already installed on your
    system, see UPDATE section below.

    Copy program files listed above into a directory on a local drive.

    Then Use VFD.EXE or VFDWIN.EXE to install components into the system:

    - Install the driver into the system
        (VFD.EXE)     VFD.EXE INSTALL
        (VFDWIN.EXE)  [Driver] -> [Install]

    - If you want the driver to start automatically every time the system
      starts
        (VFD.EXE)     VFD.EXE INSTALL /AUTO
        (VFDWIN.EXE)  [Driver] -> chosse [Auto] start type -> [Install]

    - Install the shell extension into the system if you want
        (VFD.EXE)     VFD.EXE SHELL /ON
        (VFDWIN.EXE)  [Shell] -> check [Shell Extension] box -> [Apply]

    - Define file associations if you want
        (VFDWIN.EXE)  [Association] -> make associations -> [Apply]

    !!! NOTE !!!
    Device drivers cannot be started from a network drive.  Make sure that
    at least the driver file (vfd.sys) is located on a local drive.


UPDATE

    Make sure the old version driver is NOT running and the old version of
    shell extension is NOT enabled, then just replace all program files with
    the newer version.

    !!! NOTE !!!
    The driver auto start method used for Windows 2000/XP in version 2.0 RC has
    a little problem with this version of the driver.  If you were using the 2.0
    RC with "auto" start enabled, set it to manual and then change back to "auto"
    with this version of VFD.EXE or VFDWIN.EXE.

    !!! NOTE !!!
    Windows Explorer may not release the DLL immediately after you disabled
    the shell extension and you may not be able to replace the DLL file.
    If that happens, restart the Explorer (e.g. logoff and logon again).


UNINSTALL

    Use VFD.EXE or VFDWIN.EXE to uninstall components from the system:

    - Uninstall the driver from the system
        (VFD.EXE)     VFD.EXE REMOVE
        (VFDWIN.EXE)  [Driver] -> [Uninstall]

    - Uninstall the shell extension from the system if you enabled it
        (VFD.EXE)     VFD.EXE SHELL /OFF
        (VFDWIN.EXE)  [Shell] -> uncheck [Shell Extension] box -> [Apply]

    - Delete file associations if you made associations
        (VFDWIN.EXE)  [Association] -> [Clear All] -> [Apply]

    Then delete VFD files from your hard drive.


HISTORY

    06-02-2008  Version 2.1.2008.0206 Release
                -- zlib version up, small changes to user interface
    04-04-2005  Version 2.1 Release
    27-04-2003  vfdwin.exe 1.01 -- Fixed a few bugs in user interface
    16-04-2003  Initial Release


COPYRIGHTS

    This program includes ntifs.h developed by Bo Brantén and published
    under the GNU General Public License (http://www.acc.umu.se/~bosse/).

      a free ntifs.h project
      Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004 Bo Brantén.

    This program includes zlib compression library developed by Jean-loup
    Gailly and Mark Adler (http://www.gzip.org/zlib/).

      'zlib' general purpose compression library
      Copyright (C) 1995-2004 Jean-loup Gailly and Mark Adler

      This software is provided 'as-is', without any express or implied
      warranty.  In no event will the authors be held liable for any damages
      arising from the use of this software.

      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute it
      freely, subject to the following restrictions:

      1. The origin of this software must not be misrepresented; you must not
         claim that you wrote the original software. If you use this software
         in a product, an acknowledgment in the product documentation would be
         appreciated but is not required.
      2. Altered source versions must be plainly marked as such, and must not be
         misrepresented as being the original software.
      3. This notice may not be removed or altered from any source distribution.

    This program includes a pre-built zlib library developed by Gilles Vollant
    (http://www.winimage.com/zLibDll/).

      zlibstat.lib : The 32 bits statis library of zLib for Visual C++
      Copyright (C) Gilles Vollant

