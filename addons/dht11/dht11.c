
/**
 * @file			dht11.c
 * @brief			��ʪ�ȴ�����DHT11���Գ���
 *	����DHT11�������ṩ��ȡ��ͨ�ýӿڡ�
 *	
 * @author			yizuoshe@gmail.com
*/




#include "include.h"

/*
ģ��:

�����ʽ: 
{
	"method":"dht11.read",
	"params":{}
}
����:
{
	"result":{"humidity":xx, "temperature":xxx},
}


*/

#define DHT11_DATA	WIFIIO_GPIO_01
#define DHT11_INPUT api_io.init(DHT11_DATA, IN_PULL_UP)
#define DHT11_OUTPUT api_io.init(DHT11_DATA, OUT_OPEN_DRAIN_PULL_UP)

#define DHT11_OUT_LOW api_io.low(DHT11_DATA)
#define DHT11_OUT_HIGH api_io.high(DHT11_DATA)
#define DHT11_IN_DAT  api_io.get(DHT11_DATA)



////////////////////////////////////////
//ϸ�ڵ�ʱ������
////////////////////////////////////////




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

////////////////////////////////////////
//ί�нӿڵĶ������:
// JSON_DELEGATE() ��Ϊ������Ӻ�׺����
// __ADDON_EXPORT__ �꽫�÷��Ŷ���Ϊ��¶��
// ������ʽ �� ��������һ��json��������һ��
// ִ����ɺ�Ļص� ���丽��������
//
// �ú��������������� �ǵ��ø�ί�нӿڵĽ���
// httpd��otd �����Ե��øýӿ�
// �ӿ�����Ϊ dht11.read
//{
//	"method":"dht11.read",
//	"params":{}
//}

//{
//	"result":{"humidity":xx, "temperature":xxx},
//}
////////////////////////////////////////

int JSON_RPC(read)(jsmn_node_t* pjn, fp_json_delegate_ack ack, void* ctx)	//�̳���fp_json_delegate_start
{
	int ret = STATE_OK;
	char* err_msg = NULL;

	LOG_INFO("DELEGATE dht11.read.\r\n");


/*
{
	"method":"dht11.read",
	"params":{}
}
*/

	//----�¶ȸ�8λ== U8T_data_H------
	//----�¶ȵ�8λ== U8T_data_L------
	//----ʪ�ȸ�8λ== U8RH_data_H-----
	//----ʪ�ȵ�8λ== U8RH_data_L-----
	//----У�� 8λ == U8checkdata-----
	u8_t temp_h, temp_l, humi_h, humi_l, chk;


	if(STATE_OK != (ret = dht11_start()) ||
		STATE_OK != (ret = dht11_read_byte(&humi_h))||
		STATE_OK != (ret = dht11_read_byte(&humi_l))||
		STATE_OK != (ret = dht11_read_byte(&temp_h))||
		STATE_OK != (ret = dht11_read_byte(&temp_l))||
		STATE_OK != (ret = dht11_read_byte(&chk)))
		{}


	dht11_stop();

	if(STATE_OK != ret){
		err_msg = "read time out.";
		LOG_WARN("dht11:%s %d.\r\n", err_msg, ret);
		goto exit_err;
	}

	if(chk != temp_h+temp_l+humi_h+humi_l){
		ret = STATE_ERROR;
		err_msg = "checksum err.";
		LOG_WARN("dht11:%s.\r\n", err_msg);
		goto exit_err;
	}


	if(ack){
		char json[128];
		u32_t n = 0;
		n += _snprintf(json+n, sizeof(json)-n, "{\"result\":{\"temperature\":%u.%u,\"humidity\":%u.%u}}",temp_h, temp_l, humi_h, humi_l); 
		jsmn_node_t jn = {json, n, NULL};
		ack(&jn, ctx);	//�����json���� ��Ӧjson
	}
	return ret;


//exit_safe:
	if(ack)
		jsmn.delegate_ack_result(ack, ctx, ret);	
	return ret;

exit_err:
	if(ack)
		jsmn.delegate_ack_err(ack, ctx, ret, err_msg);
	return ret;
}




////////////////////////////////////////
//ÿһ��addon����main���ú����ڼ��غ�����
//������ �� ADDON_LOADER_GRANTED ������main�������غ�addon��ж��
////////////////////////////////////////

int main(int argc, char* argv[])
{
	DHT11_OUTPUT;	//��ʼ������
	api_tim.tim6_init(100000);	//��ʼ����ʱ��6 10usΪ��λ

	LOG_INFO("main: DHT11 starting...\r\n");

	return ADDON_LOADER_GRANTED;	//����addon��ram��
//err_exit:
//	return ADDON_LOADER_ABORT;
}



