# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class Babelflow(CMakePackage):
    """FIXME: Put a proper description of your package here."""

    homepage = "https://github.com/sci-visus/BabelFlow"
    url      = "https://github.com/sci-visus/BabelFlow/archive/v1.0.1.tar.gz"

    maintainers = ['spetruzza']

    version('1.0.1', sha256='b7817870b7a1d7ae7ae2eff1a1acec2824675fb856f666d5dc95c41ce453ae91')

    depends_on('mpi')

    variant("shared", default=True, description="Build Babelflow as shared libs")

    def cmake_args(self):
        args = []

        if "+shared" in self.spec:
            args.append('-DBUILD_SHARED_LIBS=ON')
        else:
            args.append('-DBUILD_SHARED_LIBS=OFF')

        return args
  