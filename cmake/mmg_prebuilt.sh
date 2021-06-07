#!/bin/bash

LIBIGL_DIR=/Users/max/Developer/Library/Graphics/libiglmmg/libigl
MMG_SOURCEDIR=external/mmg
MMG_BUILDDIR=build/prebuilt/mmg
MMG_INSTALLDIR=$LIBIGL_DIR/install

MMG_BUILDTYPE=Release

cmake -S $MMG_SOURCEDIR -B $MMG_BUILDDIR \
-DCMAKE_BUILD_TYPE=$MMG_BUILDTYPE \
-DBUILD=MMG2D \
-DBUILD_SHARED_LIBS=ON \
-DLIBMMG2D_STATIC=ON \
-DLIBMMG2D_SHARED=ON \
-DUSE_SCOTCH=OFF \
-DUSE_ELAS=OFF \
-DUSE_VTK=OFF \
-DMMG2D_CI=OFF \
-DBUILD_TESTING=OFF \
-DCMAKE_INSTALL_PREFIX:PATH=$MMG_INSTALLDIR

make -C $MMG_BUILDDIR

make -C $MMG_BUILDDIR install