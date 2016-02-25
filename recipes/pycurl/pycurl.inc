DESCRIPTION = "PycURL is a Python interface to libcurl."
LICENSE = "LGPL-2.1"

inherit python-module binconfig-stage

DEPENDS = "libcurl util/curl-config libssl"
SRC_URI = "https://dl.bintray.com/pycurl/pycurl/pycurl-${PV}.tar.gz"
S = "${SRCDIR}/pycurl-${PV}"

EXTRA_PYCONF = " \
    --with-openssl \
    --libraries=curl \
"
PARALLEL_MAKE = ""
