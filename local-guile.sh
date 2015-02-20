#!/bin/bash

# Code is not mine.
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

export GUILE_LOAD_PATH="${DIR}"/guile/2.0
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/libballistae
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/libguile_armadillo
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/libguile_ballistae
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/ballistae_camera_plugin_pinhole
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/ballistae_geom_plugin_cylinder
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/ballistae_geom_plugin_plane
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/ballistae_geom_plugin_sphere
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}":"${DIR}"/src/ballistae_matr_plugin_phong
export LD_LIBRARY_PATH
export LD_WARN=unimportant-value # Warn about unresolved symbols.
#export LD_DEBUG=bindings
exec guile-2.0 "$@"
