#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "sgl.h"
#include "sgl_anim.h"
#include "sgl_font.h"

#define LCD_HOST SPI2_HOST
#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 19
#define PIN_NUM_CS 17
#define PIN_NUM_DC 5
#define PIN_NUM_RST 18

#define TFT_WIDTH 240
#define TFT_HEIGHT 280

static spi_device_handle_t
spi;

static unsigned int
frames;

static sgl_color_t
panel_buffer[TFT_WIDTH * 4];

static void
tft_write_cmd(uint8_t value)
{
    esp_err_t ret;
    spi_transaction_t t = {};

    t.length = 8;
    t.tx_buffer = &value;

    gpio_set_level(PIN_NUM_DC, 0);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

static void
tft_write_data(uint8_t value)
{
    esp_err_t ret;
    spi_transaction_t t = {};

    t.length = 8;
    t.tx_buffer = &value;

    gpio_set_level(PIN_NUM_DC, 1);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

static void
tft_write_data16(uint16_t value)
{
    esp_err_t ret;
    spi_transaction_t t = {};

    t.length = 16;
    t.tx_buffer = (uint8_t []){value >> 8, value};

    gpio_set_level(PIN_NUM_DC, 1);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

static void
tft_set_win(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* Set column address */
    tft_write_cmd(0x2a);
	tft_write_data16(x1);
	tft_write_data16(x2);

    /* Set row address */
    tft_write_cmd(0x2b);
	tft_write_data16(y1 + 20);
	tft_write_data16(y2 + 20);

    /* Write frame buffer */
    tft_write_cmd(0x2c);
}

static void
tft_init_sequence(void)
{
	tft_write_cmd(0x11);
    vTaskDelay(120 / portTICK_PERIOD_MS);

    /* Rotation */
	tft_write_cmd(0x36);
	tft_write_data(0x00);

	tft_write_cmd(0x3a);
	tft_write_data(0x05);

    /* Frame rate setting */
	tft_write_cmd(0xb2);
	tft_write_data(0x0c);
	tft_write_data(0x0c);
	tft_write_data(0x00);
	tft_write_data(0x33);
	tft_write_data(0x33);

	tft_write_cmd(0xb7);
	tft_write_data(0x35);

    /* Power setting */
	tft_write_cmd(0xbb);
	tft_write_data(0x32);

    tft_write_cmd(0xc2);
	tft_write_data(0x01);

    tft_write_cmd(0xc3);
	tft_write_data(0x15);

    tft_write_cmd(0xc4);
	tft_write_data(0x20);

    tft_write_cmd(0xc6);
	tft_write_data(0x0f);

	tft_write_cmd(0xd0);
	tft_write_data(0xa4);
	tft_write_data(0xa1);

	tft_write_cmd(0xe0);
	tft_write_data(0xd0);
	tft_write_data(0x08);
	tft_write_data(0x0e);
	tft_write_data(0x09);
	tft_write_data(0x09);
	tft_write_data(0x05);
	tft_write_data(0x31);
	tft_write_data(0x33);
	tft_write_data(0x48);
	tft_write_data(0x17);
	tft_write_data(0x14);
	tft_write_data(0x15);
	tft_write_data(0x31);
	tft_write_data(0x34);

	tft_write_cmd(0xe1);
	tft_write_data(0xd0);
	tft_write_data(0x08);
	tft_write_data(0x0e);
	tft_write_data(0x09);
	tft_write_data(0x09);
	tft_write_data(0x15);
	tft_write_data(0x31);
	tft_write_data(0x33);
	tft_write_data(0x48);
	tft_write_data(0x17);
	tft_write_data(0x14);
	tft_write_data(0x15);
	tft_write_data(0x31);
	tft_write_data(0x34);

	tft_write_cmd(0x21);
    vTaskDelay(20 / portTICK_PERIOD_MS);

	tft_write_cmd(0x29);
    vTaskDelay(20 / portTICK_PERIOD_MS);
}

static void
tft_init(void)
{
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    tft_init_sequence();
}

static void
tft_flush_area(int16_t x, int16_t y, int16_t w, int16_t h, sgl_color_t *src)
{
    esp_err_t ret;
    spi_transaction_t t = {};

    tft_set_win(x, y, x + w - 1, y + h - 1);

    t.length = w * h * sizeof(sgl_color_t) * 8;
    t.tx_buffer = src;

    gpio_set_level(PIN_NUM_DC, 1);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

static void
logger(const char *str)
{
    printf("%s\n", str);
}

static void
tick_timer_callback(void* arg)
{
    sgl_anim_tick_inc(10);
}

static void
info_timer_callback(void* arg)
{
    static unsigned int last;

    printf("Render: %ufps\n", frames - last);
    last = frames;
}

static void
demo_anim_path(struct sgl_anim *anim, int32_t value)
{
    sgl_obj_set_pos(anim->data, value, value);
}

static void
demo_anim_finish(struct sgl_anim *anim)
{
    uint16_t tmp;

    tmp = anim->start_value;
    anim->start_value = anim->end_value;
    anim->end_value = tmp;
}

void
app_main(void)
{
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = sizeof(panel_buffer)
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000,
        .spics_io_num = PIN_NUM_CS,
        .mode = 0,
        .queue_size = 7,
    };

    // Initialize the SPI bus
    ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = ((1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_RST));
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Initialize the LCD
    tft_init();

    sgl_device_fb_t fbdev = {
        .xres = TFT_WIDTH,
        .yres = TFT_HEIGHT,
        .xres_virtual = TFT_WIDTH,
        .yres_virtual = TFT_HEIGHT,
        .flush_area = tft_flush_area,
        .buffer[0] = panel_buffer,
        .buffer_size = SGL_ARRAY_SIZE(panel_buffer),
    };

    sgl_device_fb_register(&fbdev);
    sgl_device_log_register(logger);
    sgl_init();

    sgl_obj_t *rect1 = sgl_rect_create(NULL);
    sgl_obj_set_pos(rect1, 0, 0);
    sgl_obj_set_size(rect1, 50, 50);
    sgl_obj_set_color(rect1, SGL_COLOR_GRAY);
    sgl_obj_set_border_color(rect1, SGL_COLOR_GREEN);
    sgl_obj_set_border_width(rect1, 3);
    sgl_obj_set_radius(rect1, 10);
    sgl_obj_set_alpha(rect1, 100);

    sgl_obj_t *rect2 = sgl_rect_create(NULL);
    sgl_obj_set_pos(rect2, 0, 0);
    sgl_obj_set_size(rect2, 50, 50);
    sgl_obj_set_color(rect2, SGL_COLOR_BRIGHT_PURPLE);
    sgl_obj_set_border_color(rect2, SGL_COLOR_GREEN);
    sgl_obj_set_border_width(rect2, 3);
    sgl_obj_set_radius(rect2, 10);
    sgl_obj_set_alpha(rect2, 100);

    sgl_anim_t *anim1 = sgl_anim_create();
    sgl_anim_set_data(anim1, rect1);
    sgl_anim_set_act_duration(anim1, 1200);
    sgl_anim_set_start_value(anim1, 10);
    sgl_anim_set_end_value(anim1, TFT_WIDTH - 50);
    sgl_anim_set_path(anim1, demo_anim_path, SGL_ANIM_PATH_LINEAR);
    sgl_anim_set_repeat_cnt(anim1, -1);
    anim1->finish_cb = demo_anim_finish;

    sgl_anim_t *anim2 = sgl_anim_create();
    sgl_anim_set_data(anim2, rect2);
    sgl_anim_set_act_duration(anim2, 1200);
    sgl_anim_set_start_value(anim2, 50);
    sgl_anim_set_end_value(anim2, TFT_WIDTH - 50);
    sgl_anim_set_path(anim2, demo_anim_path, SGL_ANIM_PATH_LINEAR);
    sgl_anim_set_repeat_cnt(anim2, -1);
    anim2->finish_cb = demo_anim_finish;

    sgl_anim_start(anim1);
    sgl_anim_start(anim2);

    const esp_timer_create_args_t
    tick_timer_args = {
        .callback = &tick_timer_callback,
        .name = "sgl_tick"
    };

    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 10000));

    const esp_timer_create_args_t
    info_timer_args = {
        .callback = &info_timer_callback,
        .name = "info_tick"
    };

    esp_timer_handle_t info_timer;
    ESP_ERROR_CHECK(esp_timer_create(&info_timer_args, &info_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(info_timer, 1000000));

    for (;;) {
        vTaskDelay(1);
        sgl_task_handle();
        frames++;
    }
}
