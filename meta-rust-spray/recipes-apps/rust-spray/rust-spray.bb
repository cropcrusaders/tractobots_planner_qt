DESCRIPTION = "Rust Spray application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=00000000000000000000000000000000"

SRC_URI = "git://github.com/cropcrusaders/Rust-Spray.git;branch=main;protocol=https"

S = "${WORKDIR}/git"

do_compile() {
    echo "Placeholder compile commands"
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 rust-spray ${D}${bindir}
}
