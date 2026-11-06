/*! ----------------------------------------------------------------------------
 * @file    deca_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include <deca_device_api.h>
#include <deca_spi.h>
#include <port.h>
//#include <stm32f4xx_hal_def.h>
#include "main.h"

//extern SPI_HandleTypeDef    *hcurrent_active_spi;/*clocked from 72MHz*/
//extern uint16_t             pin_io_active_spi;
//extern GPIO_PinState        SPI_CS_state;

/****************************************************************************
 *
 *
 *                              DWxxx SPI section
 *
 *******************************************************************************/
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(/*SPI_TypeDef* SPIx*/)
{
    return 0;
} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
    return 0;
} // end closespi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospiwithcrc()
 *
 * Low level abstract function to write to the SPI when SPI CRC mode is used
 * Takes two separate byte buffers for write header and write data, and a CRC8 byte which is written last
 * returns 0 for success, or -1 for error
 */
int32_t writetospiwithcrc(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer, uint8_t crc8)
{


    return 0;
} // end writetospiwithcrc()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
int32_t writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer)
{
    decaIrqStatus_t stat;

    stat = decamutexon();

    LL_SPI_SetTransferSize(SPI1, headerLength + bodyLength);

    LL_SPI_ClearFlag_EOT(SPI1);

    LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_14); //CS low
    
    LL_SPI_StartMasterTransfer(SPI1);

    for (uint16_t i = 0; i < headerLength; i++) //transmit header
    {           
        while (!LL_SPI_IsActiveFlag_TXP(SPI1)){
            //wait (muss ich hier warten oder könnte ich auch mit interrupt arbeiten?)
        }
        LL_SPI_TransmitData8(SPI1, headerBuffer[i]); 
        
        while (!LL_SPI_IsActiveFlag_RXP(SPI1)){
            //wait
        }
        LL_SPI_ReceiveData8(SPI1); //read dummy data
    }
    for (uint16_t i = 0; i < bodyLength; i++) //transmit body
    {           
        while (!LL_SPI_IsActiveFlag_TXP(SPI1)){
            //wait
        }
        LL_SPI_TransmitData8(SPI1, bodyBuffer[i]);

        while (!LL_SPI_IsActiveFlag_RXP(SPI1)){
            //wait
        }
        LL_SPI_ReceiveData8(SPI1); //read dummy data    
    }
    while (!LL_SPI_IsActiveFlag_EOT(SPI1)){ 
        //wait for end of transfer
    }
    LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_14); //CS high

    decamutexoff(stat);

    return 0;
} // end writetospi()

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_cs_low_delay()
 *
 * @brief This function sets the CS to '0' for ms delay and than raises it up
 *
 * input parameters:
 * @param ms_delay - The delay for CS to be in '0' state
 *
 * no return value
 */
uint16_t spi_cs_low_delay(uint16_t delay_ms)
{
    while (!LL_SPI_IsActiveFlag_EOT(SPI1)){ 
        //wait for end of transfer
    }
    LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_14); //CS low
    sleep(delay_ms); //wait for delay_ms
    LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_14); //CS high
    return 0;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
//#pragma GCC optimize ("O3")
int32_t readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer)
{
    decaIrqStatus_t stat;

    stat = decamutexon();
    LL_SPI_SetTransferSize(SPI1, headerLength + readlength);
    LL_SPI_ClearFlag_EOT(SPI1);
    LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_14); //CS low
    LL_SPI_StartMasterTransfer(SPI1);
    
    for (uint16_t i = 0; i < headerLength; i++)
    {           
        while (!LL_SPI_IsActiveFlag_TXP(SPI1)){
            //wait
        }
        LL_SPI_TransmitData8(SPI1, headerBuffer[i]); //send header  
        while (!LL_SPI_IsActiveFlag_RXP(SPI1)){
            //wait
        }
        LL_SPI_ReceiveData8(SPI1); //read dummy data
    }
    for (uint16_t i = 0; i < readlength; i++)
    {           
        while (!LL_SPI_IsActiveFlag_TXP(SPI1)){
            //wait
        }
        LL_SPI_TransmitData8(SPI1, 0x00); //send dummy data to read
        while (!LL_SPI_IsActiveFlag_RXP(SPI1)){
            //wait
        }
        readBuffer[i] = LL_SPI_ReceiveData8(SPI1); //read data
    }
    while (!LL_SPI_IsActiveFlag_EOT(SPI1)){
        //wait for end of transfer
    }
    LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_14); //CS high
    decamutexoff(stat);

    return 0;
} // end readfromspi()

/****************************************************************************
 *
 *                              END OF DW3xxx SPI section
 *
 *******************************************************************************/
