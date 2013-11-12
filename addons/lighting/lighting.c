
/**
 * @file			lighting.c
 * @brief			wifiIO����led�ĳ���
 *	
 *	
 * @author			dy@wifi.io
*/





#include "include.h"

#define BRIGHTNESS(lv) (0.11f*math.pow(1.07f,lv))	//google "y = 0.11*1.07^x"

#define RGB1_Z WIFIIO_PWM_05CH1	//IO1
#define RGB1_R WIFIIO_PWM_05CH2	//IO2
#define RGB1_G WIFIIO_PWM_09CH1	//IO3
#define RGB1_B WIFIIO_PWM_09CH2	//IO4

#define RGB2_Z WIFIIO_PWM_04CH1	//IO17
#define RGB2_R WIFIIO_PWM_04CH2	//IO18
#define RGB2_G WIFIIO_PWM_12CH1	//IO19
#define RGB2_B WIFIIO_PWM_12CH2	//IO20


#define LT01 WIFIIO_GPIO_01
#define LT02 WIFIIO_GPIO_02
#define LT03 WIFIIO_GPIO_03
#define LT04 WIFIIO_GPIO_04

#define LT05 WIFIIO_GPIO_17
#define LT06 WIFIIO_GPIO_18
#define LT07 WIFIIO_GPIO_19
#define LT08 WIFIIO_GPIO_20


char id[16];



typedef struct {
	u32_t delay;	//��ʱ(��)

	wifiIO_pwm_t pwm_dev;
	u8_t Vfrom;	//��ʼֵ
	u8_t Vdest;	//Ŀ��ֵ
	u16_t halfcycle;	//������ ��
	u32_t hc_num;	//����������



	//������صĲ���
	u8_t iter_Vhigh;	//��ֵ
	u8_t iter_Vlow;	//Сֵ
	float iter_Vdelta;	//�仯ֵ
	float iter_Vcurr;	//��ǰֵ
	u32_t iter_change_num;	//�仯��������(100ms)
	u32_t iter_delay_num;	//�Ⱥ��������(100ms)
}pwm_dev_t;

pwm_dev_t dev[8] = {
		{0, WIFIIO_PWM_05CH1, 0, 0, 0, 0, 0, 0},
		{0, WIFIIO_PWM_05CH2, 0, 0, 0, 0, 0, 0},
		{0, WIFIIO_PWM_09CH1, 0, 0, 0, 0, 0, 0},
		{0, WIFIIO_PWM_09CH2, 0, 0, 0, 0, 0, 0},
		{0, WIFIIO_PWM_04CH1, 0, 0, 0, 0, 0, 0},
		{0, WIFIIO_PWM_04CH2, 0, 0, 0, 0, 0, 0},
		{0, WIFIIO_PWM_12CH1, 0, 0, 0, 0, 0, 0},
		{0, WIFIIO_PWM_12CH2, 0, 0, 0, 0, 0, 0},
};

//�������ü�������仯(������ iter_ ��ͷ�ĳ�Ա��ֵ)
void dev_iterate_calculate(pwm_dev_t* dev)
{
	u32_t n;
	float sign = 1.0f;
	//���� from �� dest ��Ҫ��һ�������򲻼���
	if(dev->Vfrom > dev->Vdest){
		dev->iter_Vhigh = (float)dev->Vfrom;
		dev->iter_Vlow = (float)dev->Vdest;
		sign = -1.0f;
	}
	else if(dev->Vfrom < dev->Vdest){
		dev->iter_Vhigh = (float)dev->Vdest;
		dev->iter_Vlow = (float)dev->Vfrom;
		sign = 1.0f;
	}
	else
		return;

//	if(dev->halfcycle == 0)
//		dev->halfcycle = 1;

	dev->iter_delay_num = dev->delay *10;
	n = dev->halfcycle * 10;		//�������������������������100ms
	if(n == 0)	//����������仯 ��ô���ٰ�������100ms
		n = 1;

	dev->iter_Vdelta = sign * (dev->iter_Vhigh - dev->iter_Vlow)/(float)n;
	dev->iter_change_num = n*dev->hc_num;
	dev->iter_Vcurr = 1.0f*dev->Vfrom;




}


void dev_iterate(pwm_dev_t* dev)
{
	if(dev->iter_delay_num > 0){	//��ʱ�׶�
		dev->iter_delay_num--;
		return;
	}

/*
	{
	//	double tmp = 0.0f;
		int i;
		for(i = 0; i < 100; i++){
			dev->iter_Vcurr += dev->iter_Vdelta;
			api_console.printf("tmp:%d \r\n", (int)dev->iter_Vcurr);
		}

	}

*/

	if(dev->iter_change_num > 0){	//������е������� 
		u8_t v;

		dev->iter_Vcurr += dev->iter_Vdelta;		//���ݵ�ǰֵ������һ��ֵ

		if(dev->iter_Vcurr > dev->iter_Vhigh || dev->iter_Vcurr < dev->iter_Vlow)
			dev->iter_Vdelta = -dev->iter_Vdelta;	//delta��ת

		v = (u8_t)dev->iter_Vcurr;
		if(v > dev->iter_Vhigh)		v = dev->iter_Vhigh;
		if(v < dev->iter_Vlow)		v = dev->iter_Vlow;

		api_pwm.t_set(dev->pwm_dev, BRIGHTNESS(v));
		dev->Vfrom = v;	//���µ�ǰֵ

		dev->iter_change_num--;

	}
}







/*
{
	"method":"lighting.schedule",
	"params":[
		[val, delay, halfcycle_period, halfcycle_num, 1],
		...
		[val, delay, halfcycle_period, halfcycle_num, 2, 3],
	]
}
val:	0~99
delay:	unsigned 32bits	��ǰ��ʱ�������� 2592000 max (1 month)
period:	������ʱ��(s) 255max
num:	��������������������״̬��ת��ż�����ջع�ԭ״̬��1024 max
����Ǳ�ţ�8��PWMͨ����ţ������������ָ��

*/
int JSON_RPC(schedule)(jsmn_node_t* pjn, fp_json_delegate_ack ack, void* ctx)	
{
	int ret = STATE_OK;
	char* err_msg = NULL;

//	LOG_INFO("DELEGATE lighting.schedule.\r\n");

	jsmntok_t* jt = pjn->tkn;	//ָ������������[[],..[]]

	if(JSMN_ARRAY != jt->type){
		ret = STATE_ERROR;
		err_msg = "Cmd array needed.";
		LOG_WARN("lighting:%s.\r\n", err_msg);
		goto exit_err;
	}

	int cmd_num = jt->size;
	LOG_INFO("%u cmds.\r\n", cmd_num);

	jt += 1; 	//skip [
	while(cmd_num--){
		int dev_num = 0;
		jsmntok_t* j = jt;
		u8_t val;
		u16_t halfcycle;
		u32_t hc_num;
		u32_t delay;
	
		if(JSMN_ARRAY != j->type || j->size <= 4){
			ret = STATE_PARAM;
			err_msg = "Cmd invalid.";
			LOG_WARN("lighting:%s. %u %u\r\n", err_msg, j->type, j->size);
			goto exit_err;
		}
		dev_num = j->size - 4;	//�������鵥Ԫ���� �õ�pwm���Ƶ�Ԫ������
		j += 1;	//skip [


		jsmn.tkn2val_u8(pjn->js, j, &val);	//��1����Ԫ�� val
		j++;
		jsmn.tkn2val_uint(pjn->js, j, &delay);	//��2����Ԫ�� delay
		j++;
		jsmn.tkn2val_u16(pjn->js, j, &halfcycle);	//��3����Ԫ�� halfcycle
		j++;
		jsmn.tkn2val_uint(pjn->js, j, &hc_num);	//��4����Ԫ�� hc_num
		j++;


		//�����ÿ��pwm��������
		while(dev_num--){
			u8_t dev_id;
			jsmn.tkn2val_u8(pjn->js, j, &dev_id);
			dev_id -= 1;

			dev[dev_id].delay = delay;
			dev[dev_id].Vdest = val;
			dev[dev_id].hc_num = hc_num;
			dev[dev_id].halfcycle = halfcycle;

			LOG_INFO("delay:%u val:%u hc:%u hcn:%u Vnow:%u\r\n", delay, val, halfcycle, hc_num, dev[dev_id].Vfrom);
			dev_iterate_calculate(&dev[dev_id]);
			j++;
		}

		jt = jsmn.next(jt);	//��Ϊÿ����λ����primitive������˲���ʹ��+1���л�
	}
	

//exit_safe:
	if(ack)
		jsmn.delegate_ack_result(ack, ctx, ret);
	return ret;

exit_err:
	if(ack)
		jsmn.delegate_ack_err(ack, ctx, ret, err_msg);
	return ret;


	
}





int main(int argc, char* argv[])
{
	int i;
 	 api_pwm.init(WIFIIO_PWM_05CH1, 600, 100, 0);
 	 api_pwm.init(WIFIIO_PWM_05CH2, 600, 100, 0);
 	 api_pwm.init(WIFIIO_PWM_09CH1, 600, 100, 0);
 	 api_pwm.init(WIFIIO_PWM_09CH2, 600, 100, 0);
 	 api_pwm.init(WIFIIO_PWM_04CH1, 600, 100, 0);
 	 api_pwm.init(WIFIIO_PWM_04CH2, 600, 100, 0);
 	 api_pwm.init(WIFIIO_PWM_12CH1, 600, 100, 0);
 	 api_pwm.init(WIFIIO_PWM_12CH2, 600, 100, 0);


	 api_pwm.start(WIFIIO_PWM_05CH1);
	 api_pwm.start(WIFIIO_PWM_05CH2);
	 api_pwm.start(WIFIIO_PWM_09CH1);
	 api_pwm.start(WIFIIO_PWM_09CH2);
	 api_pwm.start(WIFIIO_PWM_04CH1);
	 api_pwm.start(WIFIIO_PWM_04CH2);
	 api_pwm.start(WIFIIO_PWM_12CH1);
	 api_pwm.start(WIFIIO_PWM_12CH2);



/*
	char* js_buf = NULL;
	size_t js_len;
	jsmntok_t tk[16];


	//��������json�ļ�	2048bytes max
	js_buf = api_fs.read_alloc("/app/lighting/cfg.json", 2048, &js_len, 0);
	if(NULL == js_buf){
		goto load_abort;
	}


	//parse json
	do{
		jsmn_parser psr;
		jsmn.init(&psr);
		ret = jsmn.parse(&psr, js_buf, js_len, tk, NELEMENTS(tk));
		if(JSMN_SUCCESS != ret){
			LOG_WARN("parse err(%d).\r\n", ret);
			goto load_abort;
		}
		tk[psr.toknext].type= JSMN_INVALID;	//��ʶβ��
	}while(0);


	//id
	if(STATE_OK != jsmn.key2val_str(js_buf, tk, "id",  id, sizeof(id), NULL)){
		LOG_INFO("Can not get ID, abort.\r\n");
		goto load_abort;
	}

	_free(js_buf);
	js_buf = NULL;


	if(0 == _memcmp(id+3, "A01", 3)){
		thread_a01();
	}
	else if(0 == _memcmp(id+3, "A02", 3)){
		thread_a02();
	}
	else if(0 == _memcmp(id+3, "A03", 3)){
		thread_a03();
	}
	else if(0 == _memcmp(id+3, "A04", 3)){
		thread_a04();
	}
	else if(0 == _memcmp(id+3, "A05", 3)){
		thread_a05();
	}

*/


	while(1){
		for(i = 0; i < NELEMENTS(dev); i++)
			dev_iterate(&dev[i]);

		api_os.tick_sleep(100);	//100ms
	}


	return ADDON_LOADER_ABORT;

/*
load_abort:
	LOG_INFO("cfg err: aborting...\r\n");


	if(NULL != js_buf)
		_free(js_buf);
	return ADDON_LOADER_ABORT;
*/
}






//httpd�ӿ�  ��� json
/*
GET http://192.168.1.105/logic/wifiIO/invoke?target=lighting.status

{"state":[12,34,45,0,99...]}

*/


int __ADDON_EXPORT__ JSON_FACTORY(status)(char*arg, int len, fp_consumer_generic consumer, void *ctx)
{
	char buf[128];
	int n, size = sizeof(buf);

	n = 0;
	n += utl.snprintf(buf + n, size-n, "{\"state\":[%u,%u,%u,%u,%u,%u,%u,%u]}", dev[0].Vfrom, dev[1].Vfrom, dev[2].Vfrom, dev[3].Vfrom, dev[4].Vfrom, dev[5].Vfrom, dev[6].Vfrom, dev[7].Vfrom);
	return  consumer(ctx, (u8_t*)buf, n);
}


