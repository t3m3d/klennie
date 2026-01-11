# Maintainer: Your Name <you@example.com>
pkgname=kprocview-git
pkgver=0.1.0
pkgrel=1
pkgdesc="Tiny ncurses-based process viewer (KryptonBytes style)"
arch=('x86_64')
url="https://example.com/kprocview"
license=('MIT')
depends=('ncurses')
makedepends=('cmake' 'git')
source=("$pkgname::git+file://$PWD")
md5sums=('SKIP')

pkgver() {
  cd "$srcdir/$pkgname"
  echo "0.1.0"
}

build() {
  cd "$srcdir/$pkgname"
  mkdir -p build
  cd build
  cmake ..
  make
}

package() {
  cd "$srcdir/$pkgname/build"
  install -Dm755 kprocview "$pkgdir/usr/bin/kprocview"
}
