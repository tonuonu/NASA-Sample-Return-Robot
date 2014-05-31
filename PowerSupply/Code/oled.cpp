/*
 *  Copyright (c) 2013, 2014 Tonu Samuel
 *  All rights reserved.
 *
 *  This file is part of robot "Kuukulgur".
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "delay.h"
#include "hwsetup.h"
#include "oled.h"
#include "spi.h"
#include "switch.h"
#include "adc12repeat.h"
#include "stdio.h"
#include "tnroman.h"
#include "iorx630.h"
#include "rskrx630def.h"

void Write_Data(unsigned char dat) {
    OLED_DC=1;
    OLED_DATA_PORT=dat;
    OLED_WR=0;
    __delay_cycles(24UL); // 0.250us
    OLED_WR=1;
    __delay_cycles(24UL); // 0.250us
    return;
}

void Write_Instruction(unsigned char cmd) {
    OLED_DC=0;
    OLED_DATA_PORT=cmd;
    OLED_WR=0;
    __delay_cycles(24UL); // 0.250us
    OLED_WR=1;
    __delay_cycles(24UL); // 0.250us
    return;
}

#define	OLED_Shift	0x1C
#define OLED_Max_Column	0x3F				   // 256/4-1
#define OLED_Max_Row	0x3F				   // 64-1
#define	OLED_Brightness	0x0F

void
OLED_Set_Column_Address(unsigned char a, unsigned char b) {
    OLED_CS = 0;
    Write_Instruction(0x15);				   // Set Column Address
    Write_Data(a);					   //   Default => 0x00
    Write_Data(b);					   //   Default => 0x77
    OLED_CS = 1;
}

void
OLED_Set_Row_Address(unsigned char a, unsigned char b) {
    OLED_CS = 0;
    Write_Instruction(0x75);				   // Set Row Address
    Write_Data(a);					   //   Default => 0x00
    Write_Data(b);					   //   Default => 0x7F
    OLED_CS = 1;
}

void
OLED_Set_Write_RAM() {
    OLED_CS = 0;
    Write_Instruction(0x5C);				   // Enable MCU to Write into RAM
// omit!    OLED_CS = 1;
}

void
OLED_Set_Read_RAM() {
    OLED_CS = 0;
    Write_Instruction(0x5D);				   // Enable MCU to Read from RAM
// omit!    OLED_CS = 1;
}

void
OLED_Set_Remap_Format(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xA0);				   // Set Re-Map / Dual COM Line Mode
    Write_Data(d);					   //   Default => 0x40
    //     Horizontal Address Increment
    //     Column Address 0 Mapped to SEG0
    //     Disable Nibble Remap
    //     Scan from COM0 to COM[N-1]
    //     Disable COM Split Odd Even
    Write_Data(0x11);				   //   Default => 0x01 (Disable Dual COM Mode)
    OLED_CS = 1;
}
void

OLED_Set_Start_Line(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xA1);				   // Set Vertical Scroll by RAM
    Write_Data(d);					   //   Default => 0x00
    OLED_CS = 1;
}

void
OLED_Set_Display_Offset(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xA2);				   // Set Vertical Scroll by Row
    Write_Data(d);					   //   Default => 0x00
    OLED_CS = 1;
}

void
OLED_Set_Display_Mode(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xA4 | d);				   // Set Display Mode
    //   Default => 0xA4
    //     0xA4 (0x00) => Entire Display Off, All Pixels Turn Off
    //     0xA5 (0x01) => Entire Display On, All Pixels Turn On at GS Level 15
    //     0xA6 (0x02) => Normal Display
    //     0xA7 (0x03) => Inverse Display
    OLED_CS = 1;
}

void
OLED_Set_Partial_Display(unsigned char a, unsigned char b, unsigned char c) {
    OLED_CS = 0;
    Write_Instruction(0xA8 | a);				   // Default => 0x8F
    //   Select Internal Booster at Display On
    if (a == 0x00) {
	Write_Data(b);
	Write_Data(c);
    }
    OLED_CS = 1;
}

void
OLED_Set_Function_Selection(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xAB);				   // Function Selection
    Write_Data(d);					   //   Default => 0x01
    //     Enable Internal VDD Regulator
    OLED_CS = 1;
}

void
OLED_Set_Display_On_Off(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xAE | d);				   // Set Display On/Off
    //   Default => 0xAE
    //     0xAE (0x00) => Display Off (Sleep Mode On)
    //     0xAF (0x01) => Display On (Sleep Mode Off)
    OLED_CS = 1;
}

void
OLED_Set_Phase_Length(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xB1);				   // Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment
    Write_Data(d);					   //   Default => 0x74 (7 Display Clocks [Phase 2] / 9 Display Clocks [Phase 1])
    //     D[3:0] => Phase 1 Period in 5~31 Display Clocks
    //     D[7:4] => Phase 2 Period in 3~15 Display Clocks
    OLED_CS = 1;
}

void
OLED_Set_Display_Clock(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xB3);				   // Set Display Clock Divider / Oscillator Frequency
    Write_Data(d);					   //   Default => 0xD0
    //     A[3:0] => Display Clock Divider
    //     A[7:4] => Oscillator Frequency
    OLED_CS = 1;
}

void
OLED_Set_Display_Enhancement_A(unsigned char a, unsigned char b) {
    OLED_CS = 0;
    Write_Instruction(0xB4);				   // Display Enhancement
    Write_Data(0xA0 | a);				   //   Default => 0xA2
    //     0xA0 (0x00) => Enable External VSL
    //     0xA2 (0x02) => Enable Internal VSL (Kept VSL Pin N.C.)
    Write_Data(0x05 | b);				   //   Default => 0xB5
    //     0xB5 (0xB0) => Normal
    //     0xFD (0xF8) => Enhance Low Gray Scale Display Quality
    OLED_CS = 1;
}

void
OLED_Set_GPIO(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xB5);				   // General Purpose IO
    Write_Data(d);					   //   Default => 0x0A (GPIO Pins output Low Level.)
    OLED_CS = 1;
}

void
OLED_Set_Precharge_Period(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xB6);				   // Set Second Pre-Charge Period
    Write_Data(d);					   //   Default => 0x08 (8 Display Clocks)
    OLED_CS = 1;
}

void
OLED_Set_Precharge_Voltage(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xBB);				   // Set Pre-Charge Voltage Level
    Write_Data(d);					   //   Default => 0x17 (0.50*VCC)
    OLED_CS = 1;
}

void
OLED_Set_VCOMH(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xBE);				   // Set COM Deselect Voltage Level
    Write_Data(d);					   //   Default => 0x04 (0.80*VCC)
    OLED_CS = 1;
}

void
OLED_Set_Contrast_Current(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xC1);				   // Set Contrast Current
    Write_Data(d);					   //   Default => 0x7F
    OLED_CS = 1;
}

void
OLED_Set_Master_Current(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xC7);				   // Master Contrast Current Control
    Write_Data(d);					   //   Default => 0x0f (Maximum)
    OLED_CS = 1;
}

void
OLED_Set_Multiplex_Ratio(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xCA);				   // Set Multiplex Ratio
    Write_Data(d);					   //   Default => 0x7F (1/128 Duty)
    OLED_CS = 1;
}

void
OLED_Set_Display_Enhancement_B(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xD1);				   // Display Enhancement
    Write_Data(0x82 | d);				   //   Default => 0xA2
    //     0x82 (0x00) => Reserved
    //     0xA2 (0x20) => Normal
    Write_Data(0x20);
    OLED_CS = 1;
}

void
OLED_Set_Command_Lock(unsigned char d) {
    OLED_CS = 0;
    Write_Instruction(0xFD);				   // Set Command Lock
    Write_Data(0x12 | d);				   //   Default => 0x12
    //     0x12 => Driver IC interface is unlocked from entering command.
    //     0x16 => All Commands are locked except 0xFD.
    OLED_CS = 1;
}
const static unsigned int width = 256;
const static unsigned int height = 64;


#include "kuukulgur.h"

void
OLED_Show_Logo(void) {        
    unsigned char a=0;
    unsigned char b=height-1;
    unsigned int i, j;
    OLED_Set_Column_Address(OLED_Shift + a, OLED_Shift + b );
    OLED_Set_Row_Address(0,OLED_Max_Row );
    OLED_CS = 0;
    OLED_Set_Write_RAM();

    for (i = 0; i <height ; i++) { /* row */
        for (j = 0; j < width/2; j++) { /* col */        
              Write_Data(pic[i*(width/2)+j]);
	}
    }
    OLED_CS = 1;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
OLED_Fill_RAM(unsigned char Data) {
    unsigned char i, j;
    OLED_Set_Column_Address(0x00, 0x77);
    OLED_Set_Row_Address(0x00, 0x7F);
    OLED_CS = 0;
    OLED_Set_Write_RAM();
    for (i = 0; i < 128; i++) {
	for (j = 0; j < 120; j++) {
	    Write_Data(Data);
	    Write_Data(Data);
	}
    }
    OLED_CS = 1;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End (Total Columns Devided by 4)
//    c: Row Address of Start
//    d: Row Address of End
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
OLED_Fill_Block(unsigned char Data, unsigned char a, unsigned char b,
    unsigned char c, unsigned char d) {
    unsigned char i, j;
    OLED_Set_Column_Address(OLED_Shift + a, OLED_Shift + b);
    OLED_Set_Row_Address(c, d);
    OLED_CS = 0;
    OLED_Set_Write_RAM();
    for (i = 0; i < (d - c + 1); i++) {
	for (j = 0; j < (b - a + 1); j++) {
	    Write_Data(Data);
	    Write_Data(Data);
	}
    }
    OLED_CS = 1;
}
#if 0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Gray Scale Bar (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
OLED_Grayscale() {

	// Level 16 => Column 1~16
	OLED_Fill_Block(0xFF, 0x00, 0x03, 0x00, OLED_Max_Row);

	// Level 15 => Column 17~32
	OLED_Fill_Block(0xEE, 0x04, 0x07, 0x00, OLED_Max_Row);

	// Level 14 => Column 33~48
	OLED_Fill_Block(0xDD, 0x08, 0x0B, 0x00, OLED_Max_Row);

	// Level 13 => Column 49~64
	OLED_Fill_Block(0xCC, 0x0C, 0x0F, 0x00, OLED_Max_Row);

	// Level 12 => Column 65~80
	OLED_Fill_Block(0xBB, 0x10, 0x13, 0x00, OLED_Max_Row);

	// Level 11 => Column 81~96
	OLED_Fill_Block(0xAA, 0x14, 0x17, 0x00, OLED_Max_Row);

	// Level 10 => Column 97~112
	OLED_Fill_Block(0x99, 0x18, 0x1B, 0x00, OLED_Max_Row);

	// Level 9 => Column 113~128
	OLED_Fill_Block(0x88, 0x1C, 0x1F, 0x00, OLED_Max_Row);

	// Level 8 => Column 129~144
	OLED_Fill_Block(0x77, 0x20, 0x23, 0x00, OLED_Max_Row);

	// Level 7 => Column 145~160
	OLED_Fill_Block(0x66, 0x24, 0x27, 0x00, OLED_Max_Row);

	// Level 6 => Column 161~176
	OLED_Fill_Block(0x55, 0x28, 0x2B, 0x00, OLED_Max_Row);

	// Level 5 => Column 177~192
	OLED_Fill_Block(0x44, 0x2C, 0x2F, 0x00, OLED_Max_Row);

	// Level 4 => Column 193~208
	OLED_Fill_Block(0x33, 0x30, 0x33, 0x00, OLED_Max_Row);

	// Level 3 => Column 209~224
	OLED_Fill_Block(0x22, 0x34, 0x37, 0x00, OLED_Max_Row);

	// Level 2 => Column 225~240
	OLED_Fill_Block(0x11, 0x38, 0x3B, 0x00, OLED_Max_Row);

	// Level 1 => Column 241~256
	OLED_Fill_Block(0x00, 0x3C, OLED_Max_Column, 0x00, OLED_Max_Row);
}
#endif

#define OLED_BRIGHTNESS    15
#define OLED_BR_1B      OLED_BRIGHTNESS<<4
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Character (5x7)
//
//    a: Database
//    b: Ascii
//    c: Start X Address
//    d: Start Y Address
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
OLED_Show_Font57(unsigned char a, unsigned char b, unsigned char c,
    unsigned char d) {
    const unsigned char *Src_Pointer;
    unsigned char i,p,n, Font, MSB_1, LSB_1, MSB_2, LSB_2;
    switch (a) {
    case 1:
	Src_Pointer = &Ascii_1[(b - 1)][0];
	break;
#if 0
    case 2:
	Src_Pointer = &Ascii_2[(b - 1)][0];
	break;
#endif
    }
    OLED_Set_Remap_Format(0x15);
    for (i = 0; i <= 1; i++) {      // 
	MSB_1 = *Src_Pointer;       // MSB_1 = first font databyte
	Src_Pointer++;
	if (i == 1) {
	    LSB_1 = 0x00;
	    MSB_2 = 0x00;
	    LSB_2 = 0x00;
	} else {
	    LSB_1 = *Src_Pointer;
	    Src_Pointer++;
	    MSB_2 = *Src_Pointer;
	    Src_Pointer++;
	    LSB_2 = *Src_Pointer;
	    Src_Pointer++;
	}
	OLED_Set_Column_Address(OLED_Shift + c, OLED_Shift + c);
	OLED_Set_Row_Address(d, d + 7);
        OLED_CS = 0;
	OLED_Set_Write_RAM();
        n=1;
        for (p = 0; p < 8; p++){
            if (MSB_1 & n) 
                Font = OLED_BR_1B; 
            else 
                Font = 0;
            if (LSB_1 & n) 
                Font |= OLED_BRIGHTNESS; 
            else 
                Font &= 0xF0;
	    Write_Data(Font);
            if (MSB_2 & n) 
                Font = OLED_BR_1B; 
            else 
                Font = 0;
            if (LSB_2 & n) 
                Font |= OLED_BRIGHTNESS; 
            else 
                Font &= 0xF0;
            Write_Data(Font);
            n<<=1;
        }
        OLED_CS = 1;
	c++;
    }
    OLED_Set_Remap_Format(0x14);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show String
//
//    a: Database
//    b: Start X Address
//    c: Start Y Address
//    * Must write "0" in the end.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
OLED_Show_String(unsigned char a,  char *Data_Pointer, unsigned char b, unsigned char c) {
    char *Src_Pointer;
    Src_Pointer = Data_Pointer;
    OLED_Show_Font57(1, 32, b, c);			   // No-Break Space
    //   Must be written first before the string start.
    do {
	OLED_Show_Font57(a, *Src_Pointer, b, c);
	Src_Pointer++;
	b += 2;
    } while (*Src_Pointer != 0);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End (Total Columns Devided by 4)
//    c: Row Address of Start
//    d: Row Address of End
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
OLED_Show_Pattern(unsigned char *Data_Pointer, unsigned char a,
    unsigned char b, unsigned char c, unsigned char d) {
    unsigned char *Src_Pointer;
    unsigned char i, j;
    Src_Pointer = Data_Pointer;
    OLED_Set_Column_Address(OLED_Shift + a, OLED_Shift + b);
    OLED_Set_Row_Address(c, d);
    OLED_Set_Write_RAM();
    for (i = 0; i < (d - c + 1); i++) {
	for (j = 0; j < (b - a + 1); j++) {
	    Write_Data(*Src_Pointer);
	    Src_Pointer++;
	    Write_Data(*Src_Pointer);
	    Src_Pointer++;
	}
    }
}

void
OLED_Fade_In() {
    unsigned char i;
    OLED_Set_Display_On_Off(0x01);
    for (i = 0; i < (OLED_Brightness + 1); i++) {
	OLED_Set_Master_Current(i);
	uDelay(200);
	uDelay(200);
	uDelay(200);
    }
}

void
OLED_Fade_Out() {
    unsigned char i;
    for (i = (OLED_Brightness + 1); i > 0; i--) {
	OLED_Set_Master_Current(i - 1);
	uDelay(200);
	uDelay(200);
	uDelay(200);
    }
    OLED_Set_Display_On_Off(0x00);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Sleep Mode
//
//    "0x01" Enter Sleep Mode
//    "0x00" Exit Sleep Mode
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
OLED_Sleep(unsigned char a) {
    switch (a) {
    case 0:
	OLED_Set_Display_On_Off(0x00);
	OLED_Set_Display_Mode(0x01);
	break;
    case 1:
	OLED_Set_Display_Mode(0x02);
	OLED_Set_Display_On_Off(0x01);
	break;
    }
}

void
OLED_Set_Gray_Scale_Table() {
    Write_Instruction(0xB8);				   // Set Gray Scale Table
    Write_Data(0x0C);				   //   Gray Scale Level 1
    Write_Data(0x18);				   //   Gray Scale Level 2
    Write_Data(0x24);				   //   Gray Scale Level 3
    Write_Data(0x30);				   //   Gray Scale Level 4
    Write_Data(0x3C);				   //   Gray Scale Level 5
    Write_Data(0x48);				   //   Gray Scale Level 6
    Write_Data(0x54);				   //   Gray Scale Level 7
    Write_Data(0x60);				   //   Gray Scale Level 8
    Write_Data(0x6C);				   //   Gray Scale Level 9
    Write_Data(0x78);				   //   Gray Scale Level 10
    Write_Data(0x84);				   //   Gray Scale Level 11
    Write_Data(0x90);				   //   Gray Scale Level 12
    Write_Data(0x9C);				   //   Gray Scale Level 13
    Write_Data(0xA8);				   //   Gray Scale Level 14
    Write_Data(0xB4);				   //   Gray Scale Level 15
    Write_Instruction(0x00);					   // Enable Gray Scale Table
} 

void
OLED_Set_Linear_Gray_Scale_Table() {
    Write_Instruction(0xB9);				   // Set Default Linear Gray Scale Table
}
volatile char mode=E_BAT;
volatile char mode_just_changed=1;

static void 
mode_e_err(void) {
    char buf[SCREENWIDTH+1];
    snprintf(buf,sizeof(buf),"Errorlog");
    OLED_Show_String(  1,buf, 0, 0*8);
    for(int i=0;i<MAXERRORS;i++) {
        OLED_Show_String( 1,errlog[i], 0, (i+1)*8);
    }
}

static void 
mode_e_supply(void) {
    char buf[SCREENWIDTH+1];
    snprintf(buf,sizeof(buf),"Power supply");
    OLED_Show_String(  1,buf, 0, 0*8);
    
//    OUT1_EN = MAX1614_OFF; // Configure all power supply main outputs   
//    OUT2_EN = MAX1614_OFF; 
//    OUT3_EN = MAX1614_OFF;

#if 0    
    ENABLE_PWR=1; /* Enable TPS51222. 
                   * Basically 5V is enabled to ACS712 current sensors
                   */
    OUT4_EN = 1; // Configure TPS51222 DC DC outputs
    OUT5_EN = 1; // 1 is enable, 0 is disable
#endif
    
    
    snprintf(buf,sizeof(buf),"Main EN %d",ENABLE_PWR);
    OLED_Show_String(  1,buf, 0, 1*8);

    snprintf(buf,sizeof(buf),"EN1 %d",OUT4_EN);
    OLED_Show_String(  1,buf, 0, 2*8);
    snprintf(buf,sizeof(buf),"EN2 %d",OUT5_EN);
    OLED_Show_String(  1,buf, 0, 3*8);

    snprintf(buf,sizeof(buf),"SW1 %d",(SWITCHHOLD_1 & gSwitchFlag) ? 1:0);
    OLED_Show_String(  1,buf, 0, 4*8);
    snprintf(buf,sizeof(buf),"PGOOD1 %d",PGOOD1);
    OLED_Show_String(  1,buf, 0, 5*8);
    snprintf(buf,sizeof(buf),"PGOOD2 %d",PGOOD2);
    OLED_Show_String(  1,buf, 0, 6*8);

#if 1
    if(SWITCHHOLD_1 & gSwitchFlag) {
        if(OUT4_EN == 1 && ENABLE_PWR==1) {
            OUT4_EN=0;
            OUT5_EN=0;
           // ENABLE_PWR=0;
        } else if(ENABLE_PWR==0) {
            ENABLE_PWR=1;
        } else {
            OUT4_EN = 1;
            OUT5_EN = 1;
            gSwitchFlag &= (SWITCHPRESS_ALL | ~SWITCHHOLD_1);
            gSwitchFlag |= SWITCHPRESS_1;
        }
    }
#endif    
}
static void 
mode_e_gyro(void) {
    char buf[SCREENWIDTH+1];
    snprintf(buf,sizeof(buf),"Gyros");
    OLED_Show_String(  1,buf, 0, 0*8);

    snprintf(buf,sizeof(buf),"g0: %6d %6d %6d",gyro0[0],gyro0[1],gyro0[2]);
    OLED_Show_String(  1,buf, 0, 1*8);
    snprintf(buf,sizeof(buf),"g1: %6d %6d %6d",gyro1[0],gyro1[1],gyro1[2]);
    OLED_Show_String(  1,buf, 0, 2*8);
    snprintf(buf,sizeof(buf),"gs: %6d %6d %6d",(gyro0[0]+gyro1[0])/2,(gyro0[1]+gyro1[1])/2,(gyro0[2]+gyro1[2])/2);
    OLED_Show_String(  1,buf, 0, 3*8);

    snprintf(buf,sizeof(buf),"a0: %6d %6d %6d",accel0[0],accel0[1],accel0[2]);
    OLED_Show_String(  1,buf, 0, 4*8);
    snprintf(buf,sizeof(buf),"a1: %6d %6d %6d",accel1[0],accel1[1],accel1[2]);
    OLED_Show_String(  1,buf, 0, 5*8);
    snprintf(buf,sizeof(buf),"as: %6d %6d %6d",(accel0[0]+accel1[0])/2,(accel0[1]+accel1[1])/2,(accel0[2]+accel1[2])/2);
    OLED_Show_String(  1,buf, 0, 6*8);

    
}


static void 
mode_e_bat(void) {
    char buf[SCREENWIDTH+1]; 
    snprintf(buf,sizeof(buf),"mode switch %d, adapter %.1f",PORT3.PIDR.BIT.B4,adapter);
    OLED_Show_String(  1,buf, 0, 5*8);
    snprintf(buf,sizeof(buf),"steering %3.1fA (max %3.1fA)",imon2,imon2max);
    OLED_Show_String(  1,buf, 0, 6*8);
#define BAT_MISSING_THRESHOLD (2.5f*3.f) 
#define BAT_CRIT_THRESHOLD (3.3f*3.f) 
#define BAT_LOW_THRESHOLD  (3.5f*3.f)
#define BAT_FULL_THRESHOLD (4.2f*3.f)
    
    for(int i=0;i<4;i++) {
        char *statustext;
        float percent;
        percent=(adc[i]-BAT_CRIT_THRESHOLD)/(BAT_FULL_THRESHOLD-BAT_CRIT_THRESHOLD)*100.0f;
        if(adc[i]<=BAT_MISSING_THRESHOLD) {
            statustext="Missing";
            percent=0.0f;
        } else if(adc[i]<=BAT_CRIT_THRESHOLD) {
            statustext="Criticl";
            percent=0.0f;
        } else if(adc[i]<=BAT_LOW_THRESHOLD) {
            statustext="Low";
        } else if(adc[i]>BAT_FULL_THRESHOLD) {
            statustext="Overvlt";
            percent=0.0f;
        } else {
            statustext="Normal";
        }
        
        float current=adc[i+4];
        char *status[4];
        switch(i) {
        case 0:
            if(BAT0_EN==MAX1614_ON)
                status[0]="EN ";
            else {
                status[0]="DIS";
                current=0.0f;
            }
            break;
        case 1:
            if(BAT1_EN==MAX1614_ON)
                status[1]="EN ";
            else {
                status[1]="DIS";
                current=0.0f;
            }
            break;
        case 2:
            if(BAT2_EN==MAX1614_ON)
                status[2]="EN ";
            else {
                status[2]="DIS";
                current=0.0f;
            }
            break;
        case 3:
            if(BAT3_EN==MAX1614_ON)
                status[3]="EN ";
            else {
                status[3]="DIS";
                current=0.0f;
            }
            break;
        }
        
        snprintf(buf,sizeof(buf),"%d: %4.1fV %5.1fA %7s%3.0f%% %3s",i,adc[i],current,statustext,percent,status[i]);
        OLED_Show_String(  1, buf, 0, (i+1)*8);
    }  
}

void
refresh(void) {

    if(mode_just_changed) {
        OLED_Fill_RAM(0x00); // clear screen
        mode_just_changed=0;
    }    

    switch(mode) {
    case E_BAT:
        mode_e_bat();
        break;
    case E_ERR:
        mode_e_err();
        break;
    case E_GYRO:
        mode_e_gyro();
        break;
    case E_SUPPLY:
        mode_e_supply();
        break;   
    }      
  
}


void
Init_OLED() {
  
    OLED_DC_PORT_DIR=1;
    OLED_CS_PORT_DIR=1;
    OLED_WR_PORT_DIR=1;
    OLED_RD_PORT_DIR=1;
    OLED_RESET_PORT_DIR=1;
    PORTD.PDR.BYTE=0xFF;
    OLED_CS=1;
    OLED_WR=1;
    OLED_RD=1;

    OLED_RESET = 0;    
    __delay_cycles(100UL*2000UL); // 2000us delay - required in datasheet
    OLED_RESET = 1;
    __delay_cycles(100UL*2000UL); // 2000us delay    

    OLED_Set_Command_Lock(0x12);			   // Unlock Basic Commands (0x12/0x16)
    OLED_Set_Display_On_Off(0x00);			   // Display Off (0x00/0x01)
    OLED_Set_Display_Clock(0xD0);			   // Set Clock as 80 Frames/Sec
    OLED_Set_Multiplex_Ratio(0x3F);			   // 1/64 Duty (0x0F~0x3F)
    OLED_Set_Display_Offset(0x00);			   // OLED_Shift Mapping RAM Counter (0x00~0x3F)
    OLED_Set_Start_Line(0x00);				   // Set Mapping RAM Display Start Line (0x00~0x7F)
    OLED_Set_Remap_Format(0x14);			   // Set Horizontal Address Increment
    //OLED_Set_GPIO(0x00);				   // Disable GPIO Pins Input
    OLED_Set_Function_Selection(0x01);			   // Enable Internal VDD Regulator
    OLED_Set_Display_Enhancement_A(0xA0, 0xFD);		   // Enable External VSL
    // Set Low Gray Scale Enhancement
    OLED_Set_Contrast_Current(0xDF);			   // Set Segment Output Current
    OLED_Set_Master_Current(OLED_Brightness);		   // Set Scale Factor of Segment Output Current Control
    OLED_Set_Gray_Scale_Table();			   // Set Pulse Width for Gray Scale Table
    OLED_Set_Phase_Length(0xE8);			   // Set Phase 1 as 5 Clocks & Phase 2 as 14 Clocks
    OLED_Set_Display_Enhancement_B(0x20);		   // Enhance Driving Scheme Capability (0x00/0x20)
    OLED_Set_Precharge_Voltage(0x07);			   // Set Pre-Charge Voltage Level as 0.60*VCC
    OLED_Set_Precharge_Period(0x28);			   // Set Second Pre-Charge Period as 8 Clocks
    OLED_Set_VCOMH(0x07);				   // Set Common Pins Deselect Voltage Level as 0.86*VCC
    OLED_Set_Display_Mode(0x02);			   // Normal Display Mode (0x00/0x01/0x02/0x03)
    OLED_Set_Partial_Display(0x01, 0x00, 0x00);		   // Disable Partial Display
    OLED_Fill_RAM(0x00);
    OLED_Set_Display_On_Off(0x01);			   // Display On (0x00/0x01)
    OLED_Show_Logo();
    __delay_cycles(10UL*1000UL*1000UL); // 0.1s delay    

}


