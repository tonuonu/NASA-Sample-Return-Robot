/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.   */
/*******************************************************************************
* File Name    : adc12repeat.h
* Version     : 1.00
* Device     : R5F5630E
* Tool Chain   : IAR Embedded Workbench
* H/W Platform  : RSKRX630
* Description   : Contains prototypes for the ADC controlling functions 
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/* Multiple inclusion prevention macro */
#ifndef ADC12_REPEAT_H
#define ADC12_REPEAT_H

/* Define a macro to indicate selection of a periodic timer mode */
#define PERIODIC_MODE 0x1

/*******************************************************************************
* Global Function Prototypes
*******************************************************************************/
/* ADC initialisation function prototype declaration */
void Init_ADC12Repeat(void);
void uint16_ToString(uint8_t *output_string, uint8_t pos, uint16_t input_number);

/* End of multiple inclusion prevention macro */
#endif

#ifdef __cplusplus
}
#endif
