# Printer module unit tests

add_executable(test_printer
    test_printer_main.c
    test_pdfwriter.c
    test_escp.c
    test_printjob.c
)

target_link_libraries(test_printer PRIVATE ndlib m)

target_include_directories(test_printer PRIVATE
    ${CMAKE_SOURCE_DIR}/src/ndlib
)

add_test(NAME printer_tests COMMAND test_printer)
