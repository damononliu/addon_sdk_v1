
/**
 * @file			ireplay.c
 * @brief			�����طţ�����ʾ����Ӳ����ʱ����ʱ�ķ�ʽ������������ʹ���ն�(uart1)�鿴���
 *					���⣬�طŹ���ʹ��ӲPWM���38K�ز���ͨ�Ϸ�ʽ���ң�ر���
 *
 *	
 * @author			dy@wifi.io
*/




#include "include.h"


#define IR_INPUT	WIFIIO_GPIO_01
#define IR_OUTPUT	WIFIIO_GPIO_03
#define IR_OUTPUT_PWM WIFIIO_PWM_09CH1

#define IR_SET_INPUT api_io.init(IR_INPUT, IN_PULL_UP)
#define IR_SET_OUTPUT api_io.init(IR_OUTPUT, OUT_OPEN_DRAIN_PULL_UP)

#define IR_IN_DAT  api_io.get(IR_INPUT)


#define IR_OUT_LOW api_pwm.t_set( IR_OUTPUT_PWM, 120)
#define IR_OUT_HIGH api_pwm.t_set( IR_OUTPUT_PWM, 0)





u16_t ir_high_elapsed(void)
{
	api_tim.tim6_start(65535);
	while(IR_IN_DAT && !api_tim.tim6_is_timeout()){};
	return api_tim.tim6_count();
}

u16_t ir_low_elapsed(void)
{
	api_tim.tim6_start(65535);
	while(!IR_IN_DAT && !api_tim.tim6_is_timeout()){};
	return api_tim.tim6_count();
}

void ir_replay(u16_t ts[], size_t ts_len)
{
	int i;
	IR_OUT_HIGH;

	for(i = 0; i < ts_len; i++){
		if(i%2) IR_OUT_HIGH;
		else IR_OUT_LOW;

		api_tim.tim6_poll(ts[i]);
	}


	IR_OUT_HIGH;
}

////////////////////////////////////////
//ÿһ��addon����main���ú����ڼ��غ�����
//������ �� ADDON_LOADER_GRANTED ������main�������غ�addon��ж��
////////////////////////////////////////

int main(int argc, char* argv[])
{
	int i;
	u16_t ts[1024];
	size_t ts_len = 0;

	IR_SET_INPUT;	//��ʼ������
	api_tim.tim6_init(1000000);	//��ʼ����ʱ��6 1usΪ��λ
//	api_pwm.init(IR_OUTPUT_PWM, 6, 263, 130);	//0.1us 8/26.3us 
	api_pwm.init(IR_OUTPUT_PWM, 6, 462, 150);	//������ pwm���38K Ӧ����26.3us ���� ʵ�ʲ��� ��� 46.2��������һЩ��ʹ���߿��ԶԱȲ���һ�¡���ʾ���������� Ҳ���Բ���һ��

	api_pwm.start(IR_OUTPUT_PWM);
	IR_OUT_HIGH;


	LOG_INFO("IREPLAY: record starting...\r\n");

	while(1){
		u16_t t = 0;
		ts_len = 0;

		//һֱ�ȴ��͵�ƽ
		while(IR_IN_DAT){};

/*
		//�ȵȴ�������
		if((t = ir_low_elapsed()) < 1000)	//��Ч�͵�ƽС�� 4500us
			continue;	//�ٵ�
		else if(t > 10000)	//������������ ��Ч�͵�ƽ���� 10ms
			continue;	//�ٵ�
*/

		t = ir_low_elapsed();
		ts[ts_len++] = t;

/*
		if((t = ir_high_elapsed()) < 1000)	//��Ч�͵�ƽС�� 4000us
			continue;	//�ٵ�
		else if(t > 10000)	//������������ ��Ч�͵�ƽ���� 10ms
			continue;	//�ٵ�
*/
		t = ir_high_elapsed();


		//api_console.printf();

	//��ȡ����
		while(1){
			ts[ts_len++] = t;
			if((t = ir_low_elapsed()) > 2000)	//��Ч��ƽС�� 1ms
				break;
			else if(t < 100)
				break;

			ts[ts_len++] = t;
			if((t = ir_high_elapsed()) > 2000)	//��Ч��ƽС�� 1ms
				break;
			else if(t < 100)
				break;

			if(ts_len > NELEMENTS(ts)-10)	//��������
				break;
		}


		if(ts_len < 10)continue;	//̫����

		api_console.printf("\r\n");

		for(i = 0; i < ts_len; i++)
			api_console.printf("%d,", i%2?((int)ts[i]):-((int)ts[i]));

		api_console.printf("\r\n");

		_tick_sleep(5000);


		for(i = 0; i < 10; i++){
			ir_replay(ts, ts_len);
			_tick_sleep(20);
		}


		api_console.printf("\r\n");
	}

//	return ADDON_LOADER_GRANTED;	//����addon��ram��
//err_exit:
	return ADDON_LOADER_ABORT;
}




