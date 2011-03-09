include(${PROJECT_SOURCE_DIR}/cmake/pcl_utils.cmake)

###############################################################################
# Add an option to build a subsystem or not.
# _var The name of the variable to store the option in.
# _name The name of the option's target subsystem.
# _desc The description of the subsystem.
# _default The default value (ON or OFF)
# ARGN Any dependencies that this subsystem has.
macro(PCL_SUBSYS_OPTION _var _name _desc _default)
    set(_opt_name "BUILD_${_name}")
    option(${_opt_name} ${_desc} ${_default})
    if(NOT ${_opt_name})
        set(${_var} FALSE)
        PCL_SET_SUBSYS_STATUS(${_name} FALSE "Disabled manually.")
    else(NOT ${_opt_name})
        set(${_var} TRUE)
        PCL_SET_SUBSYS_STATUS(${_name} TRUE)
    endif(NOT ${_opt_name})
    set(PCL_SUBSYSTEMS "${PCL_SUBSYSTEMS};${_name}" CACHE INTERNAL
        "Internal list of subsystems" FORCE)
endmacro(PCL_SUBSYS_OPTION)


###############################################################################
# Make one subsystem depend on one or more other subsystems, and disable it if
# they are not being built.
# _var The cumulative build variable. This will be set to FALSE if the
#   dependencies are not met.
# _name The name of the subsystem.
# ARGN The subsystems to depend on.
macro(PCL_SUBSYS_DEPEND _var _name)
    if(${_var})
        foreach(_dep ${ARGN})
            set(_dep_opt "BUILD_${_dep}")
            if(NOT ${_dep_opt})
                set(${_var} FALSE)
                PCL_SET_SUBSYS_STATUS(${_name} FALSE "Requires ${_dep}")
            else(NOT ${_dep_opt})
                include_directories(${PROJECT_SOURCE_DIR}/${_dep}/include)
            endif(NOT ${_dep_opt})
        endforeach(_dep)
    endif(${_var})
endmacro(PCL_SUBSYS_DEPEND)


###############################################################################
# Add a set of include files to install.
# _component The part of PCL that the install files belong to.
# _subdir The sub-directory for these include files.
# ARGN The include files.
macro(PCL_ADD_INCLUDES _component _subdir)
    install(FILES ${ARGN} DESTINATION ${INCLUDE_INSTALL_DIR}/${_subdir}
        COMPONENT ${_component})
endmacro(PCL_ADD_INCLUDES)


###############################################################################
# Add a library target.
# _name The library name.
# _component The part of PCL that this library belongs to.
# ARGN The source files for the library.
macro(PCL_ADD_LIBRARY _name _component)
    if(PCL_SHARED_LIBS)
        set(_lib_type "SHARED")
    else(PCL_SHARED_LIBS)
        set(_lib_type "STATIC")
    endif(PCL_SHARED_LIBS)
    add_library(${_name} ${_lib_type} ${ARGN})
    set_target_properties(${_name} PROPERTIES
        VERSION ${PCL_VERSION}
        SOVERSION ${PCL_MAJOR_VERSION})
    install(TARGETS ${_name} LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT ${_component})
endmacro(PCL_ADD_LIBRARY)


###############################################################################
# Add an executable target.
# _name The executable name.
# _component The part of PCL that this library belongs to.
# ARGN the source files for the library.
macro(PCL_ADD_EXECUTABLE _name _component)
    add_executable(${_name} ${ARGN})
    set(PCL_EXECUTABLES ${PCL_EXECUTABLES} ${_name})
    install(TARGETS ${_name} RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        COMPONENT ${_component})
endmacro(PCL_ADD_EXECUTABLE)


###############################################################################
# Add compile flags to a target (because CMake doesn't provide something so
# common itself).
# _name The target name.
# _flags The new compile flags to be added, as a string.
macro(PCL_ADD_CFLAGS _name _flags)
    get_target_property(_current_flags ${_name} COMPILE_FLAGS)
    if(NOT _current_flags)
        set_target_properties(${_name} PROPERTIES COMPILE_FLAGS ${_flags})
    else(NOT _current_flags)
        set_target_properties(${_name} PROPERTIES
            COMPILE_FLAGS "${_current_flags} ${_flags}")
    endif(NOT _current_flags)
endmacro(PCL_ADD_CFLAGS)


###############################################################################
# Add link flags to a target (because CMake doesn't provide something so
# common itself).
# _name The target name.
# _flags The new link flags to be added, as a string.
macro(PCL_ADD_LINKFLAGS _name _flags)
    get_target_property(_current_flags ${_name} LINK_FLAGS)
    if(NOT _current_flags)
        set_target_properties(${_name} PROPERTIES LINK_FLAGS ${_flags})
    else(NOT _current_flags)
        set_target_properties(${_name} PROPERTIES
            LINK_FLAGS "${_current_flags} ${_flags}")
    endif(NOT _current_flags)
endmacro(PCL_ADD_LINKFLAGS)


###############################################################################
# Make a pkg-config file for a library. Do not include general PCL stuff in the
# arguments; they will be added automaticaly.
# _name The library name. "pcl_" will be preprended to this.
# _component The part of PCL that this pkg-config file belongs to.
# _desc Description of the library.
# _ext_deps External dependencies, as a space-separated string of items.
# _int_deps Internal dependencies, as a space-separated string of items.
# _cflags Compiler flags necessary to build with the library.
# _lib_flags Linker flags necessary to link to the library.
macro(PCL_MAKE_PKGCONFIG _name _component _desc _ext_deps _int_deps _cflags
        _lib_flags)
    set(PKG_NAME ${_name})
    set(PKG_DESC ${_desc})
    set(PKG_CFLAGS ${_cflags})
    set(PKG_LIBFLAGS ${_lib_flags})
    set(PKG_EXTERNAL_DEPS ${_ext_deps})
    set(PKG_INTERNAL_DEPS "")
    foreach(_dep ${_int_deps})
        set(PKG_INTERNAL_DEPS "${PKG_INTERNAL_DEPS} -l${_dep}")
    endforeach(_dep)

    set(_pc_file ${CMAKE_CURRENT_BINARY_DIR}/${_name}.pc)
    configure_file(${PROJECT_SOURCE_DIR}/cmake/pkgconfig.cmake.in ${_pc_file}
        @ONLY)
    install(FILES ${_pc_file} DESTINATION ${PKGCFG_INSTALL_DIR}
        COMPONENT ${_component})
endmacro(PCL_MAKE_PKGCONFIG)


###############################################################################
# PRIVATE

###############################################################################
# Reset the subsystem status map.
macro(PCL_RESET_MAPS)
    set(PCL_SUBSYS_STATUS "" CACHE INTERNAL
        "To build or not to build, that is the question." FORCE)
    set(PCL_SUBSYS_REASONS "" CACHE INTERNAL
        "But why?" FORCE)
    set(PCL_SUBSYSTEMS "" CACHE INTERNAL "Internal list of subsystems" FORCE)
endmacro(PCL_RESET_MAPS)


###############################################################################
# Set the status of a subsystem.
# _name Subsystem name.
# _status TRUE if being built, FALSE otherwise.
# ARGN[0] Reason for not building.
macro(PCL_SET_SUBSYS_STATUS _name _status)
    if(${ARGC} EQUAL 3)
        set(_reason ${ARGV2})
    else(${ARGC} EQUAL 3)
        set(_reason "No reason")
    endif(${ARGC} EQUAL 3)
    SET_IN_GLOBAL_MAP(PCL_SUBSYS_STATUS ${_name} ${_status})
    SET_IN_GLOBAL_MAP(PCL_SUBSYS_REASONS ${_name} ${_reason})
endmacro(PCL_SET_SUBSYS_STATUS)


###############################################################################
# Write a report on the build/not-build status of the subsystems
macro(PCL_WRITE_STATUS_REPORT)
    message(STATUS "The following subsystems will be built:")
    foreach(_ss ${PCL_SUBSYSTEMS})
        GET_IN_MAP(_status PCL_SUBSYS_STATUS ${_ss})
        if(_status)
            message(STATUS "  ${_ss}")
        endif(_status)
    endforeach(_ss)

    message(STATUS "The following subsystems will not be built:")
    foreach(_ss ${PCL_SUBSYSTEMS})
        GET_IN_MAP(_status PCL_SUBSYS_STATUS ${_ss})
        if(NOT _status)
            GET_IN_MAP(_reason PCL_SUBSYS_REASONS ${_ss})
            message(STATUS "  ${_ss}: ${_reason}")
        endif(NOT _status)
    endforeach(_ss)
endmacro(PCL_WRITE_STATUS_REPORT)

