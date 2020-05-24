# Updates and patches nasm

VERSION=2.13.03

find ./ -type f -not '(' -name BUILD -o -name update.sh ')' -delete || exit 1

curl -OL "http://www.nasm.us/pub/nasm/releasebuilds/${VERSION}/nasm-${VERSION}.tar.bz2" || exit 1
tar xf "nasm-${VERSION}.tar.bz2" --strip-components=1 || exit 1
rm "nasm-${VERSION}.tar.bz2" || exit 1
