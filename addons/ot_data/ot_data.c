/**
 * @file			ot_data.c
 * @brief			wifiIO��ȡ��ʪ�ȴ�����DHT11�ϴ��ƶ�
 *	
 *	
 * @author			yizuoshe@gmail.com
*/

#include "include.h"

#define DHT11_PIN WIFIIO_GPIO_16

#define DHT11_INPUT api_io.init(DHT11_PIN, IN_PULL_UP)
#define DHT11_OUTPUT api_io.init(DHT11_PIN, OUT_OPEN_DRAIN_PULL_UP)

#define DHT11_OUT_LOW api_io.low(DHT11_PIN)
#define DHT11_OUT_HIGH api_io.high(DHT11_PIN)
#define DHT11_IN_DAT  api_io.get(DHT11_PIN)

//����18ms ����40us �Ⱥ�dht11���� ��������ʱ��
int dht11_start(void)
{
	DHT11_OUT_LOW;
	api_tim.tim6_poll(2000);		//drive low for 2000x10us > 18ms
	DHT11_OUT_HIGH;
	api_tim.tim6_poll(4);		//40us
	DHT11_INPUT;
	if(0 != DHT11_IN_DAT)return -100;		//dht11 should resp low


	api_tim.tim6_start(10);		//start cnt to 100us, DHT11 keep low 80us max
	while(!DHT11_IN_DAT && !api_tim.tim6_is_timeout());	//wait to high
	if(api_tim.tim6_is_timeout())return -101;


	api_tim.tim6_start(10);		//start cnt to 100us, DHT11 keep high 80us max
	while(DHT11_IN_DAT && !api_tim.tim6_is_timeout());	//wait to low
	if(api_tim.tim6_is_timeout())return -102;

	return STATE_OK;
}


int dht11_read_byte(u8_t* dat)
{
	int i;
	u8_t d = 0;
	for(i = 0; i < 8; i++){
		d <<= 1;

		api_tim.tim6_start(8);		//start cnt to 80us, DHT11 keep low 50us max
		while(!DHT11_IN_DAT && !api_tim.tim6_is_timeout());	//wait to high
		if(api_tim.tim6_is_timeout())return -103;

		api_tim.tim6_start(10);
		while(DHT11_IN_DAT && !api_tim.tim6_is_timeout());	//wait to low
		if(api_tim.tim6_is_timeout())return -104;

		//0:high 28us; 1: high 70us
		if(api_tim.tim6_count() < 5){}	//high 26~28us means 0
		else	d += 1;	//high 70us means 1
	}

	if(dat)*dat = d;
	return STATE_OK;
}

void dht11_stop(void)
{
	DHT11_OUTPUT;
	DHT11_OUT_HIGH;
}



/*

int sms(void)
{
	char json[256];
	u32_t n = 0;
	n += _snprintf(json+n, sizeof(json)-n, 
		"{\"method\":\"forward\",
		  \"params\":{
				\"url\":\"http://api.wifi.io/sms/send_sms.php\",
				\"content\":{\"tel\":13671056700,\"msg\":\"��������ɧ��һ��~��\"}
			}
		 }"); 
	api_ot.notify(json, n);
	return STATE_OK;
}
*/


int main(int argc, char* argv[])
{
	int ret = STATE_OK;
	u8_t temp_h, temp_l, humi_h, humi_l, chk, humi_last = 0, temp_last = 0;
	LOG_INFO("DHT11:Service starting...\r\n");

	//sms();

	api_io.init(WIFIIO_GPIO_01, OUT_PUSH_PULL);	//��ʼ������ ����ָʾ���ݶ�ȡ
	api_io.init(WIFIIO_GPIO_02, OUT_PUSH_PULL);
	api_io.high(WIFIIO_GPIO_01);
	api_io.high(WIFIIO_GPIO_02);


	DHT11_OUTPUT;	//��ʼ������
	api_tim.tim6_init(100000);	//��ʼ����ʱ��6 10usΪ��λ



	while(1){
		api_os.tick_sleep(1000);

		if(STATE_OK != (ret = dht11_start()) ||
			STATE_OK != (ret = dht11_read_byte(&humi_h))||
			STATE_OK != (ret = dht11_read_byte(&humi_l))||
			STATE_OK != (ret = dht11_read_byte(&temp_h))||
			STATE_OK != (ret = dht11_read_byte(&temp_l))||
			STATE_OK != (ret = dht11_read_byte(&chk)))
			{}
		dht11_stop();

		if(STATE_OK != ret){
			LOG_WARN("dht11:%s %d.\r\n", "read time out.", ret);			
			continue;
		}

		if(humi_h != humi_last){
			char json[256];
			u32_t n = 0;
			n += _snprintf(json+n, sizeof(json)-n, "{\"method\":\"data\",\"params\":{\"value\":%u,\"tags\":[\"humidity\"]}}", humi_h); 	//���Գ�������µ�tag
			api_ot.notify(json, n);
			api_io.toggle(WIFIIO_GPIO_01);	//ʪ�ȱ仯 IO01
			humi_last = humi_h;
			LOG_INFO("ot_data: humidity update %u.\r\n", humi_last);
		}

		if(temp_h != temp_last){
			char json[256];
			u32_t n = 0;
			n += _snprintf(json+n, sizeof(json)-n, "{\"method\":\"data\",\"params\":{\"value\":%u,\"tags\":[\"temperature\"]}}", temp_h);  	//���Գ�������µ�tag
			api_ot.notify(json, n);
			api_io.toggle(WIFIIO_GPIO_02);	//�¶ȱ仯 IO02
			temp_last =  temp_h;
			LOG_INFO("ot_data: temperature update %u.\r\n", temp_last);
		}

	}

//	return ADDON_LOADER_GRANTED;
//err_exit:
	return ADDON_LOADER_ABORT;
}






