#include "I2C_Driver.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_PORT_NUM I2C_NUM_0
#define I2C_TIMEOUT_MS 1000


void I2C_Init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .clk_flags = 0
    };
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_PORT_NUM, &conf);
    i2c_driver_install(I2C_PORT_NUM, conf.mode, 0, 0, 0);
}


bool I2C_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (Driver_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, Reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (Driver_addr << 1) | I2C_MASTER_READ, true);
    if (Length > 1) {
        i2c_master_read(cmd, Reg_data, Length - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, Reg_data + Length - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE("I2C", "The I2C transmission fails. - I2C Read");
        return false;
    }
    return true;
}

bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (Driver_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, Reg_addr, true);
    for (int i = 0; i < Length; i++) {
        i2c_master_write_byte(cmd, Reg_data[i], true);
    }
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE("I2C", "The I2C transmission fails. - I2C Write");
        return false;
    }
    return true;
}