
/**
 * @file			ir.c
 * @brief			wifiIO�������ⷢ��ܵĲ��Գ���
 *	�ǳ��򵥵�һ�����Գ��򣬿��Լ�Ъ����38K���塣
 *	����ʹ�������ģ�������Ӧ�Ĳ��Ρ�
 *	ʵ���ϣ�ʹ��PWM���38K���Ӽ�
 * @author			yizuoshe@gmail.com
*/

#include "include.h"

#define IR_SEND_PIN WIFIIO_GPIO_01

void IR_send_38K(int circle)
{
	for(;circle > 0; circle--){
		api_io.high(IR_SEND_PIN);
		api_tim.tim6_poll(100);
		api_io.low(IR_SEND_PIN);
		api_tim.tim6_poll(163);
	}
}

int main(int argc, char* argv[])
{
	api_io.init(IR_SEND_PIN, OUT_PUSH_PULL);
	api_io.low(IR_SEND_PIN);

	api_tim.tim6_init(10000000);	//��ʼ����ʱ��6 0.1usΪ��λ

	while(1)
	{
		api_os.tick_sleep(1000);

		IR_send_38K(38000);
	}

//	return ADDON_LOADER_GRANTED;
//err_exit:
	return ADDON_LOADER_ABORT;		//release 
}


