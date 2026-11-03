/*! ----------------------------------------------------------------------------
 * @file    port.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2016-2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include <port.h>
#include "main.h"
//#include <stm32f1xx_hal_conf.h>
//#include <usbd_cdc_if.h>

/****************************************************************************
 *
 *                              APP global variables
 *
 *******************************************************************************/
// extern SPI_HandleTypeDef hspi1,hspi4;
// extern SPI_HandleTypeDef *hcurrent_active_spi;
// extern uint16_t          pin_io_active_spi;
// extern GPIO_PinState     SPI_CS_state;
// extern host_using_spi_e host_spi;

/****************************************************************************
 *
 *                  Port private variables and function prototypes
 *
 *******************************************************************************/
static volatile uint32_t signalResetDone;

/* DW IC IRQ handler definition. */
//static port_dwic_isr_t port_dwic_isr = NULL;

/****************************************************************************
 *
 *                              Time section
 *
 *******************************************************************************/

/* @fn    portGetTickCnt
 * @brief wrapper for to read a SysTickTimer, which is incremented with
 *        CLOCKS_PER_SEC frequency.
 *        The resolution of time32_incr is usually 1/1000 sec.
 * */
// __INLINE uint32_t portGetTickCnt(void)
// {
//     return HAL_GetTick();
// }

/* @fn    usleep
 * @brief precise usleep() delay
 * */
#pragma GCC optimize("O0")
int usleep(uint16_t usec)
{
    for(uint32_t start = LL_TIM_GetCounter(TIM1); ((LL_TIM_GetCounter(TIM1)-start) & UINT16_MAX) < usec; )
    {
       
    }
    
    
    return 0;
}

/* @fn    Sleep
 * @brief Sleep delay in ms using SysTick timer
 * */
// __INLINE void Sleep(uint32_t x)
// {
//     HAL_Delay(x);
// }
__INLINE void Sleep(uint32_t x){
    
    LL_mDelay(x);

}

/****************************************************************************
 *
 *                              END OF Time section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                              Configuration section
 *
 *******************************************************************************/





/**
 * @brief  Checks whether the specified IRQn line is enabled or not.
 * @param  IRQn: specifies the IRQn line to check.
 * @return "0" when IRQn is "not enabled" and !0 otherwise
 */
// ITStatus EXTI_GetITEnStatus(IRQn_Type IRQn)
// {
//     return ((NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] & (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL))) == (uint32_t)RESET) ? (RESET) : (SET);
// }
/****************************************************************************
 *
 *                          End of configuration section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                          DW IC port section
 *
 *******************************************************************************/

/* @fn      reset_DW IC
 * @brief   DW_RESET pin on DW IC has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW IC by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DWIC(void)
{
    LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_12); //RSTn low
    usleep(1);
    LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_12); //RSTn high
    LL_mDelay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)   
}




/*! ------------------------------------------------------------------------------------------------------------------
 * @fn wakeup_device_with_io()
 *
 * @brief This function wakes up the device by toggling io with a delay.
 *
 * input None
 *
 * output -None
 *
 */
void wakeup_device_with_io(void)  //hier wakeup mit CS? muss dann die mosi low sein?
{
    LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_14);
    usleep(500);
    LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_14);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn make_very_short_wakeup_io()
 *
 * @brief This will toggle the wakeup pin for a very short time. The device should not wakeup
 *
 * input None
 *
 * output -None
 *
 */
void make_very_short_wakeup_io(void)
{
    while(1){
        
    }
    // uint8_t cnt;

    // SET_WAKEUP_PIN_IO_HIGH;
    // for (cnt = 0; cnt < 10; cnt++)
    //     __NOP();
    // SET_WAKEUP_PIN_IO_LOW;
}

/* @fn      port_is_boot1_low
 * @brief   check the BOOT1 pin status.
 * @return  1 if ON and 0 for OFF
 * */
// int port_is_boot1_low(void)
// {
//     return ((GPIO_ReadInputDataBit(TA_BOOT1_GPIO, TA_BOOT1)) ? (0) : (1));
// }

// /* @fn      port_is_boot1_on
//  * @brief   check the BOOT1 pin is on.
//  * @return  1 if ON and 0 for OFF
//  * */
// int port_is_boot1_on(uint16_t x)
// {
    
// }






/* @fn      change_SPI
 * @brief   Select Host to work with (SPI1 or SPI4)
 *
 * @param   spi - HOST enum to work with
 * */
// void change_SPI(host_using_spi_e    spi)
// {
//     if (spi==SPI_1)
//     {
//         hcurrent_active_spi=&hspi1;
//         pin_io_active_spi=DW_NSS_Pin;
//         SPI_CS_state=GPIO_PIN_RESET;
//     }
//     else
//     {//SPI 2
//         hcurrent_active_spi=&hspi4;
//         pin_io_active_spi=DW_NSS1_WAKEUP_Pin;
//         SPI_CS_state=GPIO_PIN_SET;
//     }

//     host_spi = spi;
// }


/* @fn      port_set_dw_ic_spi_slowrate
 * @brief   sets SPI1 to 4.5MBits/s
 *          note: hspi1 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_slowrate(void)
{
    LL_SPI_Disable(SPI1);
    LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV32);
    LL_SPI_Enable(SPI1);
    LL_SPI_StartMasterTransfer(SPI1);
}

/* @fn      port_set_dw_ic_spi_fastrate
 * @brief   sets SPI1 to 36MBits/s
 *          note: hspi1 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_fastrate(void)
{
    LL_SPI_Disable(SPI1);
    LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV4);
    LL_SPI_Enable(SPI1);
    LL_SPI_StartMasterTransfer(SPI1);
}








