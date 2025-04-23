# ngl-pokepaste

ngl-pokepaste is a header-only [PokePaste](https://pokepast.es/syntax.html) encoder and decoder written in C++20.

The implementation is contained in the single header file `pokepaste.hpp`. Alternatively if using CMake the library can be consumed using FetchContent. eg:

```
include(FetchContent)

FetchContent_Declare(
    ngl-pokepaste
    GIT_REPOSITORY https://github.com/ningalu/ngl-pokepaste
    GIT_TAG master
    USES_TERMINAL_DOWNLOAD ON
)

FetchContent_GetProperties(ngl-pokepaste)
if(NOT ngl-pokepaste_POPULATED)
    FetchContent_Populate(ngl-pokepaste)
endif()

add_subdirectory(${ngl-pokepaste_SOURCE_DIR} ${ngl-pokepaste_BINARY_DIR})
```

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

See the [LICENSE](LICENSE.md) document.
