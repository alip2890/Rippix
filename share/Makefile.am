# Copyright (C) 2012 Aljosha Papsch <papsch.al@googlemail.com>
#
# This file is part of Rippix.
#
# Rippix is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
#
# Rippix is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see `http://www.gnu.org/licenses/'.

rippix_sharedir = $(datadir)/rippix

#
# UI files
#
uidir = $(rippix_sharedir)/ui
ui_DATA = configdialog.ui mainwindow.ui selectframe.ui

#
# GConf schema file
#
schemasdir = $(sysconfdir)/gconf/schemas
schemas_DATA = rippix.schemas

EXTRA_DIST = $(ui_DATA) $(schemas_DATA)

install-data-hook:
	-GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) \
	gconftool-2 --makefile-install-rule \
	$(schemasdir)/rippix.schemas

uninstall-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) \
	gconftool-2 --makefile-uninstall-rule \
	$(schemasdir)/rippix.schemas
	-rm -r $(uidir)
	-rm -r $(rippix_sharedir)
