/*
 * bootloader.h
 *
 *  Created on: Jul 18, 2023
 *      Author: El-Wattaneya
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

/***************Section include************************/
#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include "usart.h"
#include "crc.h"

/***************Macro declaration************************/
#define BL_DEBUG_UART        &huart2
#define BL_SEND_CMD_UART     &huart2
#define DEBUG_INFO_ENABLE    1
#define DEBUG_INFO_DISABLE   0
#define BL_DEBUG_ENABLE      DEBUG_INFO_DISABLE


#define BL_UART_DEBUG_MESSAGE   0X1
#define BL_USB_DEBUG_MESSAGE    0X2


#define BL_DEBUG_METHOD       BL_UART_DEBUG_MESSAGE

#define BL_HOST_RX_BUFFER_LENGTH     100



#define CBL_GET_VER_CMD              0x10
#define CBL_GET_HELP_CMD             0x11
#define CBL_GET_CID_CMD              0x12
/* Get Read Protection Status */
#define CBL_GET_RDP_STATUS_CMD       0x13
#define CBL_GO_TO_ADDR_CMD           0x14
#define CBL_FLASH_ERASE_CMD          0x15
#define CBL_MEM_WRITE_CMD            0x16
/* Enable/Disable Write Protection */
#define CBL_ED_W_PROTECT_CMD         0x17
#define CBL_MEM_READ_CMD             0x18
/* Get Sector Read/Write Protection Status */
#define CBL_READ_SECTOR_STATUS_CMD   0x19
#define CBL_OTP_READ_CMD             0x20
/* Change Read Out Protection Level */
#define CBL_CHANGE_ROP_Level_CMD     0x21



#define CBL_VENDOR_ID                100
#define CBL_SW_MAJOR_VERSION         1
#define CBL_SW_MINOR_VERSION         1
#define CBL_SW_PATCH_VERSION         0


#define CRC_VERIFICATION_FAILED      0x00
#define CRC_VERIFICATION_PASSED      0x01




#define CBL_SEND_NACK                0xAB
#define CBL_SEND_ACK                 0xCD



#define FLASH_SECTOR2_BASE_ADDRESS   0x08008000U

#define ADDRESS_IS_INVALID           0x00
#define ADDRESS_IS_VALID             0x01


#define STM32F103_SRAM_SIZE         (20 * 1024)
#define STM32F103_FLASH_SIZE         (64 * 1024)
#define STM32F103_SRAM_END          (SRAM_BASE + STM32F103_SRAM_SIZE)
#define STM32F103_FLASH_END          (FLASH_BASE + STM32F103_FLASH_SIZE)

#define CBL_FLASH_MAX_PAGE_NUMBER    128
#define CBL_FLASH_MASS_ERASE         0xFF


#define INVALID_PAGE_NUMBER        0x00
#define VALID_PAGE_NUMBER          0x01
#define UNSUCCESSFUL_ERASE           0x02
#define SUCCESSFUL_ERASE             0x03

#define HAL_SUCCESSFUL_ERASE         0xFFFFFFFFU



#define FLASH_PAYLOAD_WRITE_FAILED   0x00
#define FLASH_PAYLOAD_WRITE_PASSED   0x01



#define FLASH_BANK_SIZE (0X5800)		//22kB
#define FLASH_PAGE_SIZE_USER (0x400)
/***************Macro function declaration************************/






/***************Data Type declaration************************/

typedef enum{
	BL_NACK=0,
	BL_OK
}BL_Status;


typedef void (*pMainApp)(void);
typedef void (*Jump_ptr)(void);
/***************Function Declaration************************/
BL_Status BL_FeatchHostCommand();
void Bootloader_Jump_To_Application();
uint8_t Perform_Flash_Erase(uint32_t PageAddress, uint8_t page_Number);
void  BL_PrintMessage(char *mess,...);
#endif /* INC_BOOTLOADER_H_ */
