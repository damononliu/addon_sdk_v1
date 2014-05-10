
/**
 * @file			helloworld.c
 * @brief			wifiIO���Գ���-helloworld
 *	�����������ʾ����ܹ� ����Ӧ�ð�������LED�ƣ�ͨ������ ���� Զ��ʵ�֡�
 *	��յLED��ʵ���Ͼ����ӵ�IO1��IO2���ṩ���û�����Ϊ�������е�״ָ̬ʾ��
 * @author			yizuoshe@gmail.com
*/



#include "include.h"



/*
{
	"method":"hello.onoff",
	"params":"on"/"off"
}
*/
//�ǳ����͵�һ����ί�нӿڡ����������Ա��ⲿ����ͨ�� json rpc ��ʽ����

int JSON_RPC(onoff)(jsmn_node_t* pjn, fp_json_delegate_ack ack, void* ctx)	

//__ADDON_EXPORT__ ��gcc�ı������� ��Ӹú� ʹ���������Ա���������addon����
// JSON_DELEGATE �����Ϊ���������һ�� ��׺�� onoff -> onoff_JsDlgt ʵ�ʱ���¶�ĺ�������

{
	int ret = STATE_OK;
	char* err_msg = NULL;
	char onoff_msg[32];

	//�����json����ȡ param �ַ���
	if(JSMN_STRING != pjn->tkn->type || 
	     STATE_OK != jsmn.tkn2val_str(pjn->js, pjn->tkn, onoff_msg, sizeof(onoff_msg), NULL)){	//pjn->tkn ָ��json��params�ֶε���ֵ�ԣ�ʹ��tkn2val_str()������ȡstring���͵�value

		ret = STATE_ERROR;	
		err_msg = "Params error."; 
		LOG_WARN("mirror:%s.\r\n", err_msg);
		goto exit_err;
	}


	//������ȷ�ĵõ����ַ����������� onoff_msg ��
	if(0 == _strcmp("on", onoff_msg)){
		api_io.low(WIFIIO_GPIO_01);
		api_io.low(WIFIIO_GPIO_02);
	}

	else if(0 == _strcmp("off", onoff_msg)){
		api_io.high(WIFIIO_GPIO_01);
		api_io.high(WIFIIO_GPIO_02);
	}


	if(ack)jsmn.delegate_ack_result(ack, ctx, ret);
	return ret;

exit_err:
	if(ack)jsmn.delegate_ack_err(ack, ctx, ret, err_msg);
	return ret;
}





int main(int argc, char* argv[])
{
	int n;
	api_io.init(WIFIIO_GPIO_01, OUT_PUSH_PULL);
	api_io.init(WIFIIO_GPIO_02, OUT_PUSH_PULL);
	api_io.low(WIFIIO_GPIO_01);
	api_io.high(WIFIIO_GPIO_02);

	n = 0;
	while(n++ < 10)
	{
		api_os.tick_sleep(1000);

		api_io.toggle(WIFIIO_GPIO_01);
		api_io.toggle(WIFIIO_GPIO_02);

	}

	return ADDON_LOADER_GRANTED;

//err_exit:
	return ADDON_LOADER_ABORT;		//release 
}


