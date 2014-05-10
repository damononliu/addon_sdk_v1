/**
 * @file			matrixp10.c
 * @brief			����p10��Ԫ��
 *					�Ա����ۣ�http://item.taobao.com/item.htm?spm=a1z09.5.0.0.DzRqoH&id=10843524318
 * @author			yizuoshe@gmail.com
*/



#include "include.h"

#define PIN_OE 	WIFIIO_GPIO_17
#define PIN_A 	WIFIIO_GPIO_18
#define PIN_B		WIFIIO_GPIO_19
#define PIN_SCK	WIFIIO_GPIO_20
#define PIN_STB	WIFIIO_GPIO_21
#define PIN_DAT	WIFIIO_GPIO_22

#define PIN_OE_LOW api_io.low(PIN_OE)
#define PIN_OE_HIGH api_io.high(PIN_OE)

#define PIN_SCK_LOW api_io.low(PIN_SCK)
#define PIN_SCK_HIGH api_io.high(PIN_SCK)

#define PIN_STB_LOW api_io.low(PIN_STB)
#define PIN_STB_HIGH api_io.high(PIN_STB)

#define PIN_DAT_LOW api_io.low(PIN_DAT)
#define PIN_DAT_HIGH api_io.high(PIN_DAT)

#define PIN_A_LOW api_io.low(PIN_A)
#define PIN_A_HIGH api_io.high(PIN_A)

#define PIN_B_LOW api_io.low(PIN_B)
#define PIN_B_HIGH api_io.high(PIN_B)


u8_t play_buf[64];

extern const u8_t *ASCII8x8;


//���Կ��� 
void HC138_scan(u8_t temp) 
{
	PIN_OE_HIGH;
	if(0x01 & temp)
		PIN_A_HIGH;
	else
		PIN_A_LOW;

	if(0x01 &(temp>>1))
		PIN_B_HIGH;
	else
		PIN_B_LOW;
}

//���� 
void strobe() 
{
	PIN_STB_LOW;
	PIN_STB_HIGH;
}

void byte_out(u8_t dat)     
{ 
	u32_t n; 
	for(n=0;n<8;n++) 
	{
		if(dat & 0x80)
			PIN_DAT_LOW;
		else
			PIN_DAT_HIGH;
		PIN_SCK_LOW;
		PIN_SCK_HIGH;
//      NOP;
//		NOP;
//		PIN_SCK_LOW;
//		NOP;
//		NOP;
		dat=dat<<1;
	}
} 

/*
P10��Ԫ��Ϊ32x16�ķֱ��ʡ�
�����Ϊ4��panel��ÿ��panel 4����߶ȣ�Panel��A��B����ָ����
��һpanel�ڣ�ʹ��STB�ź��л�ÿһ�У�һ���ں���32��CLK���ģ�ͬʱ���DAT����ÿһ�����ݡ�
*/

//������ʾ 
void update() 
{
	int i,j,k;
	u8_t buf;
	strobe();
	for(k=0;k<4;k++)              //��ʾ������ 
	{
		for(j=0;j<2;j++)         //��ʾ3��4�� 
		{
			for(i=0;i<2;i++)      // ��ʾ1��2�� 
			{
				buf = play_buf[24+(k<<1)+i+(j<<5)];
				byte_out(buf);
				buf = play_buf[16+(k<<1)+i+(j<<5)];
				byte_out(buf);
				buf = play_buf[8+(k<<1)+i+(j<<5)];
				byte_out(buf);
				buf = play_buf[(k<<1)+i+(j<<5)];
				byte_out(buf);
			}
		}
		strobe();
		HC138_scan(k);
	}
}



void point(u8_t x,u8_t y, u8_t c)
{
	u8_t i;

	if(x>=32) return;
	if(y>=16) return;
	i=0x80;
	i>>=x-((x>>3)<<3);

	if(c)
		play_buf[(y<<1)+((x>>4)<<5)+((x>>3)&0x01)] |= i;
	else 
		play_buf[(y<<1)+((x>>4)<<5)+((x>>3)&0x01)] &= ~i;
}
//����
void line(u8_t  x1, u8_t  y1, u8_t  x2, u8_t  y2, u8_t color)
{
	u8_t  x, y, t;
	if((x1 == x2) && (y1 == y2))
		point(x1, y1, color);
	else if(ABS(y2 - y1) > ABS(x2 - x1))
	{
		if(y1 > y2)
		{
			t = y1;
			y1 = y2;
			y2 = t;
			t = x1;
			x1 = x2;
			x2 = t;
		}
		for(y = y1; y <= y2; y ++)
		{
			x = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
			point(x, y, color);
		}
	}
	else
	{
		if(x1 > x2)
		{
			t = y1;
			y1 = y2;
			y2 = t;
			t = x1;
			x1 = x2;
			x2 = t;
		}
		for(x = x1; x <= x2; x ++)
		{
			y = (x - x1) * (y2 - y1) / (x2 - x1) + y1;
			point(x, y, color);
		}
	}
}

void rectangle(u8_t x1,u8_t y1,u8_t x2,u8_t y2, u8_t color)
{
	line(x1,y1,x2,y1, color);
	line(x1,y1,x1,y2, color);
	line(x1,y2,x2,y2, color);
	line(x2,y1,x2,y2, color);
}
void filled_rectangle(u8_t x1,u8_t y1,u8_t x2,u8_t y2, u8_t color)
{
	u8_t i;
	for(i=y1;i<=y2;i++)
	{
		line(x1,i,x2,i, color);
	}
}

//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void circle(u8_t x0,u8_t y0,u8_t r, u8_t color)
{
	int a,b;
	int di;
	a=0;
	b=r;
	di=3-2*r;             //�ж��¸���λ�õı�־
	while(a<=b)
	{
		point(x0-b,y0-a, color);             //3           
		point(x0+b,y0-a, color);             //0           
		point(x0-a,y0+b, color);             //1       
		point(x0-b,y0-a, color);             //7           
		point(x0-a,y0-b, color);             //2             
		point(x0+b,y0+a, color);             //4               
		point(x0+a,y0-b, color);             //5
		point(x0+a,y0+b, color);             //6 
		point(x0-b,y0+a, color);             
		a++;
		/***ʹ��Bresenham�㷨��Բ**/     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		point(x0+a,y0+b, color);
	}
}


void put_char(u8_t x, u8_t y, u8_t c, char color)
{
	u8_t tmp_char = 0, i, j;

	for (i=0; i<8; i++){

		tmp_char = ASCII8x8[c - 0x20 + i];
		for (j = 0; j < 8; j++)
		{
			if ( (tmp_char >> (7 - j)) & 0x01)
				point(x+j, y+i, color); // �ַ���ɫ
			else
				point(x+j, y+i, !color); // ������ɫ
		}
	}
}



int main(int argc, char* argv[])
{
	api_io.init(PIN_OE, OUT_PUSH_PULL);
	api_io.init(PIN_A, OUT_PUSH_PULL);
	api_io.init(PIN_B, OUT_PUSH_PULL);
	api_io.init(PIN_SCK, OUT_PUSH_PULL);
	api_io.init(PIN_STB, OUT_PUSH_PULL);
	api_io.init(PIN_DAT, OUT_PUSH_PULL);

	PIN_SCK_LOW;
	PIN_STB_LOW;
	PIN_A_LOW;
	PIN_B_LOW;
	PIN_OE_HIGH;
	PIN_DAT_LOW;


	rectangle(0,0,31,15,1);
	circle(15,8,4,1);

	while(1)
	{
		update();
	}

//	return ADDON_LOADER_GRANTED;
//err_exit:
	return ADDON_LOADER_ABORT;		//release 
}




