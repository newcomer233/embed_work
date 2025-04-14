# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lkj/piper/build/s/src/spdlog_external"
  "/home/lkj/piper/build/s/src/spdlog_external-build"
  "/home/lkj/piper/build/s"
  "/home/lkj/piper/build/s/tmp"
  "/home/lkj/piper/build/s/src/spdlog_external-stamp"
  "/home/lkj/piper/build/s/src"
  "/home/lkj/piper/build/s/src/spdlog_external-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lkj/piper/build/s/src/spdlog_external-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lkj/piper/build/s/src/spdlog_external-stamp${cfgdir}") # cfgdir has leading slash
endif()
