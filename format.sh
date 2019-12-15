find ./ -type f -name '*.cc' -exec clang-format -i -style=file '{}' ';'
find ./ -type f -name '*.hh' -exec clang-format -i -style=file '{}' ';'
