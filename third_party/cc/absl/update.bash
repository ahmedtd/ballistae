# Updates and patches abseil-cpp

VERSION=20200225.2

find ./ -not '(' -path './' -o -name update.bash ')' -delete || exit 1

curl -OL "https://github.com/abseil/abseil-cpp/archive/${VERSION}.tar.gz" || exit 1
tar xf "${VERSION}.tar.gz" --strip-components=1 || exit 1
rm "${VERSION}.tar.gz" || exit 1

rm WORKSPACE
find ./ -type f '(' -name BUILD.bazel -o -name '*.bzl' ')' -exec sed -i 's|"//absl|"//third_party/cc/absl/absl|g' '{}' ';'
find ./ -type f '(' -name '*.h' -o -name '*.cc' -o -name '*.inc' ')' -exec sed -i 's|"absl/|"third_party/cc/absl/absl/|g' '{}' ';'
