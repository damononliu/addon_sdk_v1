/**
 * @file			helloRGB.c
 * @brief			wifiIO���Գ���-����ȫ��LED��
 *	����RGB LED��ʵ��ȫ�ʵƵĿ��ơ�ȫ��LED�ǲ���3·PWM��ʵ��RGB��ԭɫ�ġ�
 *	������ģ����������PWM���ܵ�IO���ţ����嶨��������еĺ궨�塣
 * @author			yizuoshe@gmail.com
*/

#include "include.h"


#define RGB_R WIFIIO_PWM_05CH2
#define RGB_G WIFIIO_PWM_09CH1
#define RGB_B WIFIIO_PWM_09CH2

/*
{
	"method":"RGB.pwm",
	"params":{"r":16, "g":12, "b":99}
}
*/
//�ǳ����͵�һ����ί�нӿڡ����������Ա��ⲿ����ͨ�� json rpc ��ʽ����

int JSON_RPC(pwm)(jsmn_node_t* pjn, fp_json_delegate_ack ack, void* ctx)	

//__ADDON_EXPORT__ ��gcc�ı������� ��Ӹú� ʹ���������Ա���������addon����
// JSON_DELEGATE �����Ϊ���������һ�� ��׺�� onoff -> onoff_JsDlgt ʵ�ʱ���¶�ĺ�������

{
	int ret = STATE_OK;
	char* err_msg = NULL;
	u8_t r,g,b;

	//�����json����ȡ param �ַ���
	if(STATE_OK != jsmn.key2val_u8(pjn->js, pjn->tkn, "r", &r) ||
	    STATE_OK != jsmn.key2val_u8(pjn->js, pjn->tkn, "g", &g) ||
	    STATE_OK != jsmn.key2val_u8(pjn->js, pjn->tkn, "b", &b) ){
		
	
		ret = STATE_ERROR;	
		err_msg = "Params error."; 
		LOG_WARN("mirror:%s.\r\n", err_msg);
		goto exit_err;
	}


	api_pwm.t_set(RGB_R, r);
	api_pwm.t_set(RGB_G, g);
	api_pwm.t_set(RGB_B, b);



	if(ack)jsmn.delegate_ack_result(ack, ctx, ret);
	return ret;

exit_err:
	if(ack)jsmn.delegate_ack_err(ack, ctx, ret, err_msg);
	return ret;
}





int main(int argc, char* argv[])
{
 	 api_pwm.init(RGB_R, 600, 100, 0);
	 api_pwm.init(RGB_G, 600, 100, 0);
	 api_pwm.init(RGB_B, 600, 100, 0);

 	 api_pwm.start(RGB_R);
	 api_pwm.start(RGB_G);
	 api_pwm.start(RGB_B);



	return ADDON_LOADER_GRANTED;

//err_exit:
	return ADDON_LOADER_ABORT;		//release 
}


