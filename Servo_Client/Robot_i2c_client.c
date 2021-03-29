#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#define I2C_BUS_AVAILABLE ( 5) //I2C Bus có sẵn trong Pi
#define SLAVE_DEVICE_NAME ("ETX_ROBOT") //tên TB và driver
#define ROBOT_SLAVE_ADDR  ( 0x40) //địa chỉ  I2C slave 
#define SUBADR1           ( 0x02) //đọc ghi I2C bus subaddress1
#define SUBADR2           ( 0x03) //đọc ghi I2C bus subaddress2
#define SUBADR3           ( 0x04) //đọc ghi I2C bus subaddress3
#define MODE1             ( 0x00) //đọc ghi thanh ghi chế độ 1
#define MODE2             ( 0x01) //đọc ghi thanh ghi chế độ 2
#define PRESCALE          ( 0xFE) //đọc ghi bộ định mức trước cho đầu ra tần số
#define LED0_ON_L         ( 0x06) //đọc ghi đầu ra LED0 và byte điều khiển độ sáng 0
#define LED0_ON_H         ( 0x07) //đọc ghi đầu ra LED0 và byte điều khiển độ sáng 1
#define LED0_OFF_L        ( 0x08) //đọc ghi đầu ra LED0 và byte điều khiển độ sáng 2
#define LED0_OFF_H        ( 0x09) //đọc ghi đầu ra LED0 và byte điều khiển độ sáng 3
#define ALLLED_ON_L       ( 0xFA) //tải tất cả LEDn_ON thanh ghi, byte 0
#define ALLLED_ON_H       ( 0xFB) //tải tất cả LEDn_ON thanh ghi, byte 1
#define ALLLED_OFF_L      ( 0xFC) //tải tất cả LEDn_ON thanh ghi, byte 0
#define ALLLED_OFF_H      ( 0xFD) //tải tất cả LEDn_ON thanh ghi, byte 1
#define Robot_I2C_Hz      ( 50)

//cấu trúc adapter
static struct i2c_adapter *etx_i2c_adapter     = NULL;
//cấu trúc I2C Client 
static struct i2c_client *etx_i2c_client_robot = NULL;

//Ghi dữ liệu vào client 
static int I2C_Write(unsigned char *buf, unsigned int len)
{
    //buf -> bộ đệm được gửi
    //gửi điều kiện bắt đầu, địa chỉ Slave với R/W bit
    //ĐK ACK/NACK và Stop sẽ được xử lý nội bộ
    int ret = i2c_master_send(etx_i2c_client_robot, buf, len);
    return ret;
}

//Đọc 1 byte dữ liệu từ client I2C
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    //đệm trong đó data được sao chép
    //gửi điều kiện bắt đầu, địa chỉ Slave với R/W bit
    //ĐK ACK/NACK và Stop sẽ được xử lý nội bộ
    int ret = i2c_master_recv(etx_i2c_client_robot, out_buf, len);
    return ret;
}
//i2c_master_send (thông báo 1 i2c duy nhất trong chế độ truyền chính)
//i2c_smbus_write_byte (gửi 1 byte đến slave)
//i2c_smbus_write_byte_data (byte đang được viết)
//i2c_smbus_write_word_data (từ 16 bit )
//i2c_smbus_write_block_data (mảng byte được ghi)
//i2c_master_recv (thông báo i2c duy nhất ở chế độ nhận chính)
//i2c_smbus_read_byte
//i2c_smbus_read_byte_data (byte đc diễn giải bởi slave)
//i2c_smbus_read_word_data (1 từ ko dấu 16 bit nhận từ slave)
//i2c_smbus_read_block_data(mảng byte mà dữ liệu sẽ đc đọc vào đó, mã 32 byte)
//i2c_transfer gửi nhiều thư i2c
// đoạncode chính

static void Robot_Write( bool is_cmd, unsigned char data)
{
    unsigned char buf[2] = {0};
    int ret;
    if (is_cmd == true)
    {
        buf[0] = 
    }
    else
    {
        buf[0] = 0x40;
    }
    buf[1] = data;
    ret = I2C_Write(buf, 2);
}



//cấu trúc có id thiết bị phụ
static const struct i2c_device_id etx_robot_id[] =
{
    { SLAVE_DEVICE_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, etx_robot_id);

//Cấu trúc driver I2C phải đc thêm vào linux
static struct i2c_driver etx_robot_driver =
{
    .driver = {
        .name  = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
    .probe    = etx_robot_probe,
    .remove   = etx_robot_remove,
    .id_table = etx_robot_id,
};

//I2C Board Info structure
static struct i2c_board_info robot_i2c_board_info =
{
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, ROBOT_SLAVE_ADDR) 
};

//Khởi tạo hàm
static int __init etx_driver_init(void)
{
    int ret = -1;
    etx_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if(etx_i2c_adapter != NULL)
    {
        etx_i2c_client_robot = i2c_new_device(etx_i2c_adapter, &robot_i2c_board_info);
        if (etx_i2c_client_robot != NULL)
        {
            i2c_add_driver(& etx_robot_driver);
            ret = 0;
        }
        i2c_put_adapter(etx_i2c_adapter);
    }

    pr_info("Driver Added !!! \n");
    return ret;
}

//chức năng thoát
static void __exit etx_driver_exit(void)
{
    i2c_unregister_device(etx_i2c_client_robot);
    i2c_del_driver(&etx_robot_driver);
    pr_info("Driver Removed !!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Quang Huy - K62");
MODULE_DESCRIPTION("I2C Client Driver");
MODULE_VERSION(" ");