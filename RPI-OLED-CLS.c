#include <wiringPi.h>
#define byte unsigned char
#define Max_Column 128
#define X_WIDTH 128
//======================================

void LCD_Init(void);
void LCD_CLS(void);
void LCD_Fill(byte dat);

void LCD_WrDat(unsigned char dat) {
  unsigned char i=8;
  digitalWrite(10, 0); //LCD_CS=0;
  digitalWrite(5, 1); //LCD_DC=1;
  digitalWrite(14, 0); //LCD_SCL=0;
  while(i--) {
    if(dat&0x80) {
      digitalWrite(12, 1); //LCD_SDA=1;
    } else {
      digitalWrite(12, 0); //LCD_SDA=0;
    }
    digitalWrite(14, 1); //LCD_SCL=1;
    ;;;
    digitalWrite(14, 0); //LCD_SCL=0;
    dat<<=1;
  }
  digitalWrite(10, 1); //LCD_CS=1;
}
void LCD_WrCmd(unsigned char cmd) {
  unsigned char i=8;
  digitalWrite(10, 0); //LCD_CS=0;
  digitalWrite(5, 0); //LCD_DC=0;
  digitalWrite(14, 0); //LCD_SCL=0;
  while(i--) {
    if(cmd&0x80) {
      digitalWrite(12, 1); //LCD_SDA=1;
    } else {
      digitalWrite(12, 0); //LCD_SDA=0;
    }
    digitalWrite(14, 1); //LCD_SCL=1;
    ;;;
    digitalWrite(14, 0); //LCD_SCL=0;;
    cmd<<=1;;
  }
  digitalWrite(10, 1); //LCD_CS=1;
}
void LCD_Set_Pos(unsigned char x, unsigned char y) {
  LCD_WrCmd(0xb0+y);
  LCD_WrCmd(((x&0xf0)>>4)|0x10);
  LCD_WrCmd((x&0x0f)|0x00);
}
void LCD_Fill(unsigned char bmp_dat) {
  unsigned char y,x;
  for(y=0; y<8; y++) {
    LCD_WrCmd(0xb0+y);
    LCD_WrCmd(0x01);
    LCD_WrCmd(0x10);
    for(x=0; x<X_WIDTH; x++)
      LCD_WrDat(bmp_dat);
  }
}
void LCD_CLS(void) {
  unsigned char y,x;
  for(y=0; y<8; y++) {
    LCD_WrCmd(0xb0+y);
    LCD_WrCmd(0x01);
    LCD_WrCmd(0x10);
    for(x=0; x<X_WIDTH; x++)
      LCD_WrDat(0);
  }
}
void LCD_DLY_ms(unsigned int ms) {
  unsigned int a;
  while(ms) {
    a=1335;
    while(a--);
    ms--;
  }
  return;
}
void LCD_Init(void) {
  digitalWrite(14, 1); //LCD_SCL=1;
  digitalWrite(10, 1); //LCD_CS=1;
  digitalWrite(6, 0); //LCD_RST=0;
  delay(50); //LCD_DLY_ms(50);
  digitalWrite(6, 1); //LCD_RST=1;
  LCD_WrCmd(0xae);//--turn off oled panel
  LCD_WrCmd(0x00);//---set low column address
  LCD_WrCmd(0x10);//---set high column address
  LCD_WrCmd(0x40);//--set start line address Set Mapping RAM Display Start Line (0x00~0x3F)
  LCD_WrCmd(0x81);//--set contrast control register
  LCD_WrCmd(0xcf); // Set SEG Output Current Brightness
  LCD_WrCmd(0xa1);//--Set SEG/Column Mapping
  LCD_WrCmd(0xc8);//Set COM/Row Scan Direction
  LCD_WrCmd(0xa6);//--set normal display
  LCD_WrCmd(0xa8);//--set multiplex ratio(1 to 64)
  LCD_WrCmd(0x3f);//--1/64 duty
  LCD_WrCmd(0xd3);//-set display offset Shift Mapping RAM Counter (0x00~0x3F)
  LCD_WrCmd(0x00);//-not offset
  LCD_WrCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  LCD_WrCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
  LCD_WrCmd(0xd9);//--set pre-charge period
  LCD_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  LCD_WrCmd(0xda);//--set com pins hardware configuration
  LCD_WrCmd(0x12);
  LCD_WrCmd(0xdb);//--set vcomh
  LCD_WrCmd(0x40);//Set VCOM Deselect Level
  LCD_WrCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
  LCD_WrCmd(0x02);//
  LCD_WrCmd(0x8d);//--set Charge Pump enable/disable
  LCD_WrCmd(0x14);//--set(0x10) disable
  LCD_WrCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
  LCD_WrCmd(0xa6);// Disable Inverse Display On (0xa6/a7)
  LCD_WrCmd(0xaf);//--turn on oled panel
  LCD_Fill(0x00);
  LCD_Set_Pos(0,0);
}

void main(int argc, char *argv[]) {
  unsigned char i=0;
  wiringPiSetup();
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(6, OUTPUT);
  LCD_Init();
  LCD_Fill(0x00);
  LCD_WrCmd(0xae);
}
