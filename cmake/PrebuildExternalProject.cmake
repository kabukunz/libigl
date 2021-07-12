# https://stackoverflow.com/questions/6351609/cmake-linking-to-library-downloaded-from-externalproject-add
# https://gist.github.com/amir-saniyan/4339e6f3ef109c75eda8018f7d5192a7

# this way is possible to add prebuilt projects with find_package
# during configure time using ExternalProject_add

# 
# this was created because I found impossible to use mmg2d with
# add_subdirectory and find_package(CONFIG). Could only be used
# as prebuilt library (as per its own docs, anyway). 
# 
# It has then grown into a way to manage a per-project quick 
# install directory with external libraries without rebuilding
# them from source every time, in case you have to regenerate
# your project for dev testing over and over while using always
# the same libraries. Those supposedly should be in your /usr/local,
# but I found to be difficult to manage multiple version libs while
# using Brew et al, for instance. Thus this.
# 
# remember execute_process runs prior to build system generation
# 

# --------------------------------------------------
function (prebuild_external_project
        target 
        location 
        hashing
        platform
        version
        source_dir 
        binary_dir 
        install_dir 
        install_cmd
        is_prebuilt
        )

    if(NOT DEFINED target)
        message(FATAL_ERROR "Tool name is undefined")
    endif()

    if(NOT DEFINED location)
        message(FATAL_ERROR "Tool location is undefined")
    endif()

    # check location and hash
    string(FIND ${location} "tar" IS_TAR)
    string(FIND ${location} "gz" IS_GZIP)
    string(FIND ${location} "zip" IS_ZIP)
    if((IS_TAR GREATER_EQUAL 0) OR (IS_GZIP GREATER_EQUAL 0) OR (IS_ZIP GREATER_EQUAL 0))
        set(location "URL ${location}")
        if(NOT hashing)
            set(hashing "")
        else()
            string(HEX ${hashing} hex_hashing)
            set(hashing "URL_MD5 ${hex_hashing}")
        endif()
    else()
        set(location "GIT_REPOSITORY ${location}")
        set(hashing "GIT_TAG ${hashing}")
    endif()

    # check platform
    if(platform)
        set(target "${target}${platform}")
        if(source_dir)        
            set(source_dir "${source_dir}/${target}")
        endif()
        if(binary_dir)        
            set(binary_dir "${binary_dir}/${target}")
        endif()
        if(install_dir)
            set(install_dir "${install_dir}/${target}")
        endif()
    endif()

    # check version
    if(version)
        set(target "${target}${version}")
        if(source_dir)        
            set(source_dir "${source_dir}/${target}")
        endif()
        if(binary_dir)        
            set(binary_dir "${binary_dir}/${target}")
        endif()
        if(install_dir)
            set(install_dir "${install_dir}/${target}")
        endif()
    endif()

    if(${target}_POPULATED)
        return()
    endif()
    
    # check if already built
    if(is_prebuilt)
        set(configure_cmd "CONFIGURE_COMMAND \"\"")
        set(build_cmd "BUILD_COMMAND \"\"")
        set(install_cmd "INSTALL_COMMAND \"\"")
    endif()

    # check install step
    if(NOT install_cmd)
        set(install_cmd "INSTALL_COMMAND \"\"")
    else()    
        unset(install_cmd)
        # check install prefix
        if(install_dir)
            list(APPEND ARGN "-D CMAKE_INSTALL_PREFIX=${install_dir}")
            # check if installing in prebuilt
            if(is_prebuilt)
                set(install_cmd "INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${install_dir}")
            endif()
        endif()            
    endif()    
    
    message(WARNING "TARGET = ${target}")
    message(WARNING "TARGET_POPULATED = ${target}_POPULATED")
    message(WARNING "SOURCE dir = ${source_dir}")
    message(WARNING "BINARY dir = ${binary_dir}")
    message(WARNING "INSTALL dir = ${install_dir}")
    message(WARNING "INSTALL cmd = ${install_cmd}")
    message(WARNING "IS_PREBUILT = ${IS_PREBUILT}")
    message(WARNING "OPTIONS = ${ARGN}")

    set(CMAKELIST_CONTENT "
        cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

        project(build_external_project)

        include(ExternalProject)

        ExternalProject_add(
            ${target}
            SOURCE_DIR \"${source_dir}\"
            BINARY_DIR \"${binary_dir}\"
            ${location}
            ${hashing}
            CMAKE_GENERATOR \"${CMAKE_GENERATOR}\"
            CMAKE_GENERATOR_PLATFORM \"${CMAKE_GENERATOR_PLATFORM}\"
            CMAKE_GENERATOR_TOOLSET \"${CMAKE_GENERATOR_TOOLSET}\"
            CMAKE_GENERATOR_INSTANCE \"${CMAKE_GENERATOR_INSTANCE}\"
            CMAKE_ARGS ${ARGN}
            ${configure_cmd}
            ${build_cmd}
            ${install_cmd}
            )

        add_custom_target(build_external_project)
        
        add_dependencies(build_external_project ${target})
    ")

    set(TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/${target}")

    file(WRITE "${TARGET_DIR}/CMakeLists.txt" "${CMAKELIST_CONTENT}")

    file(MAKE_DIRECTORY "${TARGET_DIR}" "${TARGET_DIR}/build")

    if(EXISTS "${TARGET_DIR}/build_ok")
        message(STATUS "External build for ${target} already done")
        return()
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND}
        -G "${CMAKE_GENERATOR}"
        -A "${CMAKE_GENERATOR_PLATFORM}"
        -T "${CMAKE_GENERATOR_TOOLSET}"
        ..
        WORKING_DIRECTORY "${TARGET_DIR}/build"
        RESULT_VARIABLE STATUS)

    if(STATUS AND NOT STATUS EQUAL 0)
        message(FATAL_ERROR "Execute process command failed with: ${STATUS}")
    endif()        

    execute_process(COMMAND ${CMAKE_COMMAND}
        --build .
        --config ${CMAKE_BUILD_TYPE}
        WORKING_DIRECTORY "${TARGET_DIR}/build"
        RESULT_VARIABLE STATUS)

    if(STATUS AND NOT STATUS EQUAL 0)
        message(FATAL_ERROR "Execute process command failed with: ${STATUS}")
    endif()        

    file(WRITE "${TARGET_DIR}/build_ok")

endfunction()

# ----------------------------------------------------------------------------------------------------
