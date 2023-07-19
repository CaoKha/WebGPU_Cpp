function(target_treat_all_warning_as_errors Target)
  if(XCODE)
    set_target_properties(
      App PROPERTIES XCODE_GENERATE_SCHEME ON
                     XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
  else()
    set_target_properties(${Target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
  endif()
  if(MSVC)
    target_compile_options(${Target} PRIVATE /w4)
  else()
    target_compile_options(${Target} PRIVATE -Wall -Wextra -pedantic)
  endif()
endfunction()
