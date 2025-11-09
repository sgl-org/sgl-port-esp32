set(SGL_SOURCE_PATH
    ${CMAKE_SOURCE_DIR}/main/sgl/source
)

set(srcs
    "main.c"
    ${SGL_SOURCE_PATH}/widgets/2dball/sgl_2dball.c
    ${SGL_SOURCE_PATH}/widgets/textbox/sgl_textbox.c
    ${SGL_SOURCE_PATH}/widgets/msgbox/sgl_msgbox.c
    ${SGL_SOURCE_PATH}/widgets/dropdown/sgl_dropdown.c
    ${SGL_SOURCE_PATH}/widgets/checkbox/sgl_checkbox.c
    ${SGL_SOURCE_PATH}/widgets/slider/sgl_slider.c
    ${SGL_SOURCE_PATH}/widgets/icon/sgl_icon.c
    ${SGL_SOURCE_PATH}/widgets/unzip_image/sgl_unzip_image.c
    ${SGL_SOURCE_PATH}/widgets/arc/sgl_arc.c
    ${SGL_SOURCE_PATH}/widgets/circle/sgl_circle.c
    ${SGL_SOURCE_PATH}/widgets/label/sgl_label.c
    ${SGL_SOURCE_PATH}/widgets/rectangle/sgl_rectangle.c
    ${SGL_SOURCE_PATH}/widgets/keyboard/sgl_keyboard.c
    ${SGL_SOURCE_PATH}/widgets/ring/sgl_ring.c
    ${SGL_SOURCE_PATH}/widgets/progress/sgl_progress.c
    ${SGL_SOURCE_PATH}/widgets/button/sgl_button.c
    ${SGL_SOURCE_PATH}/widgets/numberkbd/sgl_numberkbd.c
    ${SGL_SOURCE_PATH}/widgets/textline/sgl_textline.c
    ${SGL_SOURCE_PATH}/widgets/led/sgl_led.c
    ${SGL_SOURCE_PATH}/widgets/listview/sgl_listview.c
    ${SGL_SOURCE_PATH}/widgets/switch/sgl_switch.c
    ${SGL_SOURCE_PATH}/widgets/line/sgl_line.c
    ${SGL_SOURCE_PATH}/fonts/sgl_ascii_consolas14.c
    ${SGL_SOURCE_PATH}/fonts/sgl_ascii_kai33.c
    ${SGL_SOURCE_PATH}/fonts/sgl_ascii_song23.c
    ${SGL_SOURCE_PATH}/fonts/sgl_ascii_consolas23.c
    ${SGL_SOURCE_PATH}/mm/other/sgl_mm.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_text.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_line.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_pie.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_bar.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_icon.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_ring.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_arc.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_circle.c
    ${SGL_SOURCE_PATH}/draw/sgl_draw_rect.c
    ${SGL_SOURCE_PATH}/core/sgl_log.c
    ${SGL_SOURCE_PATH}/core/sgl_anim.c
    ${SGL_SOURCE_PATH}/core/sgl_math.c
    ${SGL_SOURCE_PATH}/core/sgl_event.c
    ${SGL_SOURCE_PATH}/core/sgl_misc.c
    ${SGL_SOURCE_PATH}/core/sgl_core.c
)

set(includes
    "."
    ${SGL_SOURCE_PATH}
    ${SGL_SOURCE_PATH}/include
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${includes}
    PRIV_REQUIRES esp_timer esp_driver_spi esp_driver_gpio
)
