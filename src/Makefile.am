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

bin_PROGRAMS = rippix

rippix_SOURCES = gettext.h \
	cddb.c cddb.h \
	cddbp.c cddbp.h \
	common.h \
	rw_config.c rw_config.h \
	windowbuilder.c windowbuilder.h \
	config_window_handler.c config_window_handler.h \
	dir_window_handler.c dir_window_handler.h \
	err_dialog_handler.c err_dialog_handler.h \
	interface_common.h \
	job_control.c job_control.h \
        main.c main.h \
        main_window_handler.c main_window_handler.h \
        misc_utils.c misc_utils.h \
        players_manipulation.c players_manipulation.h \
        ripper_encoder_manipulation.c ripper_encoder_manipulation.h \
        select_frame_handler.c select_frame_handler.h \
        status_frame_handler.c status_frame_handler.h

rippix_LDADD = \
	$(INTLLIBS) \
	$(DEPENDENCIES_LIBS)

AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"rippix\" \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATAROOTDIR=\""$(datarootdir)"\" \
	$(DEPENDENCIES_CFLAGS)


