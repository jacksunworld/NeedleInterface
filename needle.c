//needle.c writed by jack.sun 2018.5.4.
//current register's internal implementation is confusing.
//it's a driver to mask internal clutter and provied a uniform interface
//access register by bit, access register with it's name
//this code is useful for easy access our current sensor's register .
#include "needle.h"
#include <string.h>

#define REG_NUM 70
int MS = 7;
int DATAREAD[8]; //To store pg50 read pattern in dec form
u8 TestReg[10] = {0xf1, 0x2, 0x83, 0x24, 0x15, 0xd6, 7, 0xc8, 0xf9};
NeedleReg RegList[REG_NUM] = {
	{"OSC", 0x40, 0x10, 0x03},
	{"VBG", 0x40, 0x10, 0x47},
	{"LDO_EN", 0x41, 0x00, 0x01},
	{"vdaTrm", 0x41, 0x10, 0x24},
	{"EnableTempFilter", 0x00, 0x00, 0x55},
	{"disable_sda_pul_down", 0x00, 0x00, 0x66},
	{"master", 0x41, 0x00, 0x77},
	{"TempSlope", 0x42, 0x10, 0x07},
	{"TempOfst", 0x43, 0x10, 0x07},
	{"AGain", 0x4445, 0x00, 0x0703},
	{"BGain", 0x4546, 0x01, 0x4707},
	{"Cgain", 0x4748, 0x11, 0x0703},
	{"Dgain", 0x4849, 0x10, 0x4703},
	{"AOfstRat", 0x494A, 0x10, 0x4707},
	{"BOfstRat", 0x4B4C, 0x11, 0x0703},
	{"COfstRat", 0x4C4D, 0x13, 0x4707},
	{"AOfstABS1", 0x4E4F, 0x10, 0x0703},
	{"BOfstABS1", 0x4F50, 0x11, 0x4707},
	{"COfstABS1", 0x5152, 0x13, 0x0703},
	{"AOfstABS2", 0x5253, 0x10, 0x4705},
	{"BOfstABS2", 0x5354, 0x12, 0x6707},
	{"AVMIDAbs", 0x55, 0x10, 0x07},
	{"BVMIDAbs", 0x56, 0x12, 0x07},
	{"SensGain", 0x5758, 0x00, 0x0704},
	{"VREF_EN", 0x58, 0x00, 0x55},
	{"OCP_latch_en", 0x58, 0x00, 0x66},
	{"OCP_en", 0x58, 0x00, 0x77},
	{"VMIDRat", 0x59, 0x10, 0x07},
	{"Extra_Sens_Ofst", 0x5A, SM_TYPE, 0x05},
	{"Exp2", 0x5B, SM_TYPE, 0x27},
	{"Exp3", 0x5C5B, SM_TYPE, 0x4701},
	{"i2c_addr", 0x5c, 0x00, 02},
	{"ForceSDA2", 0x5c, 0x00, 0x33},
	{"LDO_Trim", 0x5d, 0x10, 0x03},
	{"gain_sel", 0x5d, 0x00, 0x47},
	{"ATM", 0x5e, 0x00, 0x07},
	{"load_adj", 0x5f, 0x00, 0x01},
	{"ofstabs1_rng", 0x5f, 0x00, 0x22},
	{"ofstrat_rng", 0x5f, 0x00, 0x34},
	{"ge_tmp", 0x5f, 0x00, 0x56},
	{"higher_BW", 0x5f, 0x00, 0x77},
	{"ocprefn", 0x60, 0x00, 0x27},
	{"ocprefp", 0x61, 0x00, 0x27},
};

void Pattern_Write(void)
{
}
void I2C_Write(int ADDR, int DATA)
{
	TestReg[ADDR % 10] = DATA;
}
void Pattern_Read(void)
{
}
void I2C_Read(int ADDR)
{
	DATAREAD[MS - 1] = TestReg[ADDR % 10];
}
void wait_ms(u8 cnt)
{
}

int sc_strnocasecmp(char *cmp1, char *cmp2, u8 len)
{
	while (len-- > 0)
	{
		while (*cmp1 != '\0' && *cmp2 != '\0')
		{
			if (*cmp1 != *cmp2)
				return *cmp1 - *cmp2;
			cmp1++;
			cmp2++;
		}
	}
	return 0;
}

static int FindRegIndexWithName(char *reg_name)
{
	int i;

	for (i = 0; i < REG_NUM; i++)
	{
		if (sc_strnocasecmp(reg_name, RegList[i].name, strlen(RegList[i].name)) == 0)
			break;
	}
	if (i >= REG_NUM)
		return ERROR;
	else
		return i;
}
static u8 InvokeI2cWrite(u8 addr, u8 value, u8 index)
{
	u8 read_data = 0, i = 0, mask = 0xff;
	if (index == 0x07)
	{
		I2C_Write(addr, value);
		Pattern_Write();
	}
	else
	{
		I2C_Read(addr); //first read
		Pattern_Read();
		read_data = DATAREAD[MS - 1]; //read_data stored the reasered value

		i = (index & 0xf0) >> 4;
		mask = mask << i;
		value = value << i; //in fact,we just need know the left shift count ,and is enough

		i = (index & 0x0f) + 1;
		i = 8 - i;
		mask = mask >> i;
		mask = ~mask;
		read_data = read_data & mask;

		value = read_data | value;
		I2C_Write(addr, value);
		wait_ms(1);
		Pattern_Write();
	}
	return value;
}

static u8 InvokeI2cRead(u8 addr, u8 index)
{
	u8 read_data = 0;
	u8 bc = 0;
	I2C_Read(addr); //first read
	Pattern_Read();
	read_data = DATAREAD[MS - 1]; //read_data stored the reasered value

	if (index == 0x07)
	{
		return read_data;
	}
	else
	{
		//read_data stored the reasered value
		bc = 8 - ((index & 0xf) + 1);
		read_data = read_data << bc; //set the high bit  that we didn't need to zero;
		read_data = read_data >> bc;
		bc = (index >> 4);
		read_data = (read_data >> bc); //set the low bit that we didn't need to zero;
	}
	return read_data;
}

static int ConvertValueBinary(float valuef, u8 len, u8 regtype)
{
	int ret_value = 0;
	int valuei = (int)valuef;

	if ((regtype >> 4) == 2) // SM_TYPE
	{
		if (valuei >= 0)
		{
			if (valuei > (~(INTMAX << len)))
				valuei = (~(INTMAX << len));

			ret_value = valuei;
		}
		else
		{
			valuei = 0 - valuei; //get true value
			if (valuei > (~(INTMAX << len)))
				valuei = (~(INTMAX << len));

			ret_value = valuei | (1 << (len)); //set sign bit
		}
	}
	else if ((regtype >> 4) == 1) //S2_type
	{
		if (valuei >= 0)
		{
			valuei = (valuei << (regtype & 0x0f));
			if (valuei > (~(INTMAX << len)))
				valuei = (~(INTMAX << len));

			ret_value = valuei;
		}
		else
		{
			valuei = 0 - valuei; //get true value
			valuei = (valuei << (regtype & 0x0f));
			if (valuei > (~(INTMAX << len)))
				valuei = (~(INTMAX << len));
			valuei = 0 - valuei;
			ret_value = valuei | (1 << (len)); //set sign bit
		}
	}
	else if ((regtype >> 4) == 0) //unsigned_type
	{
		if (valuei < 0)
		{
			ret_value = ERROR;
			goto ret;
		}
		len = len + 1; //unsigned_type  len need to add 1,for added the signed bit
		valuei = (valuei << (regtype & 0x0f));
		if (valuei > (~(INTMAX << len)))
			valuei = (~(INTMAX << len));

		ret_value = valuei;
	}
ret:
	return ret_value;
}

static float ConvertBinaryValue(int valuei, u8 len, u8 regtype)
{
	float ret_value = 0;
	float valuef = valuei;

	if ((regtype >> 4) == 2) // SM_TYPE
	{
		if ((valuei & (1 << len)) == 0)
			ret_value = valuef;
		else
			ret_value = 0 - (valuei & (~(1 << len)));
	}
	else if ((regtype >> 4) == 1) //S2_type
	{
		if ((valuei & (1 << len)) == 0)
			ret_value = valuef;
		else
		{
			ret_value = ((0 - valuei) & (~(INTMAX << len)));
			ret_value = 0 - ret_value;
		}

		if ((regtype & 0x0f) == 0)
			ret_value = ret_value / 0.99999;
		else if ((regtype & 0x0f) == 1)
		{
			ret_value = ret_value / 1.99999;
		}
		else if ((regtype & 0x0f) == 2)
		{
			ret_value = ret_value / 3.99999;
		}
		else if ((regtype & 0x0f) == 3)
		{
			ret_value = ret_value / 7.99999;
		}
		else if ((regtype & 0x0f) == 4)
		{
			ret_value = ret_value / 15.99999;
		}
		else if ((regtype & 0x0f) == 5)
		{
			ret_value = ret_value / 31.99999;
		}
		else
			ret_value = ret_value / 63.99999;
	}
	else if ((regtype >> 4) == 0) //unsigned_type
	{
		if ((regtype & 0x0f) == 0)
			ret_value = valuei;
		else if ((regtype & 0x0f) == 1)
		{
			ret_value = valuei / 2;
		}
		else
			ret_value = valuei / 4;
	}

	return ret_value;
}

// our rigister 0 is Temp_Raw_0, it's read only and didn't extend to register 1 ,so it work fine on this status
int NeedleWriteReg(char *reg_name, float value)
{
	int i;
	u8 addr1 = 0, addr2 = 0;
	u8 index_addr1, index_addr2;
	u8 value1 = 0, value2 = 0, valuelen1 = 0, valuelen2 = 0;
	int retvalue = 0;
	i = FindRegIndexWithName(reg_name);
	if ((i < REG_NUM) && (i != ERROR))
	{
		//            if(RegList[i].type==US_TYPE)
		addr1 = (RegList[i].reg >> 8);
		addr2 = (RegList[i].reg & 0x00FF);
		if (addr1 == 0) //only one reg
		{

			addr1 = addr2;
			index_addr1 = RegList[i].index & 0xFF;
			valuelen1 = (index_addr1 & 0x0f) - (index_addr1 >> 4) + 1 - 1; //(index_addr1&0x0f)-(index_addr1>>4)+1  this is total bit length, then minus 1 mean reduce the one bit of signed bit;
			if ((valuelen1 > 7) || (value > (1 << (valuelen1 + 1))))
			{
				retvalue = ERROR;
				goto ret;
			}
			retvalue = ConvertValueBinary(value, valuelen1, RegList[i].type); //if type is unsigned ,the signed bit will be added to len in this function ConvertValueBinary
			if (retvalue == ERROR)
				goto ret;
			value1 = (u8)retvalue;
			retvalue = InvokeI2cWrite(addr1, value1, index_addr1);
		}
		else
		{
			index_addr1 = RegList[i].index >> 8;
			index_addr2 = RegList[i].index & 0x00FF;
			valuelen1 = (index_addr1 & 0x0f) - (index_addr1 >> 4) + 1; //fist calc the length of bit in low register.
			valuelen2 = (index_addr2 & 0x0f) - (index_addr2 >> 4) + 1; //then  the high register's.
																	   //at last minus the signed bit, if type is unsigned ,it will be recover by the ConvertValueBinary
			retvalue = ConvertValueBinary(value, valuelen1 + valuelen2 - 1, RegList[i].type);
			if (retvalue == ERROR)
				goto ret;

			value1 = retvalue & (~(INTMAX << valuelen1));
			value2 = (retvalue - value1) >> valuelen1;
			InvokeI2cWrite(addr1, value1, index_addr1);
			InvokeI2cWrite(addr2, value2, index_addr2);
		}
	}
	else
	{
		retvalue = ERROR;
	}
ret:
	return retvalue;
}

float NeedleReadReg(char *reg_name)
{
	int i, value = 0;
	u8 addr1 = 0, addr2 = 0;
	u8 index_addr1, index_addr2;
	u8 value1 = 0, value2 = 0, valuelen1 = 0, valuelen2 = 0;
	float ret = 0;
	i = FindRegIndexWithName(reg_name);
	if ((i < REG_NUM) && (i != ERROR))
	{
		addr1 = (RegList[i].reg >> 8);
		addr2 = (RegList[i].reg & 0x00FF);
		if (addr1 == 0) //only one reg
		{
			addr1 = addr2;
			index_addr1 = RegList[i].index & 0xFF;
			value1 = InvokeI2cRead(addr1, index_addr1);
			valuelen1 = (index_addr1 & 0x0f) - (index_addr1 >> 4) + 1 - 1;
			ret = ConvertBinaryValue(value1, valuelen1, RegList[i].type);
		}
		else
		{
			index_addr1 = RegList[i].index >> 8;
			index_addr2 = RegList[i].index & 0x00FF;

			valuelen1 = (index_addr1 & 0x0f) - (index_addr1 >> 4) + 1;
			valuelen2 = (index_addr2 & 0x0f) - (index_addr2 >> 4) + 1;
			value1 = InvokeI2cRead(addr1, index_addr1);
			value2 = InvokeI2cRead(addr2, index_addr2);
			value = (value1 | ((int)value2 << valuelen1));
			ret = ConvertBinaryValue(value, valuelen1 + valuelen2 - 1, RegList[i].type);
		}
	}
	else
	{
		ret = ERROR;
	}

	return ret;
}
