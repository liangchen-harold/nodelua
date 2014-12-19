/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: spi_master.c
 *
 * Description: spi master API
 *
 * Modification history:
 *     2014/8/12, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "driver/spi_master.h"

void ICACHE_FLASH_ATTR
spi_master_init(uint8 spi_no)
{
    uint32 regvalue;

    if (spi_no > 1) {
    	return;
    }

    WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit9

    if (spi_no == SPI) {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U, 1);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U, 1);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U, 1);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U, 1);//configure io to spi mode
    } else if (spi_no == HSPI) {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);//configure io to spi mode
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);//configure io to spi mode
    }

    regvalue = READ_PERI_REG(SPI_FLASH_USER(spi_no));
	regvalue = regvalue | SPI_CS_SETUP | SPI_CS_HOLD | SPI_USR_COMMAND/*|BIT27*/; //set bit 4 bit 5 bit 27 bit31
	regvalue = regvalue & (~BIT2); //clear bit 2
	WRITE_PERI_REG(SPI_FLASH_USER(spi_no), regvalue);
	// disable div0, ie, 80MHz clock
	WRITE_PERI_REG(SPI_FLASH_CLOCK(spi_no), 0x43043); //clear bit 31,set SPI clock div
}

void ICACHE_FLASH_ATTR
spi_master_9bit_write(uint8 spi_no, uint8 high_bit, uint8 low_8bit)
{
    uint32 regvalue;
    uint8 bytetemp;

    if (spi_no > 1) {
        return;
    }

    if (high_bit) {
        bytetemp = (low_8bit >> 1) | 0x80;
    } else {
        bytetemp = (low_8bit >> 1) & 0x7f;
    }

    regvalue = 0x80000000 | ((uint32)bytetemp);		//configure transmission variable,9bit transmission length and first 8 command bit

    if (low_8bit & 0x01) {
        regvalue |= BIT15;    //write the 9th bit
    }

    while (READ_PERI_REG(SPI_FLASH_CMD(spi_no))&SPI_FLASH_USR);		//waiting for spi module available

    WRITE_PERI_REG(SPI_FLASH_USER2(spi_no), regvalue);				//write  command and command length into spi reg
    SET_PERI_REG_MASK(SPI_FLASH_CMD(spi_no), SPI_FLASH_USR);		//transmission start
}
