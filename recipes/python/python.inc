# -*- mode:python; -*-
DESCRIPTION = "Python is a programming language that lets you work more quickly and integrate your systems more effectively."
HOMEPAGE = "https://www.python.org/"
LICENSE = "python"

RECIPE_TYPES = "native machine"

inherit autotools  make

CROSS_DEPENDS = "native:python  native:python-lib libssl libz libreadline libffi libcrypto libtermcap libm"
CROSS_DEPENDS:native = ""
DEPENDS += "${CROSS_DEPENDS}"

DEPENDS_${PN}:machine += " libz libssl libreadline libffi libcrypto libtermcap libm "
RDEPENDS_${PN}:machine += "libz libssl libreadline libffi libcrypto libtermcap libm "

SRC_URI = "http://www.python.org/ftp/python/${PV}/Python-${PV}.tar.xz"
# The machine needs to be patched
# Original patch from:
# http://www.trevorbowen.com/2013/10/07/cross-compiling-python-2-7-5-for-embedded-linux/
# http://bugs.python.org/file31991/Python-2.7.5-xcompile.patch
# Note the patch has been modified!
SRC_URI:>machine = " file://Python-${PV}-xcompile.patch;striplevel=3"

# Python is in caps!
S = "${SRCDIR}/Python-${PV}"

EXTRA_OEMAKE_COMPILE:native="python Parser/pgen"
EXTRA_OECONF:machine = "\
  --disable-ipv6\
  ac_cv_file__dev_ptmx=no\
  ac_cv_file__dev_ptc=no\
"

# Make sure to include pgen and python from native into the package
do_install[postfuncs] += "${DO_INSTALL_EXTRA_FILES}"
DO_INSTALL_EXTRA_FILES = ""
DO_INSTALL_EXTRA_FILES:native = "do_install_extra_files"
do_install_extra_files () {
	install -m 0755 ${S}/Parser/pgen \
		${D}/bin/pgen
}


PACKAGES += "${PN}-lib"
FILES_${PN}-lib = "${base_libdir}/python2.7/*"

PACKAGES += "${PN}-lib-target"
FILES_${PN}-lib-target = "${libdir}/python2.7/*"
