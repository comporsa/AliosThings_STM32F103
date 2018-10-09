/*
* Copyright (C) 2015-2017 Alibaba Group Holding Limited
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/aos.h>
#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/base.h>
#include "common.h"
#include "hal/sensor.h"


#define N2DM_I2C_ADDR1                 		(0x18)
#define N2DM_I2C_ADDR2                 		(0x19)
#define N2DM_I2C_ADDR_TRANS(n)         		((n)<<1)  
#define N2DM_I2C_ADDR                  		N2DM_I2C_ADDR_TRANS(N2DM_I2C_ADDR2)

#define N2DM_STATUS_REG_AUX        			0x07

#define N2DM_ACC_OUT_TEMP_L            		0x0C
#define N2DM_ACC_OUT_TEMP_H            		0x0D
#define N2DM_ACC_WHO_AM_I              		0x0F
#define N2DM_ACC_CTRL_REG0             		0x1E
#define N2DM_ACC_TEMP_CFG_REG          		0x1F

#define N2DM_ACC_CTRL_REG1             		0x20
#define N2DM_ACC_CTRL_REG2             		0x21
#define N2DM_ACC_CTRL_REG3             		0x22
#define N2DM_ACC_CTRL_REG4             		0x23
#define N2DM_ACC_CTRL_REG5             		0x24
#define N2DM_ACC_CTRL_REG6            		0x25
#define N2DM_ACC_REFERENCE            		0x26
#define N2DM_ACC_STATUS_REG           		0x27
#define N2DM_ACC_OUT_X_L              		0x28
#define N2DM_ACC_OUT_X_H              		0x29
#define N2DM_ACC_OUT_Y_L              		0x2A
#define N2DM_ACC_OUT_Y_H              		0x2B
#define N2DM_ACC_OUT_Z_L              		0x2C
#define N2DM_ACC_OUT_Z_H              		0x2D
#define N2DM_ACC_FIFO_CTRL_REG        		0x2E
#define N2DM_ACC_FIFO_SRC_REG         		0x2F
#define N2DM_ACC_INT1_CFG             		0x30
#define N2DM_ACC_INT1_SRC             		0x31
#define N2DM_ACC_INT1_THS             		0x32
#define N2DM_ACC_INT1_DURATION        		0x33
#define N2DM_ACC_INT2_CFG             		0x34
#define N2DM_ACC_INT2_SRC             		0x35
#define N2DM_ACC_INT2_THS             		0x36
#define N2DM_ACC_INT2_DURATION        		0x37
#define N2DM_ACC_CLICK_CFG            		0x38
#define N2DM_ACC_CLICK_SRC            		0x39
#define N2DM_ACC_CLICK_THS            		0x3A
#define N2DM_ACC_TIME_LIMIT           		0x3B
#define N2DM_ACC_TIME_LATENCY        		0x3C
#define N2DM_ACC_TIME_WINDOW          		0x3D
#define N2DM_ACC_ACT_THS              		0x3E
#define N2DM_ACC_ACT_DUR              		0x3F


#define N2DM_ACC_SELFTESTDISABLE           	(0x0)
#define N2DM_ACC_SELFTESTENABLE           	(0x2)
#define N2DM_ACC_SELFTEST_MSK          		(0x06)
#define N2DM_ACC_SELFTEST_POS           	(2)

#define N2DM_ACC_RANGE_2G               	(0x0)
#define N2DM_ACC_RANGE_4G               	(0x1)
#define N2DM_ACC_RANGE_8G               	(0x2)
#define N2DM_ACC_RANGE_16G              	(0x3)
#define N2DM_ACC_RANGE_MSK              	(0x30)
#define N2DM_ACC_RANGE_POS              	(4)

#define N2DM_ACC_SENSITIVITY_2G         	976
#define N2DM_ACC_SENSITIVITY_4G         	1953
#define N2DM_ACC_SENSITIVITY_8G         	3906
#define N2DM_ACC_SENSITIVITY_16G        	11718

#define N2DM_ACC_CHIP_ID_VALUE				(0x33)

#define N2DM_SHIFT_EIGHT_BITS           	(8)
#define N2DM_SHIFT_FOUR_BITS             	(4)

#define N2DM_16_BIT_SHIFT               	(0xFF)
#define N2DM_ACC_MUL                    	(1000)

#define N2DM_ACC_ODR_POWER_DOWN				(0x00)
#define N2DM_ACC_ODR_1_HZ             		(0x01)
#define N2DM_ACC_ODR_10_HZ               	(0x02)
#define N2DM_ACC_ODR_25_HZ               	(0x03)
#define N2DM_ACC_ODR_50_HZ              	(0x04)
#define N2DM_ACC_ODR_100_HZ              	(0x05)
#define N2DM_ACC_ODR_200_HZ              	(0x06)
#define N2DM_ACC_ODR_400_HZ              	(0x07)
#define N2DM_ACC_ODR_1_62_KHZ            	(0x08)
#define N2DM_ACC_ODR_5_376_KHZ				(0x09)
#define N2DM_ACC_ODR_1_344_HZ				(0x09)
#define N2DM_ACC_ODR_MSK                 	(0XF0)
#define N2DM_ACC_ODR_POS                 	(4)

#define N2DM_ACC_DEFAULT_ODR_100HZ          (100)

#define N2DM_BDU_ENABLE						(0x80)

#define N2DM_ACC_STATUS_ZYXDA				(0x08)

#define N2DM_DEFAULT_ODR_100HZ				(100)

#define N2DM_ACC_SELF_TEST_MIN_X           	(17 * 4) // 17 counts, per count 4mg
#define N2DM_ACC_SELF_TEST_MIN_Y           	(17 * 4) // 17 counts, per count 4mg  
#define N2DM_ACC_SELF_TEST_MIN_Z          	(17 * 4)  // 17 counts, per count 4mg
#define N2DM_ACC_SELF_TEST_MAX_X          	(360 * 4) // 360 counts, per count 4mg
#define N2DM_ACC_SELF_TEST_MAX_Y           	(360 * 4) // 360 counts, per count 4mg
#define N2DM_ACC_SELF_TEST_MAX_Z           	(360 * 4) // 360 counts, per count 4mg

#define N2DM_ACC_SELF_TEST_DRY_WAIT_CNT  	5
#define N2DM_ACC_SELF_TEST_AVG_SAMPLE_CNT  	5

#define N2DM_DEFAULT_ODR_100HZ				(100)

#define N2DM_GET_BITSLICE(regvar, bitname)\
((regvar & bitname##_MSK) >> bitname##_POS)
	
#define N2DM_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##_MSK) | ((val<<bitname##_POS)&bitname##_MSK))


static int32_t n2dm_acc_factor[ACC_RANGE_MAX] = { N2DM_ACC_SENSITIVITY_2G, N2DM_ACC_SENSITIVITY_4G, 
                                     N2DM_ACC_SENSITIVITY_8G, N2DM_ACC_SENSITIVITY_16G };
static int32_t cur_acc_factor = 0;


i2c_dev_t n2dm_ctx = {
    .port = 3,
    .config.address_width = 8,
    .config.freq = 400000,
    .config.dev_addr = N2DM_I2C_ADDR,
};

static int drv_acc_st_n2dm_validate_id(i2c_dev_t* drv, uint8_t id_value)
{
    uint8_t value = 0x00;
    int ret = 0;

    if(drv == NULL){
        return -1;
    }
    
    ret = sensor_i2c_read(drv, N2DM_ACC_WHO_AM_I, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    
    if (id_value != value){
        return -1;
    }
    
    return 0;
}

static int drv_acc_st_n2dm_set_power_mode(i2c_dev_t* drv, dev_power_mode_e mode)
{
    uint8_t value,value1 = 0x00;
    int ret = 0;
    uint8_t buf[4];
    
    ret = sensor_i2c_read(drv, N2DM_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    switch(mode){
        case DEV_POWER_ON:{
            value = N2DM_SET_BITSLICE(value,N2DM_ACC_ODR,N2DM_ACC_ODR_10_HZ);
            ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
            if(unlikely(ret)){
                return ret;
            }
        }break;
        
        case DEV_POWER_OFF:{
            value = N2DM_SET_BITSLICE(value,N2DM_ACC_ODR,N2DM_ACC_ODR_POWER_DOWN);
            ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
            if(unlikely(ret)){
                return ret;
            }
        }break;
            
        case DEV_SLEEP:{
            value = N2DM_SET_BITSLICE(value,N2DM_ACC_ODR,N2DM_ACC_ODR_10_HZ);
            ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
            if(unlikely(ret)){
                return ret;
            }

        }break;

       default:break;
    }
    return 0;
}

static uint8_t drv_acc_st_n2dm_hz2odr(uint32_t hz)
{
    if(hz > 1620)
        return N2DM_ACC_ODR_5_376_KHZ;
    else if(hz > 1344)
        return N2DM_ACC_ODR_1_62_KHZ;
    else if(hz > 400)
        return N2DM_ACC_ODR_1_344_HZ;
    else if(hz > 200)
        return N2DM_ACC_ODR_400_HZ;
    else if(hz > 100)
        return N2DM_ACC_ODR_200_HZ;
    else if(hz > 50)
        return N2DM_ACC_ODR_100_HZ;
    else if(hz > 25)
        return N2DM_ACC_ODR_50_HZ;
    else if(hz > 10)
        return N2DM_ACC_ODR_25_HZ;
    else if(hz >= 1)
        return N2DM_ACC_ODR_10_HZ;
    else
        return N2DM_ACC_ODR_1_HZ;

}

static int drv_acc_st_n2dm_set_odr(i2c_dev_t* drv, uint32_t hz)
{
    int ret = 0;
    uint8_t value = 0x00;
    uint8_t odr = drv_acc_st_n2dm_hz2odr(hz);

    ret = sensor_i2c_read(drv, N2DM_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    value = N2DM_SET_BITSLICE(value,N2DM_ACC_ODR,odr);
	
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    return 0;    
}

static int drv_acc_st_n2dm_set_range(i2c_dev_t* drv, uint32_t range)
{

    int ret = 0;
    uint8_t value = 0x00;
    uint8_t tmp = 0;

    ret = sensor_i2c_read(drv, N2DM_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    switch (range){
        case ACC_RANGE_2G:{
            tmp = N2DM_ACC_RANGE_2G;
        }break;
        
        case ACC_RANGE_4G:{
            tmp = N2DM_ACC_RANGE_4G;
        }break;
        
        case ACC_RANGE_8G:{
            tmp = N2DM_ACC_RANGE_8G;
        }break;
            
        case ACC_RANGE_16G:{
            tmp = N2DM_ACC_RANGE_16G;
        }break;
        
        default:break;
    }
    
    value  = N2DM_SET_BITSLICE(value,N2DM_ACC_RANGE,tmp);
    
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    if((range >= ACC_RANGE_2G)&&(range <= ACC_RANGE_16G)){
        cur_acc_factor = n2dm_acc_factor[range];
    }
    
    return 0;
}


static int drv_acc_st_n2dm_set_bdu(i2c_dev_t* drv)
{
    uint8_t value = 0x00;
    int ret = 0;
    
    ret = sensor_i2c_read(drv, N2DM_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    value |= N2DM_BDU_ENABLE;
    
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    return 0;
}

static int drv_acc_st_n2dm_st_discard(i2c_dev_t* drv)
{
    uint8_t i;
    uint8_t value = 0x00;
    int ret = 0;
    uint8_t buffer[6];

    for (i = 0; i < N2DM_ACC_SELF_TEST_DRY_WAIT_CNT; i ++) {
        ret = sensor_i2c_read(drv, N2DM_ACC_STATUS_REG, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
        if(unlikely(ret)){
            LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
            return ret;
        }
        if (value & 0x08)
            break;

        aos_msleep(20);
    }

    if (i >= N2DM_ACC_SELF_TEST_DRY_WAIT_CNT) {
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        return -1;
    }

    ret = sensor_i2c_read(drv, (N2DM_ACC_OUT_X_L | 0x80), buffer, 6, I2C_OP_RETRIES);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        return ret;
    }

    return ret;
}

static int drv_acc_st_n2dm_st_data(i2c_dev_t* drv,int32_t* data)
{
    uint8_t i, j;
    int16_t x_raw, y_raw, z_raw;
    int32_t x_mg, y_mg, z_mg;
    int32_t x_sum, y_sum, z_sum;
    uint8_t value = 0x00;
    int ret = 0;
    uint8_t buffer[6];

    x_sum = 0; y_sum = 0; z_sum = 0;
    
    for (i = 0; i < N2DM_ACC_SELF_TEST_AVG_SAMPLE_CNT; i ++) {
        for (j = 0; j < N2DM_ACC_SELF_TEST_DRY_WAIT_CNT; j ++) {
            ret = sensor_i2c_read(drv, N2DM_ACC_STATUS_REG, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
            if(unlikely(ret)){
                LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
                return ret;
            }
            if (value & 0x08)
                break;
        
            aos_msleep(20);
        }

        if (j >= N2DM_ACC_SELF_TEST_DRY_WAIT_CNT) {
            LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
            return -1;
        }

        ret = sensor_i2c_read(drv,  (N2DM_ACC_OUT_X_L | 0x80), buffer, 6, I2C_OP_RETRIES);
        if(unlikely(ret)){
            LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
            return ret;
        }

        x_raw = (buffer[1] << 8) + buffer[0];
        y_raw = (buffer[3] << 8) + buffer[2];
        z_raw = (buffer[5] << 8) + buffer[4];
        //LOG("%s %s %d: i(%d), raw(%d, %d, %d)\n", SENSOR_STR, __func__, __LINE__, i, x_raw, y_raw, z_raw);

        x_mg = x_raw >> 4;
        y_mg = y_raw >> 4;
        z_mg = z_raw  >> 4;
        //LOG("%s %s %d: i(%d), mg(%d, %d, %d)\n", SENSOR_STR, __func__, __LINE__, i, x_mg, y_mg, z_mg);

        x_sum += x_mg;
        y_sum += y_mg;
        z_sum += z_mg;
    }

    data[0] = x_sum / N2DM_ACC_SELF_TEST_AVG_SAMPLE_CNT;
    data[1] = y_sum / N2DM_ACC_SELF_TEST_AVG_SAMPLE_CNT;
    data[2] = z_sum / N2DM_ACC_SELF_TEST_AVG_SAMPLE_CNT;
    return ret;
}

static int drv_acc_st_n2dm_self_test(i2c_dev_t* drv,int32_t* data)
{
    uint8_t i, j;
    uint8_t value = 0x00;
    int ret = 0;
    uint8_t buffer[6];
    uint8_t ctrl_reg[4];
    int32_t out_nost[3];
    int32_t out_st[3];
    int32_t out_diff[3];

    // Save cfg registers which will be modified during self-test
    ret = sensor_i2c_read(drv, (N2DM_ACC_CTRL_REG1 | 0x80), ctrl_reg, 4, I2C_OP_RETRIES);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        return ret;
    }

    // Initialize Sensor, turn on sensor, enable X/Y/Z axes 
    // Set BDU=1, FS=2G, Normal mode, ODR = 50Hz
    value = 0;
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG2, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }
    value = 0;
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG3, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }
    
    value = 0x80;
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }

    value = 0x47;
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }

    aos_msleep(90);

    // Discard the first sample
    ret = drv_acc_st_n2dm_st_discard(drv);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }

    // Read some samples, and averate them
    ret = drv_acc_st_n2dm_st_data(drv, out_nost);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }

    // Enable seft-test
    value = 0x82;
    ret = sensor_i2c_write(drv, N2DM_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }
    aos_msleep(90);
    
    // Discard the first sample
    ret = drv_acc_st_n2dm_st_discard(drv);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }

    // Read some samples, and average them
    ret = drv_acc_st_n2dm_st_data(drv, out_st);
    if(unlikely(ret)){
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        goto restore;
    }

    // Check if the differences are between min and max
    for (i = 0; i < 3; i ++) {
        out_diff[i] = abs(out_st[i] - out_nost[i]);
        data[i] = out_diff[i];
    }
    
    if ((N2DM_ACC_SELF_TEST_MIN_X > out_diff[0]) || (out_diff[0] > N2DM_ACC_SELF_TEST_MAX_X)) {
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        ret = -1;
        goto restore;
    }
    if ((N2DM_ACC_SELF_TEST_MIN_Y > out_diff[1]) || (out_diff[1] > N2DM_ACC_SELF_TEST_MAX_Y)) {
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        ret = -1;
        goto restore;
    }
    if ((N2DM_ACC_SELF_TEST_MIN_Z > out_diff[2]) || (out_diff[2] > N2DM_ACC_SELF_TEST_MAX_Z)) {
        LOG("%s %s %s %d\n", SENSOR_STR, __func__, ERROR_LINE, __LINE__);
        ret = -1;
        goto restore;
    }

restore:
    ret += sensor_i2c_write(drv, N2DM_ACC_CTRL_REG1 | 0x80, ctrl_reg, 4, I2C_OP_RETRIES);
    return ret;
}

static void drv_acc_st_n2dm_irq_handle(void)
{
    /* no handle so far */
}

static int drv_acc_st_n2dm_open(void)
{
    int ret = 0;
    ret  = drv_acc_st_n2dm_set_power_mode(&n2dm_ctx, DEV_POWER_ON);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_acc_st_n2dm_set_range(&n2dm_ctx, ACC_RANGE_8G);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_acc_st_n2dm_set_odr(&n2dm_ctx, N2DM_ACC_DEFAULT_ODR_100HZ);
    if(unlikely(ret)){
        return -1;
    }
     
    return 0;

}

static int drv_acc_st_n2dm_close(void)
{
    int ret = 0;
    ret  = drv_acc_st_n2dm_set_power_mode(&n2dm_ctx, DEV_POWER_OFF);
    if(unlikely(ret)){
        return -1;
    }
    return 0;
}

static int drv_acc_st_n2dm_read(void *buf, size_t len)
{

    int ret = 0;
    size_t size;
    uint8_t reg[6];
    accel_data_t *accel = (accel_data_t *)buf;
    if(buf == NULL){
        return -1;
    }

    size = sizeof(accel_data_t);
    if(len < size){
        return -1;
    }
	
	ret = sensor_i2c_read(&n2dm_ctx, (N2DM_ACC_OUT_X_L | 0x80), reg, 6, I2C_OP_RETRIES);
	
	if(unlikely(ret)){
        return -1;
    }
    accel->data[DATA_AXIS_X] = (int16_t)((((int16_t)((int8_t)reg[1]))<< N2DM_SHIFT_EIGHT_BITS)|(reg[0]));
	accel->data[DATA_AXIS_X] = accel->data[DATA_AXIS_X] >> N2DM_SHIFT_FOUR_BITS;
    accel->data[DATA_AXIS_Y] = (int16_t)((((int16_t)((int8_t)reg[3]))<< N2DM_SHIFT_EIGHT_BITS)|(reg[2]));
	accel->data[DATA_AXIS_Y] = accel->data[DATA_AXIS_Y] >> N2DM_SHIFT_FOUR_BITS;
    accel->data[DATA_AXIS_Z] = (int16_t)((((int16_t)((int8_t)reg[5]))<< N2DM_SHIFT_EIGHT_BITS)|(reg[4]));
	accel->data[DATA_AXIS_Z] = accel->data[DATA_AXIS_Z] >> N2DM_SHIFT_FOUR_BITS;

    if(cur_acc_factor != 0){
        accel->data[DATA_AXIS_X] = accel->data[DATA_AXIS_X] * cur_acc_factor / 1000;
        accel->data[DATA_AXIS_Y] = accel->data[DATA_AXIS_Y] * cur_acc_factor / 1000;
        accel->data[DATA_AXIS_Z] = accel->data[DATA_AXIS_Z] * cur_acc_factor / 1000;
    }
    
    accel->timestamp = aos_now_ms();

    return (int)size;
}


static int drv_acc_st_n2dm_ioctl(int cmd, unsigned long arg)
{
    int ret = 0;
    dev_sensor_info_t *info = (dev_sensor_info_t *)arg;
    
    switch(cmd){
        case SENSOR_IOCTL_ODR_SET:{
            ret = drv_acc_st_n2dm_set_odr(&n2dm_ctx, arg);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_RANGE_SET:{
            ret = drv_acc_st_n2dm_set_range(&n2dm_ctx, arg);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_SET_POWER:{
            ret = drv_acc_st_n2dm_set_power_mode(&n2dm_ctx, arg);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_GET_INFO:{ 
            /* fill the dev info here */
            info->model = "N2DM";
            info->range_max = 16;
            info->range_min = 2;
            info->unit = mg;
			
        }break;
	   	case SENSOR_IOCTL_SELF_TEST:{
		   ret = drv_acc_st_n2dm_self_test(&n2dm_ctx, info->data);
		   //printf("%d	%d	 %d\n",info->data[0],info->data[1],info->data[2]);
           LOG("%s %s: %d, %d, %d\n", SENSOR_STR, __func__, info->data[0],info->data[1],info->data[2]);
		   return ret;
	   }
       default:break;
    }

    return 0;
}

int drv_acc_st_n2dm_init(void){
    int ret = 0;
    sensor_obj_t sensor;

    /* fill the sensor obj parameters here */
    sensor.io_port    = I2C_PORT;
    sensor.tag        = TAG_DEV_ACC;
    sensor.path       = dev_acc_path;
    sensor.open       = drv_acc_st_n2dm_open;
    sensor.close      = drv_acc_st_n2dm_close;
    sensor.read       = drv_acc_st_n2dm_read;
    sensor.write      = NULL;
    sensor.ioctl      = drv_acc_st_n2dm_ioctl;
    sensor.irq_handle = drv_acc_st_n2dm_irq_handle;

    ret = sensor_create_obj(&sensor);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_acc_st_n2dm_validate_id(&n2dm_ctx, N2DM_ACC_CHIP_ID_VALUE);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_acc_st_n2dm_set_range(&n2dm_ctx, ACC_RANGE_8G);
    if(unlikely(ret)){
        return -1;
    }

	ret = drv_acc_st_n2dm_set_bdu(&n2dm_ctx);
    if(unlikely(ret)){
        return -1;
    }
	
    //set odr is 100hz, and will update
    ret = drv_acc_st_n2dm_set_odr(&n2dm_ctx, N2DM_DEFAULT_ODR_100HZ);
    if(unlikely(ret)){
        return -1;
    }

    /* update the phy sensor info to sensor hal */
    LOG("%s %s successfully \n", SENSOR_STR, __func__);
    return 0;
}

