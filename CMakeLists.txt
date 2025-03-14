cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

SET(CMAKE_C_FLAGS "-std=c99") 

add_compile_definitions(_WIN_)

project(svg LANGUAGES C)

find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME} STATIC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../TigorEngine/includes includes)
target_sources(${PROJECT_NAME} PRIVATE
                            src/core/cxconfig.c
                            src/core/cxdefs.c
                            src/core/cxgrptable.c 
                            src/core/cxlist.c 
                            src/core/cxliteral.c 
                            src/core/cxlrucache.c 
                            src/core/cxmem.c 
                            src/core/cxmset.c 
                            src/core/cxstack.c 
                            src/core/cxstr.c 
                            src/core/cxtable.c 
                            src/cxml/cxml.c 
                            src/query/cxqapi.c 
                            src/query/cxql.c 
                            src/sax/cxsax.c 
                            src/utils/cxutf8hook.c 
                            src/utils/cxutils.c 
                            src/xml/cxlexer.c 
                            src/xml/cxparser.c 
                            src/xml/cxprinter.c 
                            src/xml/cxscope.c 
                            src/xml/cxstream.c 
                            src/xpath/cxxpcontext.c 
                            src/xpath/cxxpdata.c 
                            src/xpath/cxxpeval.c 
                            src/xpath/cxxplexer.c 
                            src/xpath/cxxplib.c 
                            src/xpath/cxxpmemdeb.c 
                            src/xpath/cxxpopt.c 
                            src/xpath/cxxpparser.c 
                            src/xpath/cxxpresolver.c
                            src/svg.c
                            src/svg_errors.c
                            src/svg_parser_circle.c
                            src/svg_parser_desc.c
                            src/svg_parser_ellipse.c
                            src/svg_parser_group.c
                            src/svg_parser_line.c
                            src/svg_parser_path.c
                            src/svg_parser_polygon.c
                            src/svg_parser_polyline.c
                            src/svg_parser_rect.c
                            src/svg_parser_title.c
                            src/svg_parser.c
                            src/svg_string.c
                            src/svg_types.c
                            src/svg_xml.c)
                            
enable_testing()
add_executable(TestDraw src/main.c)
target_link_libraries(TestDraw ${CMAKE_CURRENT_SOURCE_DIR}/../TigorEngine/${CMAKE_BUILD_TYPE}/libTigorEngine.a ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/libsvg.a Vulkan::Vulkan)
add_test(NAME Test2
		COMMAND TestDraw)
                            
