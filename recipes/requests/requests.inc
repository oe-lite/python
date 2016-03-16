DESCRIPTION = "Requests is the only Non-GMO HTTP library for Python, safe for human consumption."
LICENSE = "Apache-2.0"

inherit python-module

SRC_URI = "https://github.com/kennethreitz/requests/archive/v${PV}.tar.gz"
S = "${SRCDIR}/requests-${PV}"
