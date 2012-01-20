#!/bin/sh

aclocal \
  && libtoolize --force --copy \
  && autoheader \
  && automake --add-missing --foreign --copy \
  && autoconf \
  && ./configure --enable-maintainer-mode $@

# clean up a bit
rm -rf ./autom4te.cache
