# Function to recursively list all target linked libraries
function(list_target_linked_libraries target_name)
   # Check if target exists
   if(NOT TARGET ${target_name})
      message(WARNING "Target '${target_name}' does not exist")
      return()
   endif()
   message(STATUS "Listing linked libraries for target: ${target_name}")
   # Set to track visited targets to avoid infinite recursion
   set(visited_targets "")
   # Call the recursive helper function
   _list_target_libs_recursive("${target_name}" "" "${visited_targets}")
endfunction()

# Helper function for recursive library listing
function(_list_target_libs_recursive current_target indent visited_targets)
   # Check if we've already visited this target
   list(FIND visited_targets "${current_target}" found_index)
   if(NOT found_index EQUAL -1)
      # Print the target as visited and return without exploring children
      message(STATUS "${indent}>> already visited")
      return()
   endif()

   # Add current target to visited list
   list(APPEND visited_targets "${current_target}")

   # Get different types of linked libraries
   get_target_property(link_libs ${current_target} LINK_LIBRARIES)
   get_target_property(interface_link_libs ${current_target} INTERFACE_LINK_LIBRARIES)
   get_target_property(imported_link_interface_libs ${current_target} IMPORTED_LINK_INTERFACE_LIBRARIES)

   # Process LINK_LIBRARIES
   if(link_libs)
      foreach(lib ${link_libs})
         message(STATUS "${indent}-> ${lib}")
         # If it's a target, recurse into it
         if(TARGET ${lib})
            _list_target_libs_recursive("${lib}" "${indent}    " "${visited_targets}")
         endif()
      endforeach()
   endif()

   # Process INTERFACE_LINK_LIBRARIES
   if(interface_link_libs)
      foreach(lib ${interface_link_libs})
         message(STATUS "${indent}-> (interface) ${lib}")
         # If it's a target, recurse into it
         if(TARGET ${lib})
            _list_target_libs_recursive("${lib}" "${indent}    " "${visited_targets}")
         endif()
      endforeach()
   endif()

   # Process IMPORTED_LINK_INTERFACE_LIBRARIES (for imported targets)
   if(imported_link_interface_libs)
      foreach(lib ${imported_link_interface_libs})
         message(STATUS "${indent}-> (imported) ${lib}")
      endforeach()
   endif()

   set(visited_targets "${visited_targets}" PARENT_SCOPE)
endfunction()
