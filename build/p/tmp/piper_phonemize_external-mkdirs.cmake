# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lkj/piper/build/p/src/piper_phonemize_external"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-build"
  "/home/lkj/piper/build/p"
  "/home/lkj/piper/build/p/tmp"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-stamp"
  "/home/lkj/piper/build/p/src"
  "/home/lkj/piper/build/p/src/piper_phonemize_external-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lkj/piper/build/p/src/piper_phonemize_external-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lkj/piper/build/p/src/piper_phonemize_external-stamp${cfgdir}") # cfgdir has leading slash
endif()
