# Copyright (C) 2011,2012 Aljosha Papsch <papsch.al@googlemail.com>
# and ripperX developers (http://sourceforge.net/projects/ripperx)
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

AUTOMAKE_OPTIONS = dist-bzip2

SUBDIRS = po src config plugins

man_MANS = \
     ripperX.1

EXTRA_DIST = \
	BUGS \
	README.plugin \
	README.plugin_spec_v0.1 \
	README.plugin_tester \
	autogen.sh \
	ripperX.desktop \
	ripperX.pc.in \
	$(man_MANS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ripperX.pc
