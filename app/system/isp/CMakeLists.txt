aux_source_directory(. DIR_APP_SRC)

add_library(isp ${DIR_APP_SRC})

target_link_libraries(isp mi_isp mi_iqserver fbc_decode)

