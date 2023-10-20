/*
 * bootloader.c
 *
 *  Created on: Jul 18, 2023
 *      Author: El-Wattaneya
 */
#include "bootloader.h"





/* ----------------- Static Functions Decleration -----------------*/

static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);

 void Bootloader_Jump_To_Application();
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_ACK(uint8_t Replay_Len);
static void Bootloader_Send_NACK(void);
void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);
static uint8_t Host_Address_Verification(uint32_t Jump_Address);
uint8_t Perform_Flash_Erase(uint32_t PageAddress, uint8_t page_Number);
static uint8_t FlashMemory_Payload_Write(uint16_t *Host_Buff, uint32_t Payload_StartAddresss,uint32_t payloadLen);
uint8_t BL_HostBuff[BL_HOST_RX_BUFFER_LENGTH];

static uint8_t Bootloader_Supported_CMDs[2] = {

    CBL_FLASH_ERASE_CMD,
    CBL_MEM_WRITE_CMD,

};

/**
 * @breif : this function Fetch the selected command from the Host
 */
BL_Status BL_FeatchHostCommand()
{
	BL_Status status =BL_NACK;
	HAL_StatusTypeDef HAL_status= HAL_ERROR;
	uint8_t DataLenght=0;
	memset(BL_HostBuff,0,BL_HOST_RX_BUFFER_LENGTH);
	/*Receive the length of the packet from the host*/
	HAL_status = HAL_UART_Receive(BL_SEND_CMD_UART, BL_HostBuff, 1, HAL_MAX_DELAY);
	if(HAL_OK != HAL_status)
	{
		status =BL_NACK;
	}
	else
	{
		DataLenght = BL_HostBuff[0]-4;
		/*Receive the whole packet from the Host*/
		HAL_status = HAL_UART_Receive(BL_SEND_CMD_UART, &BL_HostBuff[1], DataLenght, HAL_MAX_DELAY);
		DataLenght++;

		if(HAL_OK != HAL_status)
		{
			status =BL_NACK;
		}
		else
		{
			switch(BL_HostBuff[1])
			{

			case CBL_FLASH_ERASE_CMD:
				Bootloader_Erase_Flash(BL_HostBuff);
				break;
			case CBL_MEM_WRITE_CMD:
				Bootloader_Memory_Write(BL_HostBuff);

				break;

			}
		}


	}
	return status;
}

void BL_PrintMessage(char *mess,...)
{
	char message[100]={0};
	va_list args;
	va_start(args,mess);
	vsprintf(message,mess,args);

#if BL_DEBUG_METHOD ==	BL_UART_DEBUG_MESSAGE
	HAL_UART_Transmit(BL_DEBUG_UART,(uint8_t*) message,sizeof(message),HAL_MAX_DELAY );
#elif BL_DEBUG_METHOD == BL_USB_DEBUG_MESSAGE
	CDC_Transmit_FS((uint8_t*)&message,va_arg(arg, int));
#endif
	va_end(args);

}

static void Bootloader_Erase_Flash(uint8_t *Host_Buffer)
{
	uint16_t Host_CMD_Packet_length = 0;
	uint32_t Host_CRC32 = 0;
	uint8_t eraseStatus=SUCCESSFUL_ERASE;
	Host_CMD_Packet_length = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t*) ((Host_Buffer + Host_CMD_Packet_length) - 4));
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMessage("Mass erase or sector erase user flash ");
#endif
	if (CRC_VERIFICATION_PASSED
			== Bootloader_CRC_Verify((uint8_t*) &Host_Buffer[0],
					Host_CMD_Packet_length - 4, Host_CRC32)) {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMessage("CRC calculation passed");
#endif
		Bootloader_Send_ACK(1);
		eraseStatus = Perform_Flash_Erase(Host_Buffer[2],Host_Buffer[3]);
		if(eraseStatus == SUCCESSFUL_ERASE)
		{
			Bootloader_Send_Data_To_Host((uint8_t*)&eraseStatus, 1);
		}
		else
		{
			Bootloader_Send_Data_To_Host((uint8_t*)&eraseStatus, 1);
		}

	} else {
		Bootloader_Send_NACK();
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMessage("CRC calculation failed");
#endif
	}
}
uint16_t i=0;
static void Bootloader_Memory_Write(uint8_t *Host_Buffer)
{

	uint16_t Host_CMD_Packet_length = 0;
	uint32_t Host_CRC32 = 0;
	static uint32_t HostAddress=0;
	uint32_t PayloadLen=0;
	uint8_t Address_Verification=ADDRESS_IS_INVALID;
	uint8_t Flash_payload_status=FLASH_PAYLOAD_WRITE_FAILED;
	Host_CMD_Packet_length = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t*) ((Host_Buffer + Host_CMD_Packet_length) - 4));
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMessage("Write data in different memories in MCU  ");
#endif
	/*if (CRC_VERIFICATION_PASSED
			== Bootloader_CRC_Verify((uint8_t*) &Host_Buffer[0],
					Host_CMD_Packet_length - 4, Host_CRC32)) {*/
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMessage("CRC calculation passed");
#endif
		//Bootloader_Send_ACK(1);
		HostAddress = *((uint32_t*)&Host_Buffer[2]) + 64*i;
		i++;
		PayloadLen=Host_Buffer[6];
		Address_Verification = Host_Address_Verification(HostAddress);
		if(ADDRESS_IS_VALID == Address_Verification)
		{
			Flash_payload_status = FlashMemory_Payload_Write((uint16_t*)&Host_Buffer[7],HostAddress,PayloadLen);

			if(Flash_payload_status == FLASH_PAYLOAD_WRITE_PASSED)
			{
				Bootloader_Send_Data_To_Host((uint8_t*)&Flash_payload_status, 1);
			}
			else
			{
				Bootloader_Send_Data_To_Host((uint8_t*)&Flash_payload_status, 1);
			}
		}
		else
		{
			Bootloader_Send_Data_To_Host((uint8_t*)&Address_Verification, 1);
		}


	/*} else {
		Bootloader_Send_NACK();
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMessage("CRC calculation failed");
#endif
	}*/

}

/**
 * @breif : this function that jump to the application
 */
 void Bootloader_Jump_To_Application()
{
	uint32_t MSP_Value = *((volatile uint32_t*)FLASH_SECTOR2_BASE_ADDRESS);
	uint32_t MainAppAdd = *((volatile uint32_t*)(FLASH_SECTOR2_BASE_ADDRESS+4));

	pMainApp ResetHandler_Address=(pMainApp)MainAppAdd;
	HAL_RCC_DeInit();
	__set_MSP(MSP_Value);
	ResetHandler_Address();
}
/**
 * @breif : this function that calculate the CRC for the Receive data and verify
 * 			it with the CRC value which sent by the Host
 * param  : pData -> pointer to the data received
 * 			Data_Len -> the length of the data
 * 			Host_CRC -> the CRC value calculated by the Host
 * return : CRC_VERIFICATION_FAILED if the data corrupted
 * 			CRC_VERIFICATION_PASS if the data received success
 */
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC)
{
	uint8_t crc_status=CRC_VERIFICATION_FAILED;
	uint32_t MCU_crc_calculated=0;
	uint32_t DataBuffer=0;
	for(uint8_t count=0;count<Data_Len;count++)
	{
		DataBuffer=(uint32_t)pData[count];
		MCU_crc_calculated = HAL_CRC_Accumulate(&hcrc, &DataBuffer, 1);

	}
	__HAL_CRC_DR_RESET(&hcrc);
	if(Host_CRC== MCU_crc_calculated)
	{
		crc_status=CRC_VERIFICATION_PASSED;
	}
	else
	{
		crc_status=CRC_VERIFICATION_FAILED;
	}

	return crc_status;
}
/**
 * @breif : this function send Not acknowledge to Bootloader
 *
 */
static void Bootloader_Send_ACK(uint8_t Replay_Len)
{
	uint8_t ACK_valu[2]={0};
	ACK_valu[0] = CBL_SEND_ACK;
	ACK_valu[1] = Replay_Len;
	HAL_UART_Transmit(BL_DEBUG_UART, (uint8_t *)ACK_valu, 2, HAL_MAX_DELAY);
}
/**
 * @breif : this function send acknowledge to Bootloader
 *
 */
static void Bootloader_Send_NACK(void)
{
	uint8_t  ACK_valu = CBL_SEND_NACK;
	HAL_UART_Transmit(BL_DEBUG_UART,&ACK_valu,sizeof( ACK_valu),HAL_MAX_DELAY );
}
void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len)
{
	HAL_UART_Transmit(BL_DEBUG_UART,Host_Buffer,Data_Len,HAL_MAX_DELAY );
}
/**
 * @breif : this function for Address Verification
 * param  : Address the address needed to verify
 * return : ADDRESS_IS_INVALID if the address is invalid
 * 			ADDRESS_IS_VALID   if the address is valid
 */
static uint8_t Host_Address_Verification(uint32_t Address)
{
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	if(Address >= SRAM_BASE && Address <= STM32F103_SRAM_END)
	{
		Address_Verification = ADDRESS_IS_VALID;
	}
	else if(Address >= FLASH_BASE && Address <= STM32F103_FLASH_END)
	{
		Address_Verification = ADDRESS_IS_VALID;
	}
	else{
		Address_Verification = ADDRESS_IS_INVALID;
	}

	return Address_Verification;
}
/**
 * @breif  : this function erase the flash memory
 * param   : PageAddress -> the start address of the the code needed to erase
 *   		 page_Number -> the number of page needed to erase
 * return  : INVALID_PAGE_NUMBER if the page is invalid
 * 			 VALID_PAGE_NUMBER  if the page is valid
 * 			 UNSUCCESSFUL_ERASE if unsuccessful erase
 * 			 SUCCESSFUL_ERASE	if successful erase
 */
uint8_t Perform_Flash_Erase(uint32_t PageAddress, uint8_t page_Number)
{
	FLASH_EraseInitTypeDef pEraseInit; /*Struct has the the initialize data */
	uint8_t PageStatus = INVALID_PAGE_NUMBER;
	HAL_StatusTypeDef eraseStatus=HAL_ERROR;
	uint32_t PageError=0;
	/*check if the valid page number or not */
	if(page_Number>CBL_FLASH_MAX_PAGE_NUMBER)
	{

		PageStatus = INVALID_PAGE_NUMBER;
	}
	else
	{
		PageStatus = VALID_PAGE_NUMBER;
		if(page_Number <= (CBL_FLASH_MAX_PAGE_NUMBER-1) || CBL_FLASH_MASS_ERASE ==PageAddress)
		{
			if(CBL_FLASH_MASS_ERASE ==PageAddress)
			{
				pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
				pEraseInit.PageAddress= 0x08008000;
				pEraseInit.NbPages =  7;
			}
			else
			{

				pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
				pEraseInit.PageAddress= PageAddress;
				pEraseInit.NbPages = page_Number;
			}
			pEraseInit.Banks=FLASH_BANK_1;
			HAL_FLASH_Unlock();
			eraseStatus = HAL_FLASHEx_Erase(&pEraseInit,&PageError);
			HAL_FLASH_Lock();
			if(HAL_SUCCESSFUL_ERASE == PageError)
			{
				PageStatus = SUCCESSFUL_ERASE;
			}
			else
			{
				PageStatus = UNSUCCESSFUL_ERASE;
			}
		}
		else
		{
			PageStatus = UNSUCCESSFUL_ERASE;
		}
	}

	return PageStatus;
}
/**
 * @breif : this function Write the received code in the flash memory
 * param  : Host_Buff -> pointer to the data needed to written in the flash
 * 			Payload_StartAddresss -> start address of the Data
 * 			payloadLen -> the length of the data needed to write in the flash
 * return :	FLASH_PAYLOAD_WRITE_FAILED if Writing data is fail
 * 			FLASH_PAYLOAD_WRITE_PASS if Writing data seccess
 */
static uint8_t FlashMemory_Payload_Write(uint16_t *Host_Buff, uint32_t Payload_StartAddresss,uint32_t payloadLen)
{
	HAL_StatusTypeDef HALstatus = HAL_ERROR;  /*has the status of the HAL functions*/
	uint8_t Flash_payload_status=FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Payload_counter=0 ;
	uint16_t Payload_count=0 ;
	uint8_t status=FLASH_PAYLOAD_WRITE_PASSED;
	uint32_t Address = 0;
	/*unlock the flash memory to write the code*/
	HAL_FLASH_Unlock();
	/*Writing the Received binary data (Code) from host */
	for(Payload_counter=0,Payload_count=0 ;Payload_counter<payloadLen/2;Payload_counter+=1,Payload_count+=2)
	{
		/*update the flash address each iltration */
		Address = Payload_StartAddresss+Payload_count;
		/*Writing the Date in the flash Halfword by Halfword */
		HALstatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Address,Host_Buff[Payload_counter]);

		/*Check status of writing in the flash*/
		if(HALstatus != HAL_OK)
		{
			Flash_payload_status=FLASH_PAYLOAD_WRITE_FAILED;
			break;

		}
		else
		{
			Flash_payload_status=FLASH_PAYLOAD_WRITE_PASSED;
		}
	}

	if(Flash_payload_status==FLASH_PAYLOAD_WRITE_PASSED && HALstatus == HAL_OK)
	{
		HAL_FLASH_Lock();
	}
	/*lock flash after Writing the data (code)*/
	HAL_FLASH_Lock();
	return Flash_payload_status;
}
