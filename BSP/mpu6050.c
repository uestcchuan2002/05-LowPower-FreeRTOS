#include "mpu6050.h"

// MPU6050 的默认 7 位 I2C 地址 (AD0 引脚接地时)
#define MPU6050_HW_ADDR 0x68 

#define Gyro_Range  0x18 // ±2000 dps
#define Accel_Range 0x18 // ±16g

/**
 * @brief 使用硬件 IIC 向 MPU6050 写入一个字节
 * @param RegAddress 寄存器地址
 * @param Data 要写入的数据
 */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data) 
{
    // 这一句代码，完美等效于你原来的 8 行代码！
    // 它是由 STM32 芯片内部的硬件 I2C 控制器自动发出 Start、寻址、等待 ACK 并发出 Stop 的。
    // 超时时间设为 50ms，防止总线卡死。
    HAL_I2C_Mem_Write(&hi2c2, (MPU6050_HW_ADDR << 1), RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, 50);
}

/**
 * @brief 使用硬件 IIC 从 MPU6050 读取一个字节
 * @param RegAddress 寄存器地址
 * @return 读到的数据
 */
uint8_t MPU6050_ReadReg(uint8_t RegAddress) 
{
    uint8_t Data = 0;
    
    // 这一行代码等效于你原来的 11 行裸机代码！
    // 它会自动执行：Start -> 写物理地址 -> 写寄存器 -> Restart -> 读物理地址 -> 接收数据 -> NACK -> Stop
    HAL_I2C_Mem_Read(&hi2c2, (MPU6050_HW_ADDR << 1), RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, 50);
    
    return Data;
}

// 设置工作模式
void MPU6050_SetPowerMode(uint8_t mode) 
{
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, mode);
}

// 配置采样率
void MPU6050_SetSampleRate(uint8_t div) 
{
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, div);
}

// 启用传感器
void MPU6050_EnableSensors(void) 
{
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x00); // 启用设备
}

// ==========================================
// 极速版：连续读取加速度计
// ==========================================
void MPU6050_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az) 
{
    uint8_t buffer[6]; 
    
    // 💡 绝杀优化：只发送1次指令，连续索要 6 个字节的数据！
    // 耗时只有原来的 1/6，且绝对保证 XYZ 三轴数据属于同一个采样时刻！
    HAL_I2C_Mem_Read(&hi2c2, (MPU6050_HW_ADDR << 1), MPU6050_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, buffer, 6, 50);

    // 位运算组合数据保持不变
    *ax = (buffer[0] << 8) | buffer[1]; 
    *ay = (buffer[2] << 8) | buffer[3]; 
    *az = (buffer[4] << 8) | buffer[5]; 
}

// ==========================================
// 极速版：连续读取陀螺仪
// ==========================================
void MPU6050_ReadGyro(int16_t *gx, int16_t *gy, int16_t *gz) 
{
    uint8_t buffer[6]; 
    
    // 从 GYRO_XOUT_H 开始，一口气读完 6 字节
    HAL_I2C_Mem_Read(&hi2c2, (MPU6050_HW_ADDR << 1), MPU6050_GYRO_XOUT_H, I2C_MEMADD_SIZE_8BIT, buffer, 6, 50);

    *gx = (buffer[0] << 8) | buffer[1]; 
    *gy = (buffer[2] << 8) | buffer[3]; 
    *gz = (buffer[4] << 8) | buffer[5]; 
}

float MPU6050_ConvertAccel(int16_t value, uint8_t range) 
{
    float factor; // 用于存储转换因子
    // 根据加速度计的量程选择合适的转换因子
    switch (range) {
        case 0x00: factor = 16384.0; break; // ±2g，转换因子为16384
        case 0x08: factor = 8192.0; break;  // ±4g，转换因子为8192
        case 0x10: factor = 4096.0; break;  // ±8g，转换因子为4096
        case 0x18: factor = 2048.0; break;  // ±16g，转换因子为2048
        default: factor = 16384.0; break;    // 默认量程范围为±2g
    }
    // 将原始加速度值转换为g单位
    return value / factor;
}
 
float MPU6050_ConvertGyro(int16_t value, uint8_t range) 
{
    float factor; // 用于存储转换因子
    // 根据陀螺仪的量程选择合适的转换因子
    switch (range) {
        case 0x00: factor = 131.0; break;  // ±250°/s，转换因子为131
        case 0x08: factor = 65.5; break;   // ±500°/s，转换因子为65.5
        case 0x10: factor = 32.8; break;   // ±1000°/s，转换因子为32.8
        case 0x18: factor = 16.4; break;   // ±2000°/s，转换因子为16.4
        default: factor = 131.0; break;     // 默认量程范围为±250°/s
    }
    // 将原始陀螺仪值转换为°/s单位
    return value / factor;
}

/**
 * @brief 初始化硬件MPU6050
 * @retval 0:成功, 1:失败(未检测到设备)
 */
uint8_t MPU6050_Init(void)
{
    // 1. IIC_Init(); 
    // 【注意】如果你用的是 CubeMX，MX_I2C2_Init() 已经在 main.c 里调用过了，这里可以直接删掉这句。


    // 2. 检测设备是否在位 (WHO_AM_I 寄存器地址是 0x75，默认返回值是 0x68)
    if (MPU6050_ReadReg(0x75) != 0x68)
    {
        return 1; // 初始化失败，找不到 MPU6050！
    }

    // 3. 复位设备
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x80); // 最高位写1，设备复位
    osDelay(50); // 【关键】等待复位完成

    // 4. 唤醒并设置时钟源为 X轴陀螺仪 (官方推荐，比内部振荡器准)
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01); 

    // 5. 使能3轴加速度和3轴陀螺仪 (全部写 0 开启)
    MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00); 
    
    // 6. 设置采样分频器和低通滤波
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09); // 设置采样分频器为10
    MPU6050_WriteReg(MPU6050_CONFIG, 0x06);     // 设置数字低通滤波器
    
    // 7. 配置量程
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, Gyro_Range);  // ±2000dps
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, Accel_Range);// ±16g
    
    return 0; // 初始化成功
}

/**
 * @brief 一次性读取并转换所有传感器数据 (浮点数)
 */
void MPU6050_ReadSensors(float *ax, float *ay, float *az, float *gx, float *gy, float *gz) 
{
    int16_t raw_ax, raw_ay, raw_az;
    int16_t raw_gx, raw_gy, raw_gz;

    // 极速突发读取原始数据 (上一回合优化的连续读取)
    MPU6050_ReadAccel(&raw_ax, &raw_ay, &raw_az);
    MPU6050_ReadGyro(&raw_gx, &raw_gy, &raw_gz);

    // 将加速度计的原始数据转换为浮点数形式（单位：g）
    *ax = MPU6050_ConvertAccel(raw_ax, Accel_Range); 
    *ay = MPU6050_ConvertAccel(raw_ay, Accel_Range); 
    *az = MPU6050_ConvertAccel(raw_az, Accel_Range); 
    
    // 将陀螺仪的原始数据转换为浮点数形式（单位：°/s）
    *gx = MPU6050_ConvertGyro(raw_gx, Gyro_Range); 
    *gy = MPU6050_ConvertGyro(raw_gy, Gyro_Range); 
    *gz = MPU6050_ConvertGyro(raw_gz, Gyro_Range); 
}

// 让传感器进入深度休眠 (功耗降至约 5μA)
void MPU6050_Sleep(void)
{
    // 0x40 即 0100 0000，将 SLEEP 位置 1
    MPU6050_WriteReg(0x6B, 0x40); 
}

// 唤醒传感器 (恢复至约 4mA 工作电流)
void MPU6050_WakeUp(void)
{
    // 0x00 即全 0，清除 SLEEP 位并使用内部时钟
    MPU6050_WriteReg(0x6B, 0x00);
    // 极其关键：传感器刚醒来时内部电路需要稳定，必须给它一点时间！
    osDelay(30); 
}

