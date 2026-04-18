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

# HDLC module unit tests

add_executable(test_hdlc
    test_hdlc_main.c
    test_tcp_receive_buffer.c
    test_hdlc_crc.c
    test_hdlc_frame.c
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc/tcpReceiveBuffer.c
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc/hdlcFrame.c
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc/hdlc_crc.c
)

target_include_directories(test_hdlc PRIVATE
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/ndlib
    ${CMAKE_SOURCE_DIR}/src/cpu
)

target_link_libraries(test_hdlc PRIVATE m)

add_test(NAME hdlc_tests COMMAND test_hdlc)
