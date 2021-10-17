pkgname=dwmblocks
pkgver=0.5.r4.g60e49dc
pkgrel=1
arch=('x86_64')
url="https://github.com/eFyhDv6dLgi9TWN/dwmblocks"
license=('GPL2')
depends=('libx11' 'glibc')
makedepends=('git' 'make' 'sed')

pkgver() {
	git describe --tags --long master |\
		sed 's/\([^-]*-g\)/r\1/;s/-/./g'
}

build() {
	cd ${srcdir}/..
	make
}

package() {
	cd ${srcdir}/..
	make PREFIX="${pkgdir}/usr" install
	make clean
}
