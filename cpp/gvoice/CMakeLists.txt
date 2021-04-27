add_library(gvoice SHARED gvoice.cpp)

target_link_libraries(gvoice gmod-module-base)
target_compile_definitions(gvoice PRIVATE FMT_HEADER_ONLY)
target_include_directories(gvoice PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Gmod things
set_gmod_suffix_prefix(gvoice)

# CLR things
set_target_properties(gvoice PROPERTIES COMMON_LANGUAGE_RUNTIME "")
set_target_properties(gvoice PROPERTIES VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6")
set_target_properties(gvoice PROPERTIES VS_DOTNET_REFERENCES "System")