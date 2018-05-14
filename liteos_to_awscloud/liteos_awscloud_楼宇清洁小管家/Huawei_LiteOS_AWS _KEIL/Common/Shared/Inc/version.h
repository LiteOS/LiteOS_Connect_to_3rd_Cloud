/**
  ******************************************************************************
  * @file    version.h
  * @author  MCD Application Team
  * @brief   STM32 FW version definition
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __version_H
#define __version_H

#define FWVERSION_NAME_SIZE   20
#define FWVERSION_DATE_SIZE   16
#define FWVERSION_TIME_SIZE   9
   

/** Firmware version information.
 * @note  For user information.
 */
typedef struct {
  char name[FWVERSION_NAME_SIZE];
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  char *packaged_date;
} firmware_version_t;


#define FW_VERSION_MAJOR 1
#define FW_VERSION_MINOR 2
#define FW_VERSION_PATCH 0
#define FW_VERSION_DATE "12-February-2018  5:47:19 PM"

#ifdef AWS
#define FW_VERSION_NAME   "X-CUBE-AWS"
#define AWS_VERSION	TBD
#endif

#ifdef AZURE
#define FW_VERSION_NAME   "X-CUBE-AZURE"
#define AZURE_VERSION TBD

#endif

#ifdef BLUEMIX
#define FW_VERSION_NAME   "X-CUBE-WATSON-X"
#define BLUEMIX_VERSION TBD

#endif

#if defined(GENERICMQTT) || defined(EXOSITEHTTP) || defined(HTTPCLIENT)
#define FW_VERSION_NAME   "X-CUBE-CLD-GEN"
#define LITMUS_VERSION TBD
#endif


#endif /* __version_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
