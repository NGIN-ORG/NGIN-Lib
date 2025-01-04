include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

CPMAddPackage(
    NAME ut
    VERSION 2.1.1
    GITHUB_REPOSITORY boost-ext/ut
    OPTIONS 
    "BOOST_UT_DISABLE_MODULE ON"
    "BOOST_UT_BUILD_EXAMPLES OFF"
    "BOOST_UT_BUILD_TESTS OFF"
)
add_executable(NGINLibTests
        "main.cpp"
        "Containers/String.cpp"
        "Meta/FunctionTraits.cpp"
        "Meta/TypeTraits.cpp"
        "Utils/LSBFlag.cpp"
        "Utils/MSBFlag.cpp"

)

set_target_properties(NGINLibTests PROPERTIES
CXX_STANDARD 23
CXX_STANDARD_REQUIRED ON
CXX_EXTENSIONS OFF
)

target_link_libraries(NGINLibTests PRIVATE Boost::ut NGIN::NGINLib)

add_test(NAME NGINLibTests COMMAND NGINLibTests)
