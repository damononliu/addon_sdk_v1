
/**
 * @file			app_stcpc.c
 * @brief			����תtcp�ͻ���
 *	
 *	
 * @author			yizuoshe@gmail.com
*/


#include "include.h"



#define STCPC_LOADMAX_SIZE_MAX 1460

#define DOMAIN_SIZE_MAX 64


typedef struct stcpc_opt{

	char domain[DOMAIN_SIZE_MAX];
	u16_t peer_port;
	net_addr_t peer_ip;
	u8_t pkt_buf[STCPC_LOADMAX_SIZE_MAX];
	//u16_t pkt_buf_len;

	u16_t hold_time;
	nb_timer_t *tmr;

	serial_cfg_t serial_cfg;

	u32_t statistic_up_serial_bytes;
	u32_t statistic_down_serial_bytes;

}stcpc_opt_t;



static int  tcpc_timer_resolve_connect(tcpc_info_t* tcpc, void* ctx)
{
	int ret = STATE_OK;
	stcpc_opt_t* stcpc;
	stcpc = (stcpc_opt_t*)api_tcpc.info_opt(tcpc);

	//�����ǰ��û�н����������̣���ô����
	if(TCPC_STATE_IDLE == api_tcpc.state(tcpc)){

		//���ipΪ0������domain�ĵ�ַ
		if(stcpc->peer_ip.s_addr == 0){
			ret = api_net.name2ip(stcpc->domain, &(stcpc->peer_ip));

			if(STATE_OK != ret){
				LOG_WARN("stcpc: Domain %s resolve failed.\r\n", stcpc->domain);
				goto retry;
			}
			LOG_INFO("stcpc: Domain %s resolved to %s.\r\n", stcpc->domain, api_net.ntoa(&stcpc->peer_ip));
		}


		//��������
		api_tcpc.req_conn(tcpc, stcpc->peer_ip.s_addr, stcpc->peer_port);
		LOG_INFO("stcpc: Try connecting to %s.\r\n", api_net.ntoa(&stcpc->peer_ip));


		//�������Ӻ�����׼����������Ϊ ϵͳ�������ڿ���netif��û��up����һ��try����û��Ч��
		//����� ״̬Ϊ TCPC_STATE_CONNECTING����ô�Ͳ�����try�ˣ�ֱ�ӵȴ�CONN_OK ���� CONN_ERR �¼�����
		retry:
			return 3000;	// 3�������

	}
	
	//����Ѿ�������������
	else
		//���ﷵ��0 ��ʶ���ٽ�����ʱ
		return 0;

}



//hold time��ʱ���
static int stcpc_timer_serial_check(nb_info_t* pnb, nb_timer_t* ptmr, void* ctx)
{
	int slen = 0;
	tcpc_info_t* tcpc = (tcpc_info_t*)pnb;
	stcpc_opt_t* stcpc = (stcpc_opt_t*)api_tcpc.info_opt(tcpc);

	//��鴮������
	if(0 == (slen = api_serial.peek(SERIAL2))){
		//stcpc->last_peek = 0;	//���
		goto TIMER_EXIT_NULL;
	}

	//û�����ӣ������������ݣ����ҷ���֮��Ķ�ʱ���
	if(TCPC_STATE_CONNECTED != api_tcpc.state(tcpc)){
		api_serial.clean(SERIAL2);
		//stcpc->last_peek = 0;	//���
		goto TIMER_EXIT_STOP;
	}

	//������ϴβ�һ������ô˵����holdtimeʱ����������ݽ��룬������
	//if(stcpc->last_peek != slen){
	//	stcpc->last_peek = slen;
	//	goto TIMER_EXIT_SKIP;
	//}

	//����������֮
	do{
		int send_len = MIN(slen, sizeof(stcpc->pkt_buf));

		//����
		api_serial.read(SERIAL2, stcpc->pkt_buf, send_len);

		//����
		api_tcpc.send(tcpc, stcpc->pkt_buf, send_len);
		LOG_INFO("stc(spi no %u).\r\n", send_len);

		slen -= send_len;

		stcpc->statistic_up_serial_bytes += send_len;
	}while(slen);



	//stcpc->last_peek = slen;

//TIMER_EXIT_SKIP:
TIMER_EXIT_NULL:

	//1С�ģ�
	//2����һ���ص�����������ֵ��ʶ�Ƿ������ʱ��
	//2�������Ҫ�ٶ���ʱ��ֱ�ӷ���0����

	//���°�װ
	return stcpc->hold_time;

TIMER_EXIT_STOP:
	return 0;

}






/*
//���ڽ�����ϢԴ
static int stcpc_serial_event_src(void* sdev, void*ctx, int len)
{
	int ret = STATE_OK;
	tcpc_info_t * tcpc =  (tcpc_info_t*)ctx;
	stcpc_opt_t* stcpc = (stcpc_opt_t*)api_tcpc.info_opt(tcpc);
	nb_msg_t* msg = stcpc->serial_rx_msg;

	add_stcpc_msg_t *stcpc_msg = api_nb.msg_opt(msg);
	stcpc_msg->len = len;


	ret = api_nb.msg_send((nb_info_t *)tcpc, msg, TRUE);
	//ARCH_ASSERT("",ret == STATE_OK);

	return ret;
}
*/



//��û������peer_ip����������
static int tcpc_stcpc_cb_enter(tcpc_info_t* tcpc)
{
	stcpc_opt_t* stcpc = api_tcpc.info_opt(tcpc);
	//�ܿ����ؽű�
	//api_os.tick_sleep(1000);


	//��������
	api_serial.open(SERIAL2);
	api_serial.ctrl(SERIAL2, SERIAL_CTRL_SETUP, &(stcpc->serial_cfg));	//����


	//�������� ����Ŀ�꣬��nb��ʼ��ת��ִ��
	api_nb.timer_attach((nb_info_t*)tcpc, 0, (fp_nb_timer_handler)tcpc_timer_resolve_connect, NULL, 0);


	return STATE_OK;

}




static int tcpc_stcpc_cb_exit(tcpc_info_t* tcpc)
{

	//stcpc_opt_t* stcpc = api_tcpc.info_opt(tcpc);

	api_serial.close(SERIAL2);

	//if(stcpc->serial_rx_msg)
	//	nb_msg_free(stcpc->serial_rx_msg);

	//api_nb.hdlr_unset((nb_info_t*)tcpc, MSG_TYPE_STCPC_SERIAL);

	return STATE_OK;
}



static int tcpc_stcpc_cb_recv(tcpc_info_t* tcpc)
{
	int ret = STATE_OK;
	net_pkt_t* pkt;
	stcpc_opt_t* stcpc = api_tcpc.info_opt(tcpc);

	ret = api_tcpc.recv(tcpc, &pkt);
	if(ret != STATE_OK){
		LOG_WARN("stcpc: recv err(%d).\r\n",ret);
		return ret;
	}

	do{
//		net_addr_t  *paddr;
//		u16_t peer_port;

		u16_t pkt_len = 0;
		u8_t *pkt_ptr = NULL; 
		if(STATE_OK != api_net.pkt_data(pkt, (void**)&pkt_ptr, &pkt_len) || pkt_len == 0)//
			break;

//		paddr = api_net.pkt_faddr(pkt);
//		peer_port = api_net.pkt_fport(pkt);

		if(pkt_len != 0){
			int rlen = pkt_len;
			//���ڷ���
			do{
				int n;
				n = api_serial.write(SERIAL2,  (const void*)pkt_ptr, rlen);
				rlen -= n; pkt_ptr += n;
				if(rlen)api_os.tick_sleep(1);
			}while(rlen);

			stcpc->statistic_down_serial_bytes += pkt_len;

			LOG_INFO("stc(ni so %u).\r\n", pkt_len);
		}
	}while(api_net.pkt_next(pkt) >= 0);

	api_net.pkt_free(pkt);

	return ret;
}


//���ӳɹ�
int tcpc_stcpc_cb_conn_ok(tcpc_info_t* tcpc)
{
	int ret = STATE_OK;
	stcpc_opt_t* stcpc = api_tcpc.info_opt(tcpc);

	LOG_INFO("[stcpc]:Connected to [%s:%u].\r\n", api_net.ntoa(&(stcpc->peer_ip)), stcpc->peer_port);


	//���ʱ����ֵ�����ö��Ҵ���0����װ���ڼ�ⶨʱ��
	if(stcpc->hold_time > 0){
		stcpc->tmr = api_nb.timer_attach((nb_info_t*)tcpc, stcpc->hold_time, stcpc_timer_serial_check, NULL, 0);
		ARCH_ASSERT("",NULL != stcpc->tmr);
	}

	return ret;
}


//����ʧ��
int tcpc_stcpc_cb_conn_err(tcpc_info_t* tcpc)
{
	int ret = STATE_OK;
	stcpc_opt_t* stcpc = api_tcpc.info_opt(tcpc);

	LOG_INFO("[stcpc]:Connect failed.\r\n");

	//If gives domain, may be IP changes. Set IP to 0, this will cause new DNS resolve.
	if(stcpc->domain[0] != '\0')
		 stcpc->peer_ip.s_addr = 0;

	// 3��������������ӹ���
	api_nb.timer_attach((nb_info_t*)tcpc, 3000, (fp_nb_timer_handler)tcpc_timer_resolve_connect, NULL, 0);

	return ret;
}

//���ӶϿ�
int tcpc_stcpc_cb_disconn(tcpc_info_t* tcpc)
{
	//stcpc_opt_t* stcpc = api_tcpc.info_opt(tcpc);

	LOG_INFO("[stcpc]:Peer disconnect.\r\n");

	// 3��������������ӹ���
	api_nb.timer_attach((nb_info_t*)tcpc, 3000, (fp_nb_timer_handler)tcpc_timer_resolve_connect, NULL, 0);

	return STATE_OK;
}

static const tcpc_if_t tcpc_stcpc_if = {
	tcpc_stcpc_cb_enter ,
	tcpc_stcpc_cb_exit ,
	tcpc_stcpc_cb_conn_ok ,
	tcpc_stcpc_cb_conn_err ,
	tcpc_stcpc_cb_disconn ,
	tcpc_stcpc_cb_recv
};













/*
json ת serial config

{
		"baud":115200,		//��ֵ�� 1200~115200
		"bits":"8bit",		//"8bit" "9bit"
		"sbits":"0.5bit",	//"0.5bit" "1bit" "1.5bit" "2bit"
		"parity":"none",	//"none" "odd" "even"
		"rs485":false
}

*/

static int serial_json_cfg(serial_cfg_t* cfg, const char* js, size_t js_len, jsmntok_t* tk)
{
	char str[32];
	u32_t u32_tmp;

	if(STATE_OK != jsmn.key2val_uint(js, tk, "baud", &cfg->baudrate)){
		LOG_WARN("Default baudrate.\r\n");
		cfg->baudrate = 115200;	//default
	}

	if(STATE_OK != jsmn.key2val_str(js, tk, "bits", str,  sizeof(str), (size_t*)&u32_tmp)){
		LOG_WARN("Default bits.\r\n");
		cfg->bits = 0;
	}
	else{
		if(str[0] == '9')cfg->bits = 1; else cfg->bits = 0;
	}

	if(STATE_OK != jsmn.key2val_str(js, tk, "sbits", str,  sizeof(str), (size_t*)&u32_tmp)){
		LOG_WARN("Default sbits.\r\n");
		cfg->sbits = 1;
	}
	else{
		 //"0.5bit" "1bit" "1.5bit" "2bit"
		if(0 == _strncmp(str, "0.5bit", u32_tmp))
			cfg->sbits = 0;
		else if(0 == _strncmp(str, "1.5bit", u32_tmp))
			cfg->sbits = 2;
		else if(0 == _strncmp(str, "2bit", u32_tmp))
			cfg->sbits = 3;
		else 	cfg->sbits = 1;
	}

	if(STATE_OK != jsmn.key2val_str(js, tk, "parity", str,  sizeof(str), (size_t*)&u32_tmp)){
		LOG_WARN("Default parity.\r\n");
		cfg->Parity = 0;
	}
	else{
		 //"none" "even" "odd"
		if(0 == _strncmp(str, "odd", u32_tmp))
			cfg->Parity = 1;
		else if(0 == _strncmp(str, "even", u32_tmp))
			cfg->Parity = 2;
		else 	cfg->Parity = 0;
	}

	if(STATE_OK != jsmn.key2val_bool(js, tk, "rs485", (u8_t*)&u32_tmp)){
		LOG_WARN("Default rs485.\r\n");
		cfg->option = 0;
	}
	else{
		if(TRUE == u32_tmp)cfg->option = 1; else cfg->option = 0;
	}

	return STATE_OK;
}


/*
{	"enable":true,	//��ѡ
	"net":{
		"domain":"",
		"dip":"",
		"dport":0,
		"lport":55555
	}, 
	"serial":{
		"baud":115200,		//��ֵ�� 1200~115200
		"bits":"8bit",		//"8bit" "9bit"
		"sbits":"0.5bit",	//"0.5bit" "1bit" "1.5bit" "2bit"
		"parity":"none",	//"none" "odd" "even"
		"rs485":false
	},
	"holdtime":100
}
*/


int main(int argc, char* argv[])
{
	int ret = STATE_OK;
	char* js_buf = NULL;
	size_t js_len;

	tcpc_info_t* tcpc_inf = NULL;
	stcpc_opt_t* stcpc;

	jsmntok_t tk[32];	//180+ bytes


	//��������json�ļ�	2048bytes max
	js_buf = api_fs.read_alloc("/app/stcpc/cfg.json", 2048, &js_len, 0);
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





	u8_t u8_tmp;

	//enable
	if(STATE_OK == jsmn.key2val_bool(js_buf, tk, "enable",  &u8_tmp) && FALSE == u8_tmp){
		LOG_INFO("stcpc disabled, abort.\r\n");
		goto load_abort;
	}


	//���������Դ ������֮

	tcpc_inf = api_tcpc.info_alloc(sizeof(stcpc_opt_t));	//����Ϊ �û����ݽṹ(stcpc_opt_t) ���ٿռ䣬������tcpc�������Ҫ�Ŀ��handle
	if(NULL == tcpc_inf)goto load_abort;
	stcpc = api_tcpc.info_opt(tcpc_inf);	//ͨ�����handle��ȡ �û����ݽṹ��ָ��

	u16_t local_port;
	jsmntok_t *jt;

	//�ҵ�net����
	jt = jsmn.key_value(js_buf, tk, "net");

	if(STATE_OK !=	jsmn.key2val_str(js_buf, jt, "domain", stcpc->domain, sizeof(stcpc->domain), NULL))
		goto load_abort;
	if(STATE_OK != jsmn.key2val_ipv4(js_buf, jt, "dip", &(stcpc->peer_ip.s_addr)))
		goto load_abort;
	if(STATE_OK != jsmn.key2val_u16(js_buf, jt, "dport", &(stcpc->peer_port)))
		goto load_abort;
	if(STATE_OK != jsmn.key2val_u16(js_buf, jt, "lport", &local_port))
		goto load_abort;


	jt = jsmn.key_value(js_buf, tk, "serial");
	serial_json_cfg(&(stcpc->serial_cfg), js_buf, js_len, jt);	//json -> serial_cfg_t


	if(STATE_OK != jsmn.key2val_u16(js_buf, tk, "holdtime", &(stcpc->hold_time)))
		goto load_abort;


	//json������� �ͷ�֮
	if(js_buf){
		_free(js_buf);
		js_buf = NULL;
	}


	LOG_INFO("stcpc: cfg file parsed...\r\n");


	//����ȫ�������ṹ��Ĵ���
	api_tcpc.info_preset(tcpc_inf, "stcpc", local_port, (tcpc_if_t*)&tcpc_stcpc_if);	//����tcpc���

	//��ܽ�����Ϣ���������˳�
	if(STATE_OK != api_tcpc.start(tcpc_inf))
		goto load_abort;

	return ADDON_LOADER_GRANTED;



load_abort:
	LOG_INFO("stcpc: aborting...\r\n");


	if(NULL != tcpc_inf)
		api_tcpc.info_free(tcpc_inf);


	if(NULL != js_buf)
		_free(js_buf);
	return ADDON_LOADER_ABORT;
}


//httpd�ӿ�  ��� json
/*

GET http://192.168.1.105/logic/wifiIO/invoke?target=stcpc.stcpc_status

{
	"up_byte":yyyyyyyyyy,
	"dn_byte":nnnnnnnnnn,
	"state":"connecting...",
	"ip":"192.168.120.124"	//dest
}

*/
int __ADDON_EXPORT__ JSON_FACTORY(status)(char*arg, int len, fp_consumer_generic consumer, void *ctx)
{
	char buf[128];
	int n, size = sizeof(buf);

	tcpc_info_t* tcpc_inf;
	stcpc_opt_t* stcpc;

	tcpc_inf = (tcpc_info_t*)api_nb.find("stcpc");

	//�ҵ�nb��ܳ���
	if(tcpc_inf){
		char* str;
		stcpc = api_tcpc.info_opt(tcpc_inf);
		switch(api_tcpc.state(tcpc_inf)){
		case TCPC_STATE_IDLE:
			str = "Resolving...";
			break;
		case TCPC_STATE_CONNECTING:
			str = "Connecting...";
			break;
		case TCPC_STATE_CONNECTED:
			str = "Connected";
			break;
		default:
			str = "Panic!";
			break;
		}

		n = 0;
		n += utl.snprintf(buf + n, size-n, "{\"up_byte\":%u,\"dn_byte\":%u,\"state\":\"%s\",\"ip\":\"", 
					stcpc->statistic_up_serial_bytes, stcpc->statistic_down_serial_bytes, str);
		n += api_net.ntoa_buf(&(stcpc->peer_ip), buf + n);	//IP
		n += utl.snprintf(buf + n, size-n, "\"}");

	}else{
		n = 0;
		n += utl.snprintf(buf + n, size-n, "{\"err\":\"not running.\"}");
	}

	return  consumer(ctx, (u8_t*)buf, n);
}




