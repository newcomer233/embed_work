# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-src"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-build"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-subbuild/sonic-git-populate-prefix"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-subbuild/sonic-git-populate-prefix/tmp"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-subbuild/sonic-git-populate-prefix/src/sonic-git-populate-stamp"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-subbuild/sonic-git-populate-prefix/src"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-subbuild/sonic-git-populate-prefix/src/sonic-git-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-subbuild/sonic-git-populate-prefix/src/sonic-git-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lkj/piper/build/p/src/piper_phonemize_external-build/e/src/espeak_ng_external-build/_deps/sonic-git-subbuild/sonic-git-populate-prefix/src/sonic-git-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
