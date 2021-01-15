#!/usr/bin/env python
#
# Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

# ------------------------------------------------------------ #
# Simple example of how to communicate with Lunar using the   #
# com.expidus.Lunar D-BUS interface.                             #
#                                                              #
# Lunar must be compiled with D-BUS support for this to work. #
# ------------------------------------------------------------ #

import gtk
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
  import dbus.glib

# acquire a reference to the Lunar object
bus = dbus.SessionBus()
lunar_object = bus.get_object('com.expidus.Lunar', '/com/expidus/FileManager')
lunar = dbus.Interface(lunar_object, 'com.expidus.Lunar')

# You can now invoke methods on the lunar object, for
# example, to terminate a running Lunar instance (just
# like Lunar -q), you can use:
#
# lunar.Terminate()
#
# or, if you want to open the bulk rename dialog in the
# standalone version with an empty file list and /tmp
# as default folder for the "Add Files" dialog, use:
#
# lunar.BulkRename('/tmp', [], True, '', '')
#
# See the lunar-dbus-service-infos.xml file for the exact
# interface definition.
#

# We just popup the bulk rename dialog to
# demonstrate that it works. ;-)
lunar.BulkRename('/tmp', [], True, '', '')
