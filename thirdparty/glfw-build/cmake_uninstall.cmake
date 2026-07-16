
if (NOT EXISTS "/home/embdev/Documents/GitHub/melonyx/thirdparty/glfw-build/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: \"/home/embdev/Documents/GitHub/melonyx/thirdparty/glfw-build/install_manifest.txt\"")
endif()

file(READ "/home/embdev/Documents/GitHub/melonyx/thirdparty/glfw-build/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")

foreach (file ${files})
  message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  if (EXISTS "$ENV{DESTDIR}${file}")
    exec_program("/nix/store/rfad6gqp02yfhjkh7m080jzcrq104jq8-cmake-4.1.2/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
                 OUTPUT_VARIABLE rm_out
                 RETURN_VALUE rm_retval)
    if (NOT "${rm_retval}" STREQUAL 0)
      MESSAGE(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif()
  elseif (IS_SYMLINK "$ENV{DESTDIR}${file}")
    EXEC_PROGRAM("/nix/store/rfad6gqp02yfhjkh7m080jzcrq104jq8-cmake-4.1.2/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
                 OUTPUT_VARIABLE rm_out
                 RETURN_VALUE rm_retval)
    if (NOT "${rm_retval}" STREQUAL 0)
      message(FATAL_ERROR "Problem when removing symlink \"$ENV{DESTDIR}${file}\"")
    endif()
  else()
    message(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
  endif()
endforeach()

