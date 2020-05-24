VERSION=2.0.4
curl -OL "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/${VERSION}.tar.gz" || exit 1
tar xf "${VERSION}.tar.gz" --strip-components=1 || exit 1
rm "${VERSION}.tar.gz" || exit 1


