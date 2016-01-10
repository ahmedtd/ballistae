# We use nonrecursive autotools.  For now, all Makefile logic is kept at the top
# directory of the project.

# Pull in our local macros.  autoreconf gives a warning if we try to use :=
# assignment
ACLOCAL_AMFLAGS = -I m4

# We use c++14 across the board.
AM_CFLAGS := -std=c++14 -Wall -Werror

lib_LTLIBRARIES :=
noinst_PROGRAMS :=

# libballistae

cameraincludedir   := $(pkgincludedir)/camera
geometryincludedir := $(pkgincludedir)/geometry
materialincludedir := $(pkgincludedir)/material

pkginclude_HEADERS := \
  include/libballistae/aabox.hh \
  include/libballistae/camera.hh \
  include/libballistae/color.hh \
  include/libballistae/contact.hh \
  include/libballistae/dense_signal.hh \
  include/libballistae/geometry.hh \
  include/libballistae/image.hh \
  include/libballistae/kd_tree.hh \
  include/libballistae/material.hh \
  include/libballistae/material_map.hh \
  include/libballistae/options.hh \
  include/libballistae/ray.hh \
  include/libballistae/render_scene.hh \
  include/libballistae/scene.hh \
  include/libballistae/span.hh \
  include/libballistae/uniform_sphere_sampling.hh \
  include/libballistae/vector_distributions.hh \
  include/libballistae/vector.hh

camerainclude_HEADERS := \
  include/libballistae/camera/pinhole.hh

geometryinclude_HEADERS := \
  include/libballistae/geometry/box.hh \
  include/libballistae/geometry/cylinder.hh \
  include/libballistae/geometry/infinity.hh \
  include/libballistae/geometry/load_obj.hh \
  include/libballistae/geometry/plane.hh \
  include/libballistae/geometry/sphere.hh \
  include/libballistae/geometry/surface_mesh.hh \
  include/libballistae/geometry/tri_mesh.hh

materialinclude_HEADERS := \
  include/libballistae/material/directional_emitter.hh \
  include/libballistae/material/emitter.hh \
  include/libballistae/material/gauss.hh \
  include/libballistae/material/mc_lambert.hh \
  include/libballistae/material/nc_smooth.hh \
  include/libballistae/material/pc_smooth.hh

lib_LTLIBRARIES += libballistae.la

libballistae_la_SOURCES := \
  src/libballistae/scene.cc \
  src/libballistae/camera/pinhole.cc \
  src/libballistae/geometry/box.cc \
  src/libballistae/geometry/cylinder.cc \
  src/libballistae/geometry/infinity.cc \
  src/libballistae/geometry/load_obj.cc \
  src/libballistae/geometry/plane.cc \
  src/libballistae/geometry/sphere.cc \
  src/libballistae/geometry/surface_mesh.cc \
  src/libballistae/geometry/tri_mesh.cc

libballistae_la_CXXFLAGS := -I$(srcdir)/include -fopenmp $(AM_CFLAGS)

libballistae_la_LIBADD := -lboost_program_options

#
# Example scenes
#

noinst_PROGRAMS += examples/cornell-box
examples_cornell_box_SOURCES := examples/cornell-box.cc
examples_cornell_box_CXXFLAGS := -I$(srcdir)/include $(AM_CFLAGS)
examples_cornell_box_LDADD := libballistae.la -lboost_program_options
