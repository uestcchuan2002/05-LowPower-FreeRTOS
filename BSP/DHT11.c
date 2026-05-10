#include "DHT11.h"

unsigned int rec_data[4] = {0};  // 初始化，避免脏数据
uint8_t dht11_error_code = 0;    // 全局错误码，方便在 main 里面打印排错

// DHT11电源开启
void DHT11_Power_Open(void)
{
    HAL_GPIO_WritePin(DHT11_POWER_GPIO_Port, DHT11_POWER_Pin, GPIO_PIN_SET);
}

// DHT11电源关闭
void DHT11_Power_Close(void)
{
    HAL_GPIO_WritePin(DHT11_POWER_GPIO_Port, DHT11_POWER_Pin, GPIO_PIN_RESET);
}

// 统一的 GPIO 初始化：开漏输出 + 上拉 (替代原来的 IN 和 OUT 函数)
static void DH11_GPIO_Init_OD(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStructure.Pin   = DHT11_DATA_Pin;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD; // 关键修复：开漏输出模式
    GPIO_InitStructure.Pull  = GPIO_PULLUP;         // 内部上拉
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStructure);
}



// 获取一个字节（引入动态阈值）
static char DHT11_Rec_Byte(uint32_t threshold)
{
    unsigned char i = 0;
    unsigned char data = 0; 

    for(i=0; i<8; i++)
    {
        uint32_t timeout = 100000;
        // 等待低电平结束
        while( Read_Data() == 0 && timeout-- ) if(timeout==0) return 0;

        uint32_t high_time = 0;
        timeout = 100000;
        // 核心：计算高电平持续的时间（循环次数）
        while( Read_Data() == 1 && timeout-- )
        {
            high_time++;
            if(timeout==0) return 0;
        }

        data <<= 1;
        // 如果测得的高电平循环次数大于动态阈值，说明这是 '1'
        if( high_time > threshold )
        {
            data |= 1; 
        }
    }
    return data;
}

// 获取数据（主控函数）

void DHT11_REC_Data(void)
{
    // 把所有的变量声明统一放在最前面，解决 goto 跨越初始化的警告
    uint8_t R_H=0, R_L=0, T_H=0, T_L=0, CHECK=0;
    uint32_t wait_cnt = 800; 
    uint32_t timeout = 100000;
    uint32_t calib_80us = 0; 
    uint32_t threshold = 0;

    dht11_error_code = 0; 

    DH11_GPIO_Init_OD(); 
    
    // 主机拉低 20ms 发送起始信号
    dht11_low();
    osDelay(20); 

    __disable_irq(); // 保护微秒级时序

    // 主机释放总线
    dht11_high();
    
    // 短暂死循环延时等待 DHT11 响应
    while(wait_cnt--);

    // 等待 DHT11 拉低
    timeout = 100000;
    while( Read_Data() == 1 && timeout-- ) if(timeout==0) { dht11_error_code = 1; goto end_read; }
    
    // 等待 80us 低电平结束
    timeout = 100000;
    while( Read_Data() == 0 && timeout-- ) if(timeout==0) { dht11_error_code = 2; goto end_read; }
    
    // ==========================================
    // 高能技巧：测量 80us 高电平究竟需要多少次循环
    // ==========================================
    timeout = 100000;
    while( Read_Data() == 1 && timeout-- ) 
    {
        calib_80us++;
        if(timeout==0) { dht11_error_code = 3; goto end_read; }
    }

    // 计算阈值：0(28us) 和 1(70us) 的分割点大约是 40us，即 80us 的一半。
    threshold = calib_80us / 2;

    // 传入动态阈值，开始接收数据！
    R_H = DHT11_Rec_Byte(threshold);
    R_L = DHT11_Rec_Byte(threshold);
    T_H = DHT11_Rec_Byte(threshold);
    T_L = DHT11_Rec_Byte(threshold);
    CHECK = DHT11_Rec_Byte(threshold);

end_read:
    __enable_irq(); // 恢复中断

    dht11_high(); 

    // 校验数据
    if(dht11_error_code == 0) 
    {
        if( (R_H + R_L + T_H + T_L) == CHECK && CHECK != 0)
        {
            rec_data[0] = R_H;  
            rec_data[1] = R_L;  
            rec_data[2] = T_H;  
            rec_data[3] = T_L;  
        }
        else
        {
            dht11_error_code = 4; // 校验失败
        }
    }
}

