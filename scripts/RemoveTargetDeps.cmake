# Function to remove specific dependencies from a target
function(_remove_target_dependencies target_name dependency_names)
   # Parse optional VERBOSE argument
   set(options VERBOSE)
   set(oneValueArgs "")
   set(multiValueArgs "")
   cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

   # Check if target exists
   if(NOT TARGET ${target_name})
      message(WARNING "Target '${target_name}' does not exist")
      return()
   endif()

   # Convert single dependency name to list if needed
   if(NOT "${dependency_names}" MATCHES ";")
      set(dependency_names "${dependency_names}")
   endif()

   if(ARGS_VERBOSE)
      message(STATUS "Removing dependencies from target: ${target_name}")
      foreach(dep_name ${dependency_names})
         message(STATUS "  - Removing: ${dep_name}")
      endforeach()
   endif()

   # Process LINK_LIBRARIES property
   get_target_property(link_libs ${target_name} LINK_LIBRARIES)
   if(link_libs)
      set(filtered_libs "")
      foreach(lib ${link_libs})
         set(should_remove FALSE)
         foreach(dep_name ${dependency_names})
            if("${lib}" MATCHES "${dep_name}")
               set(should_remove TRUE)
               if(ARGS_VERBOSE)
                  message(STATUS "  Removed '${lib}' from LINK_LIBRARIES")
               endif()
               break()
            endif()
         endforeach()

         if(NOT should_remove)
            list(APPEND filtered_libs "${lib}")
         endif()
      endforeach()

      # Update the property with filtered list
      if(filtered_libs)
         set_target_properties(${target_name} PROPERTIES LINK_LIBRARIES "${filtered_libs}")
      else()
         # Clear the property if no libraries remain
         set_target_properties(${target_name} PROPERTIES LINK_LIBRARIES "")
      endif()
   endif()

   # Process INTERFACE_LINK_LIBRARIES property
   get_target_property(interface_link_libs ${target_name} INTERFACE_LINK_LIBRARIES)
   if(interface_link_libs)
      set(filtered_interface_libs "")
      foreach(lib ${interface_link_libs})
         set(should_remove FALSE)
         foreach(dep_name ${dependency_names})
            if("${lib}" MATCHES "${dep_name}")
               set(should_remove TRUE)
               if(ARGS_VERBOSE)
                  message(STATUS "  Removed '${lib}' from INTERFACE_LINK_LIBRARIES")
               endif()
               break()
            endif()
         endforeach()

         if(NOT should_remove)
            list(APPEND filtered_interface_libs "${lib}")
         endif()
      endforeach()

      # Update the property with filtered list
      if(filtered_interface_libs)
         set_target_properties(${target_name} PROPERTIES INTERFACE_LINK_LIBRARIES "${filtered_interface_libs}")
      else()
         # Clear the property if no libraries remain
         set_target_properties(${target_name} PROPERTIES INTERFACE_LINK_LIBRARIES "")
      endif()
   endif()

   # Process INTERFACE_LINK_LIBRARIES_DIRECT property (CMake 3.13+)
   get_target_property(interface_link_libs_direct ${target_name} INTERFACE_LINK_LIBRARIES_DIRECT)
   if(interface_link_libs_direct)
      set(filtered_interface_libs_direct "")
      foreach(lib ${interface_link_libs_direct})
         set(should_remove FALSE)
         foreach(dep_name ${dependency_names})
            if("${lib}" MATCHES "${dep_name}")
               set(should_remove TRUE)
               if(ARGS_VERBOSE)
                  message(STATUS "  Removed '${lib}' from INTERFACE_LINK_LIBRARIES_DIRECT")
               endif()
               break()
            endif()
         endforeach()

         if(NOT should_remove)
            list(APPEND filtered_interface_libs_direct "${lib}")
         endif()
      endforeach()

      # Update the property with filtered list
      if(filtered_interface_libs_direct)
         set_target_properties(${target_name} PROPERTIES INTERFACE_LINK_LIBRARIES_DIRECT "${filtered_interface_libs_direct}")
      else()
         # Clear the property if no libraries remain
         set_target_properties(${target_name} PROPERTIES INTERFACE_LINK_LIBRARIES_DIRECT "")
      endif()
   endif()
endfunction()

# Helper function for recursive dependency removal
function(_remove_dependencies_recursive current_target dependency_names visited_targets verbose_flag)
   # Check if we've already visited this target
   list(FIND visited_targets "${current_target}" found_index)
   if(NOT found_index EQUAL -1)
      return()
   endif()

   # Add current target to visited list
   list(APPEND visited_targets "${current_target}")

   # Remove dependencies from current target, passing VERBOSE flag if set
   if(verbose_flag)
      _remove_target_dependencies("${current_target}" "${dependency_names}" VERBOSE)
   else()
      _remove_target_dependencies("${current_target}" "${dependency_names}")
   endif()

   # Get linked libraries to recurse into
   get_target_property(link_libs ${current_target} LINK_LIBRARIES)
   get_target_property(interface_link_libs ${current_target} INTERFACE_LINK_LIBRARIES)

   # Recursively process LINK_LIBRARIES
   if(link_libs)
      foreach(lib ${link_libs})
         if(TARGET ${lib})
            _remove_dependencies_recursive("${lib}" "${dependency_names}" "${visited_targets}" ${verbose_flag})
         endif()
      endforeach()
   endif()

   # Recursively process INTERFACE_LINK_LIBRARIES
   if(interface_link_libs)
      foreach(lib ${interface_link_libs})
         if(TARGET ${lib})
            _remove_dependencies_recursive("${lib}" "${dependency_names}" "${visited_targets}" ${verbose_flag})
         endif()
      endforeach()
   endif()

   set(visited_targets "${visited_targets}" PARENT_SCOPE)  # Update the visited targets list
endfunction()

# Function to recursively remove dependencies from a target and all its dependencies
function(remove_target_dependencies_recursive target_name dependency_names)
   # Parse optional VERBOSE argument
   set(options VERBOSE)
   set(oneValueArgs "")
   set(multiValueArgs "")
   cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

   # Check if target exists
   if(NOT TARGET ${target_name})
      message(WARNING "Target '${target_name}' does not exist")
      return()
   endif()

   # Convert single dependency name to list if needed
   if(NOT "${dependency_names}" MATCHES ";")
      set(dependency_names "${dependency_names}")
   endif()

   if(ARGS_VERBOSE)
      message(STATUS "Recursively removing dependencies from target tree: ${target_name}")
   endif()

   # Set to track visited targets to avoid infinite recursion
   set(visited_targets "")

   # Call the recursive helper function, passing VERBOSE flag
   _remove_dependencies_recursive("${target_name}" "${dependency_names}" "${visited_targets}" ${ARGS_VERBOSE})
endfunction()

# Example usage of dependency removal functions:
# Recursively remove dependencies from target and all its dependencies (silent)
# remove_target_dependencies_recursive(microcl-plugin "LLVMSupport")
# Recursively remove dependencies from target and all its dependencies (with verbose output)
# remove_target_dependencies_recursive(microcl-plugin "LLVMSupport" VERBOSE)
