include(FetchContent)
FetchContent_Declare(
        doxygen-awesome-css
        URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(doxygen-awesome-css)
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

find_package(Doxygen REQUIRED)

#configure_file("index.md.in" "index.md" @ONLY)

set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")
set(DOXYGEN_MARKDOWN_ID_STYLE "GITHUB")
set(DOXYGEN_GENERATE_LATEX "NO")
set(DOXYGEN_CLASS_GRAPH "NO")
set(DOXYGEN_GENERATE_TREEVIEW "YES")
set(DOXYGEN_DISABLE_INDEX "NO")
set(DOXYGEN_FULL_SIDEBAR "NO")
set(DOXYGEN_PAGE_OUTLINE_PANEL "NO")
set(DOXYGEN_HTML_EXTRA_STYLESHEET "${AWESOME_CSS_DIR}/doxygen-awesome.css" "${AWESOME_CSS_DIR}/doxygen-awesome-sidebar-only.css")
set(DOXYGEN_HTML_COLORSTYLE "TOGGLE")
#set(DOXYGEN_HTML_COLORSTYLE "LIGHT")

doxygen_add_docs(
        my_doc
        "${PROJECT_SOURCE_DIR}/README.md"
        "${PROJECT_SOURCE_DIR}/docs"
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/include"
)
