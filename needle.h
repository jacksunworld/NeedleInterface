// needle.h writed by jack.sun 2018.5.4,this code is useful for easy access our current sensor's register.


#ifndef __NEEDLE_H
#define __NEEDLE_H
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#define INTMAX   0xffffffff

#define ERROR   -1
#define SUCCESS  0

#define S2_TYPE	            0x10	        
#define SM_TYPE			    0x20

typedef struct {
	char *name;					//register name
	u32		reg;				//register addr
	u8		type;				//register type,such as 2's complemnet or float,or int
	u16		index;				//describe register the bit's range,occupied in the addr
}NeedleReg;


int ConvertValueBinary(float valuef,u8 len,u8 regtype);   
u8 InvokeI2cWrite(u8 addr,u8 value,u8 index);
int FindRegIndexWithName(char *reg_name);
int NeedleWriteReg(char *reg_name,float value);
float NeedleReadReg(char *reg_name);
float ConvertBinaryValue(int valuei,u8 len,u8 regtype);


#endif



