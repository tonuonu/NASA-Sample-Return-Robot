

/* OLED ouptut pin settings */
#define  OLED_DS      PORT5.PODR.BIT.B3
#define  OLED_RD      PORT5.PODR.BIT.B2
#define  OLED_RESET   PORT5.PODR.BIT.B1
#define  OLED_WR      PORT5.PODR.BIT.B0
#define  OLED_CS      PORTC.PODR.BIT.B7

/* OLED data direction */
#define OLED_DS_PORT_DIR     PORT5.PDR.BIT.B3
#define OLED_RD_PORT_DIR     PORT5.PDR.BIT.B2
#define OLED_RESET_PORT_DIR  PORT5.PDR.BIT.B1
#define OLED_WR_PORT_DIR     PORT5.PDR.BIT.B0
#define OLED_CS_PORT_DIR     PORTC.PDR.BIT.B7

#define OLED_DATA_PORT_DIR   PORTD.PDR.BYTE

void Init_OLED(void);

