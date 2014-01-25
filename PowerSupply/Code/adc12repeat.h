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
//void uint16_ToString(uint8_t *output_string, uint8_t pos, uint16_t input_number);

/* End of multiple inclusion prevention macro */
#endif

#ifdef __cplusplus
}
#endif
