#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#define ADAPTER_NAME "ETX_I2C_ADAPTER"
#define SCL_GPIO 20
#define SDA_GPIO 21
#define I2C_DELAY usleep_range(5,10)

//đọc chân SCL GPIO
static bool ETX_I2C_Read_SCL(void)
{
    gpio_direction_input(SCL_GPIO);//đặt chiều tín hiệu
    return gpio_get_value(SCL_GPIO);
}

//đọc chân SDA GPIO
static bool ETX_I2C_Read_SDA(void)
{
    gpio_direction_input(SDA_GPIO);
    return gpio_get_value(SDA_GPIO);
}

//xóa SCL GPIO
static void ETX_I2C_Clear_SCL(void)
{
    gpio_direction_output(SCL_GPIO, 0);
    gpio_set_value(SCL_GPIO, 0);
}

//xóa SDA GPIO
static void ETX_I2C_Clear_SDA(void)
{
    gpio_direction_output(SDA_GPIO, 0);
    gpio_set_value(SDA_GPIO, 0);
}

//Đặt SCL GPIO 
static void ETX_I2C_Set_SCL(void)
{
    gpio_direction_output(SCL_GPIO, 1);
    gpio_set_value(SCL_GPIO, 1);
}

//Đặt SDA GPIO
static void ETX_I2C_Set_SDA(void)
{
    gpio_direction_output(SDA_GPIO, 1);
    gpio_set_value(SDA_GPIO, 1);
}

//Khởi tạo GPIO
static int ETX_I2C_Init(void)
{
    int ret = 0;
    do 
    {   
        //kiểm tra chân SCL có hợp lệ hay không
        // https://www.kernel.org/doc/html/latest/core-api/printk-basics.html

        if(gpio_is_valid(SCL_GPIO) == false)
        {
            pr_err("SCL GPIO %d is not valid\n", SCL_GPIO);
            ret = - 1;
            break;
        }

        //kiểm tra chân SDA có hợp lệ hay không
        if(gpio_is_valid(SDA_GPIO) == false)
        {
            pr_err("SDA GPIO %d is not valid\n", SDA_GPIO);
            ret = -1;
            break;
        }

        //yêu cầu SCL GPIO
        if(gpio_request(SCL_GPIO,"SCL_GPIO") < 0)
        {
            pr_err("ERROR: SCL GPIO %d request\n", SCL_GPIO);
            ret = -1;
            break;
        }

        if(gpio_request(SDA_GPIO, "SDA_GPIO") < 0)
        {
            pr_err("ERROR: SDA GPIO %d request\n", SDA_GPIO);
            gpio_free(SCL_GPIO);
            ret = -1;
            break;
        }

        // cấu hình SCL và SDA là đầu ra
        gpio_direction_output(SCL_GPIO, 1);
        gpio_direction_output(SDA_GPIO, 1);
    } while(false);

    return ret;
}

static void ETX_I2C_DeInit(void)
{
    gpio_free(SCL_GPIO);
    gpio_free(SDA_GPIO);
}

//gửi điều kiện start
static void ETX_I2C_Start(void)
{
    ETX_I2C_Set_SDA();
    ETX_I2C_Set_SCL();
    I2C_DELAY;
    ETX_I2C_Clear_SDA();
    I2C_DELAY;
    ETX_I2C_Clear_SCL();
    I2C_DELAY;
}

//gửi điều kiện stop
static void ETX_I2C_Stop(void)
{
    ETX_I2C_Clear_SDA()
    I2C_DELAY;
    ETX_I2C_Set_SCL();
    I2C_DELAY;
    ETX_I2C_Set_SDA();
    I2C_DELAY;
    ETX_I2C_Clear_SCL();
}

//đọc SDA để nhận trạng thái, trả về (0 cho NACK, 1 cho ACK)
//khi master gửi đ/c của slave, 1 slave nhận ra thì gửi ACk
//chủ biết nô lệ nó đang tiếp cận thực sự đang trên bus
static int ETX_I2C_Read_NACK_ACK(void)
{
    int ret = 1;
    //đọc ACK/NACK
    I2C_DELAY;
    ETX_I2C_Set_SCL();
    I2C_DELAY;
    if(ETX_I2C_Read_SDA()) // kiểm tra ACK/NACK
    {
        ret = 0
    }
    ETX_I2C_Clear_SCL();
    return ret;
}

//gửi 7 bit dịa chỉ đến slave, return 0 nếu thành công
static int ETX_I2C_Send_Addr (u8 byte, bool is_read)
{
    int ret = -1; //unsigned int
    u8 bit;
    u8 i = 0;
    u8 size = 7;

    //ghi 7bit slave địa chỉ
    for(i = 0; i < size; i++)
    {
        //lưu trữ giá trị MSB
        bit = ((byte >> (size - (i + 1))) & 0x01);
        //ghi giá trị MSB
        (bit) ? ETX_I2C_Set_SDA() : ETX_I2C_Clear_SDA();
        I2C_DELAY;
        ETX_I2C_Set_SCL();
        I2C_DELAY;
        ETX_I2C_Clear_SCL();
    }

    //ghi đọc/ghi bit (bit thứ 8)
    //read = 1, write = 0
    (is_read) ? ETX_I2C_Set_SDA() : ETX_I2C_Clear_SDA();
    I2C_DELAY;
    ETX_I2C_Set_SCL();
    I2C_DELAY;
    ETX_I2C_Clear_SCL();
    I2C_DELAY;

    if(ETX_I2C_Read_NACK_ACK())
    {
        ret = 0;// có ACK
    }
    return ret;
}

// gửi 1 byte slave, return 0 nếu thành công
static int ETX_I2C_Send_Byte(u8 byte)
{
    int ret = -1;
    u8 bit;
    u8 i = 0;
    u8 size = 7;

    //
    for(i = 0; i <= size; i++)
    {
        //lưu trữ giá trị MSB
        bit = ((byte >> (size - i)) & 0x01);
        //ghi giá trị MSB 
        (bit) ? ETX_I2C_Set_SDA() : ETX_I2C_Clear_SDA();
        I2C_DELAY;
        ETX_I2C_Set_SCL();
        I2C_DELAY;
        ETX_I2C_Clear_SCL();
    }
    if(ETX_I2C_Read_NACK_ACK() )
    {
        ret = 0; //có ACK
    }
    return ret;
}

// đọc 1 byte từ slave, return 0 nếu thành công 
static int ETX_ETX_Read_Byte(u8 *byte)
{
    int ret = 0;
    return ret;
}

//gửi số byte đến slave, return 0 nếu thành công
static int ETX_I2C_Send(u8 slave_addr, u8 *buf, u16 len)
{
    int ret = 0;
    u16 num = 0;
    do
    {   //gửi địa chỉ slave 
        if(ETX_I2C_Send_Addr(slave_addr, false) < 0)
        {
            pr_err("ERROR: ETX_I2C_Send_Byte - Slave Addr\n");
            ret = -1;
            break;
        }
        for(num = 0; num < len; num++)
        {
            //gửi các byte dữ liệu
            if(ETX_I2C_Send_Byte(buf[num]) < 0)7
            {
                pr_err("ERROR: ETX_I2C_Send_Byte - [Data = 0x%02x]\n", buf[num]);
                ret = -1;
                break;
            }
        }
    }while(false);

    return ret;
}

//đọc số byte từ slave, return 0 nếu thành công
static int ETX_I2C_Read(u8 slave_addr, u8 *buf, u16 len)
{
    int ret = 0;
    return ret;
}

//nhận các chức năng được hỗ trợ bởi bus driver 
static u32 etx_func(struct i2c_adapter *adapter)
{
    return (I2C_FUNC_I2C         |
            I2C_FUNC_SMBUS_QUICK |
            I2C_FUNC_SMBUS_BYTE  |
            I2C_FUNC_SMBUS_BYTE_DATA |
            I2C_FUNC_SMBUS_WORD_DATA |
            I2C_FUNC_SMBUS_BLOCK_DATA);
}

//gọi bất cứ khi nào bạn gọi I2C read, các API write
// ví dụ i2c_master_send (), i2c_master_recv 
static s32 etx_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    int i;
    s32 ret = 0;
    do 
    {
        //Init các GPIOs 
        if (ETX_I2C_Init() < 0)
        {
            pr_err("ERROR: ETX_I2C_Init\n");
            break;
        }
        //gửi tình trạng start
        ETX_I2C_Start();
        for(i = 0; i < num; i++)
        {
            struct i2c_msg *msg_temp = &msgs[i];
            if (ETX_I2C_Send(msg_temp->addr, msg_temp->buf, msg_temp->len) < 0)
            {
                ret = 0;
                break;
            }
            ret++;
#if 0
    pr_info("[Count: %d] [%s]: [Addr = 0x%x] [Len = %d] [Data] = ", i, __func__, msg_temp->addr, msg_temp->len);

    for(j = 0; j < msg_temp -> len; j++)
    {
        pr_cont("[0x%02x]", msg_temp ->buff[j]);
    }
#endif

        }

    }while(false);
    // gửi tình trạng stop
    ETX_I2C_Stop();
    // DeInit các GPIOs
    ETX_I2C_DeInit()

    return ret;
}

//gọi bất cứ khi nào gọi SMBUS read, write API
static s32 etx_smbus_xfer(  struct i2c_adapter *adap,
                            u16 addr,
                            unsigned short flags,
                            char read_write,
                            u8 command,
                            int size,
                            union i2c_smbus_data *data

                            )
{
    pr_info("In %s\n", __func__);
    return 0;
}

//I2C cấu trúc thuật toán
static struct i2c_algorithm etx_i2c_algorithm = 
{
    .smbus_xfer = etx_smbus_xfer,
    .master_xfer = etx_i2c_xfer,
    .functionality = etx_func,
};

//I2C cấu trúc adapter
static struct i2c_adapter etx_i2c_adapter =
{
    .owner = THIS_MODULE,
    .class = I2C_CLASS_HWMON, // I2C_CLASS_SPD
    .algo = &etx_i2c_algorithm,
    .name = ADAPTER_NAME,
    .nr = 5,
};

//chức năng Init 
static int __init etx_driver_init(void)
{
    int ret = -1;
    ret = i2c_add_number_adapter(&etx_i2c_adapter);
    pr_info("Bus Driver Added!!!\n");
    return ret;
}

//chức năng EXit
static void __exit etx_driver_exit(void)
{
    i2c_del_adapter(&etx_i2c_adapter);
    pr_info("Bus Driver Removed!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Quang Huy - K62");
MODULE_DESCRIPTION("I2C Bus driver");
MODULE_VERSION("   ");
