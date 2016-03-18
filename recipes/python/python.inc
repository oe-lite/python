DESCRIPTION = "Python is a programming language that lets you work quickly and integrate systems more effectively."
HOMEPAGE = "https://www.python.org/"
LICENSE = "python-2.0"
RECIPE_TYPES = "native machine"
COMPATIBLE_HOST_ARCHS = ".*linux"

# Make sure to set these in the version file
MAJOR_PV = ""
MINOR_PV = ""
MINOR_M  = ""

# It's important to inherit auto-package-libs before
# autotools to make sure the libs end up in the right
# packages
inherit auto-package-utils auto-package-libs chrpath autotools pkgconfig

# The native build is only used when building for machine
# target, so keep dependencies to a minimum
DEPENDS += "libffi"

# When building for machine target, include the full
# dependency list
DEPENDS:>machine = " \
    native:python${MAJOR_PV} \
    native:python${MAJOR_PV}-dev \
    native:python${MAJOR_PV}-distutils \
    native:python${MAJOR_PV}-pgen \
    native:python${MAJOR_PV}-freeze-importlib \
    libsqlite3 \
    libreadline \
    libm \
    libz \
    libexpat \
    libpthread \
    libssl \
    liblzo2 \
    libbz2 \
    libncurses \
    libcrypto \
"

SRC_URI = " \
    https://www.python.org/ftp/python/${PV}/Python-${PV}.tgz \
    file://libffi.patch \
"

SRC_URI:>machine = " \
    file://host-tools.patch \
    file://0001-setup.py-do-not-add-invalid-header-locations.patch \
"
S = "${SRCDIR}/Python-${PV}"

# Ensure that we are using the system copy of various
# libraries (expat, zlib, libffi, and libmpdec),
# rather than copies shipped in the tarball
do_unpack[postfuncs] += "do_unpack_rm_deps"
do_unpack_rm_deps() {
    rm -r ${S}/Modules/expat
    rm -r ${S}/Modules/zlib
    rm -r ${S}/Modules/_ctypes/{darwin,libffi}*

    # libmpdec is only bundled with python3
    rm -rf ${S}/Modules/_decimal/libmpdec
}

EXTRA_OECONF:>machine = " \
    ac_cv_file__dev_ptmx=yes \
    ac_cv_file__dev_ptc=no \
    --with-threads \
    --with-computed-gotos \
    --with-dbmliborder=ndbm:bdb:gdbm \
    --with-system-expat \
    --with-system-ffi \
    --with-system-libmpdec \
    --without-ensurepip \
    --without-cxx-main \
    --without-doc-strings \
    --enable-shared \
    --enable-ipv6 \
    PGEN_FOR_BUILD=pgen \
    FREEZE_IMPORTLIB_FOR_BUILD=freeze_importlib \
"

EXTRA_OECONF:>native = " \
    --enable-shared \
    --with-system-expat \
    --with-system-ffi \
    --with-system-libmpdec \
    --without-cxx-main \
    --with-expat=none \
    --disable-sqlite3 \
    --disable-tk \
    --disable-curses \
    --disable-codecs-cjk \
    --disable-nis \
    --disable-unicodedata \
    --disable-dbm \
    --disable-gdbm \
    --disable-bsddb \
    --disable-test-modules \
    --disable-bz2 \
    --disable-zlib \
    --disable-ssl \
"

# Python looks for libraries and includes in weird places, so
# help it out
LDFLAGS:>native = " -L${S} -L${TARGET_SYSROOT}${libdir}"
LDFLAGS:>machine = " -L${S} -L${TARGET_SYSROOT}${libdir}"
CFLAGS:>machine = " -I${S}"

# Only build the native tools needed to build
# python for target
EXTRA_OEMAKE_COMPILE:>native = " python Parser/pgen"

# Install native tools for later use when cross building
# for target
DO_INSTALL_PGEN = ""
DO_INSTALL_PGEN:native = "do_install_pgen"
do_install[postfuncs] += "${DO_INSTALL_PGEN}"
do_install_pgen () {
    install -m 0755 ${S}/Parser/pgen ${D}${bindir}/pgen
}

# remove dynlibs as these are not used when bootstrapping
# python, and they make packageqa compain
DO_INSTALL_RM_DYNLIBS = ""
DO_INSTALL_RM_DYNLIBS:native = "do_install_rm_dynlibs"
#do_install[postfuncs] += "${DO_INSTALL_RM_DYNLIBS}"
do_install_rm_dynlibs () {
    rm -rf ${D}${libdir}/${PN}${MAJOR_PV}.${MINOR_PV}/lib-dynload/
}

# Some of the python tools set the shebang to #!/bin/python
# which is wrong. Replace this with #!/usr/bin/env python${MAJOR_PV}
do_install[postfuncs] += "do_install_shebangs"
do_install_shebangs() {
    for file in 2to3 idle idle3 pydoc pydoc3 pyvenv; do
        [ -x ${D}${bindir}/$file ] || continue
        sed -i 's:#!${bindir}/python[0-9\.]*:#!/usr/bin/env python${MAJOR_PV}:' \
          ${D}${bindir}/$file
    done
}

#
# Main python package
#
FILES_${PN} = " \
    ${bindir}/python${MAJOR_PV} \
    ${bindir}/python${MAJOR_PV}.${MINOR_PV} \
    ${bindir}/python${MAJOR_PV}.${MINOR_PV}m \
"
PROVIDES_${PN} = "${PN}${MAJOR_PV}"
PROVIDES_${PN}-dev = "${PN}${MAJOR_PV}-dev"
DEPENDS_${PN} = "${PN}-libpython${MAJOR_PV} ${PN}${MAJOR_PV}-modules ${PN}${MAJOR_PV}-symlink"
RDEPENDS_${PN} = "${PN}-libpython${MAJOR_PV} ${PN}${MAJOR_PV}-modules ${PN}${MAJOR_PV}-symlink"

# python2 install a symlink, while python3 doesn't. Try to align this
# by always creating the symlink and split it out in a separate package
PACKAGES =+ "${PN}-symlink"
FILES_${PN}-symlink = "${bindir}/python"
PROVIDES_${PN}-symlink = "${PN}${MAJOR_PV}-symlink"
do_install[postfuncs] += "do_install_symlink"
do_install_symlink() {
    ln -sf ./python${MAJOR_PV} ${D}${bindir}/python
}

# Split modules into a few separate packages
PACKAGES =+ "${PN}-test ${PN}-distutils ${PN}-modules"

#
# python-test package
#
PROVIDES_${PN}-test = "${PN}${MAJOR_PV}-test"
FILES_${PN}-test = "${libdir}/python${MAJOR_PV}.${MINOR_PV}/test"

#
# python-distutils package
#
PROVIDES_${PN}-distutils = "${PN}${MAJOR_PV}-distutils"
FILES_${PN}-distutils = "${libdir}/python${MAJOR_PV}.${MINOR_PV}/distutils"

#
# python-modules package
#
PROVIDES_${PN}-modules += "${PN}${MAJOR_PV}-modules"
FILES_${PN}-modules = "${libdir}/python${MAJOR_PV}.${MINOR_PV}/"
FILES_${PN}-modules[qa] += "allow-elf-bins-in-other-dirs"
DEPENDS_${PN}-modules += "libexpat libncurses libpanel libffi ${LIBPYTHON_M}"
DEPENDS_${PN}-modules:>machine = " libbz2 libcrypto libm libreadline libsqlite3 libssl libz"
RDEPENDS_${PN}-modules += "libexpat libncurses libpanel libffi"
RDEPENDS_${PN}-modules:>machine = " libbz2 libcrypto libm ${LIBPYTHON_M} libreadline libsqlite3 libssl libz"
# run chrpaths on lib-dynload extensions
CHRPATH_DIRS += "${libdir}/python${MAJOR_PV}.${MINOR_PV}/lib-dynload"

# Use placeholders when configuring the libs to
# shorten the lines
PYTHON   = "python${MAJOR_PV}"
LIBPYTHON = "lib${PYTHON}"
PYTHON_M = "python${MAJOR_PV}.${MINOR_PV}${MINOR_M}"
LIBPYTHON_M = "lib${PYTHON_M}"

# add python libs
AUTO_PACKAGE_LIBS = "${PYTHON} ${PYTHON_M}"
LIBRARY_VERSION = "1.0"

#
# libpythonX.Ym library package
#
LIBPYTHON_M_DEPENDS = ""
LIBPYTHON_M_DEPENDS:machine = "libc libdl libm libpthread"
DEPENDS_${PN}-${LIBPYTHON_M} = "${LIBPYTHON_M_DEPENDS}"
RDEPENDS_${PN}-${LIBPYTHON_M} = "${LIBPYTHON_M_DEPENDS}"
PROVIDES_${PN}-${LIBPYTHON_M} += "${LIBPYTHON_M}-1.0 ${LIBPYTHON_M}-1"
PROVIDES_${PN}-${LIBPYTHON_M}[qa] += "allow-missing-soname"

#
# libpythonX library package
#
DEPENDS_${PN}-${LIBPYTHON} += "${LIBPYTHON_M} libc"
RDEPENDS_${PN}-${LIBPYTHON} += "${LIBPYTHON_M} libc"
DEPENDS_${PN}-${LIBPYTHON}-dev += "${LIBPYTHON_M}"
PROVIDES_${PN}-${LIBPYTHON}[qa] += "allow-missing-soname"

# Split utils into separate packages
AUTO_PACKAGE_UTILS = " \
    2to3 \
    easy-install-${MAJOR_PV} \
    idle${MAJOR_PV} \
    pip${MAJOR_PV} \
    pydoc${MAJOR_PV} \
    pyvenv \
    python${MAJOR_PV}-config \
    smtpd.py \
"
AUTO_PACKAGE_UTILS:>native = " pgen freeze_importlib"
AUTO_PACKAGE_UTILS_RDEPENDS = "python${MAJOR_PV}"

# Provide version dependent packages to ensure the
# right version is used to build for target
PROVIDES_${PN}-pgen:>native = " ${PN}${MAJOR_PV}-pgen"
PROVIDES_${PN}-freeze-importlib:>native += "${PN}${MAJOR_PV}-freeze-importlib"

# Add symlinked files to each util package
FILES_${PN}-2to3 += "${bindir}/2to3-${MAJOR_PV}.${MINOR_PV} ${bindir}/2to3"
FILES_${PN}-easy-install-${MAJOR_PV} += "${bindir}/easy_install-${MAJOR_PV}.${MINOR_PV} ${bindir}/easy_install"
FILES_${PN}-idle${MAJOR_PV} += "${bindir}/idle${MAJOR_PV}.${MINOR_PV} ${bindir}/idle"
FILES_${PN}-pip${MAJOR_PV} += "${bindir}/pip${MAJOR_PV}.${MINOR_PV} ${bindir}/pip"
FILES_${PN}-pydoc${MAJOR_PV} += "${bindir}/pydoc${MAJOR_PV}.${MINOR_PV} ${bindir}/pydoc"
FILES_${PN}-pyvenv += "${bindir}/pyvenv-${MAJOR_PV}.${MINOR_PV}"
FILES_${PN}-python${MAJOR_PV}-config += " \
    ${bindir}/python-config \
    ${bindir}/python${MAJOR_PV}.${MINOR_PV}-config \
    ${bindir}/python${MAJOR_PV}.${MINOR_PV}m-config \
"
