/**
 * @file			app_sudp.c
 * @brief			����תudp
 *	
 *	
 * @author			yizuoshe@gmail.com
*/

#include "include.h"



#define SUDP_LOADMAX_SIZE_MAX 1460


typedef enum udp_sudp_msg_type{
	MSG_TYPE_SUDP_SERIAL = MSG_TYPE_ADD_BASE ,
	MSG_TYPE_SUDP_MAX
}udp_sudp_msg_type_t;


typedef struct add_sudp_msg{
	u16_t len;
}add_sudp_msg_t;


typedef struct add_sudp_info{

//	char domain[DOMAIN_SIZE_MAX];
//	u8_t pkt_buf[SUDP_PACKET_SIZE_MAX];
//	u16_t pkt_buf_len;

	u16_t hold_time;
	u16_t load_max;

	u16_t last_peek;

	u16_t peer_port;
	net_addr_t peer_ip;

	net_addr_t last_peer_ip;
	u16_t last_peer_port;

	nb_msg_t* serial_rx_msg;

	nb_timer_t *tmr;

	serial_cfg_t serial_cfg;

	u32_t statistic_up_serial_bytes;
	u32_t statistic_up_udp_pkts;
	u32_t statistic_down_serial_bytes;
	u32_t statistic_down_udp_pkts;

}add_sudp_info_t;











//���յ�serial���ڽ��յ��¼�
static int msg_sudp_serial_handler(udp_info_t* udp, nb_msg_t* msg)
{
	int slen = 0;
	add_sudp_info_t* sudp = (add_sudp_info_t*)api_udp.info_opt(udp);
	//add_sudp_msg_t* sudp_msg = (add_sudp_msg_t*)api_nb.msg_opt(msg);

	//��鴮������
	if(0 == (slen = api_serial.peek(SERIAL2)))
		goto safe_exit_null;


	//û����ȷ���ÿͻ��ˣ������һ��û�еõ�������Ŀ���ַ��������������
	if((sudp->peer_port == 0 || sudp->peer_ip.s_addr == 0) && 
		(sudp->last_peer_ip.s_addr == 0 || sudp->last_peer_port == 0)){
		api_serial.clean(SERIAL2);

		sudp->last_peek = 0;
		goto safe_exit_clean;
	}


	//���趨ʱ��
	if(sudp->tmr)
		api_nb.timer_restart((nb_info_t*)udp , sudp->hold_time, sudp->tmr);




	do{
		net_pkt_t* send_pkt = NULL;
		void* sbuf = NULL;
		int send_len = MIN(slen, sudp->load_max);

		//new pkt
		if((send_pkt = api_net.pkt_new()) == NULL){
			LOG_WARN("sudp: pkt alloc err.\r\n");
			break;
		}

		sbuf = (char*)api_net.pkt_alloc(send_pkt, send_len);
		if(sbuf == NULL){
			api_net.pkt_free(send_pkt);
			LOG_WARN("sudp: buf alloc err.\r\n");
			break;
		}

		//����
		api_serial.read(SERIAL2, sbuf, send_len);
		LOG_INFO_BUF("su[sei%u]", send_len);

		//����
		if(sudp->peer_port != 0 && sudp->peer_ip.s_addr != 0)
			api_udp.pkt_send(udp, send_pkt, &(sudp->peer_ip), sudp->peer_port);	//ֱ��Ŀ��
		else
			api_udp.pkt_send(udp, send_pkt, &(sudp->last_peer_ip), sudp->last_peer_port);	//������ʵ�Ŀ��

		sudp->statistic_up_serial_bytes+= send_len;
		sudp->statistic_up_udp_pkts++;


		//�ͷ�
		api_net.pkt_free(send_pkt);

		LOG_INFO_BUF("[no%u]", slen);

		slen -= send_len;
	}while(0);

	//��¼ʣ��δ����
	sudp->last_peek = slen;

	LOG_INFO_BUF("\r\n");
	LOG_FLUSH();

safe_exit_clean:

safe_exit_null:

	return STATE_OK;
}





//hold time��ʱ���
static int sudp_timer_holdtime_check(nb_info_t* nb, nb_timer_t* ptmr, void* ctx)
{
	int slen = 0;
	udp_info_t* udp = (udp_info_t*)nb;
	add_sudp_info_t* sudp = (add_sudp_info_t*)api_udp.info_opt(udp);


	//��鴮������
	if(0 == (slen = api_serial.peek(SERIAL2))){
		sudp->last_peek = 0;	//���
		goto TIMER_EXIT_NULL;
	}

	//û����ȷ���ÿͻ��ˣ������һ��û�еõ�������Ŀ���ַ��������������
	if((sudp->peer_port == 0 ||sudp->peer_ip.s_addr == 0) && 
		(sudp->last_peer_ip.s_addr == 0 || sudp->last_peer_port == 0)){
		api_serial.clean(SERIAL2);
		sudp->last_peek = 0;	//���
		goto TIMER_EXIT_CLEAN;
	}

	//������ϴβ�һ������ô˵����holdtimeʱ����������ݽ��룬������
	if(sudp->last_peek != slen){
		sudp->last_peek = slen;
		goto TIMER_EXIT_SKIP;
	}

	//����������֮
	do{
		net_pkt_t* send_pkt = NULL;
		void* sbuf = NULL;
		int send_len = MIN(slen, SUDP_LOADMAX_SIZE_MAX);

		//new pkt
		if((send_pkt = api_net.pkt_new()) == NULL){
			LOG_WARN("sudp: pkt alloc err.\r\n");
			break;
		}

		sbuf = (char*)api_net.pkt_alloc(send_pkt, send_len);
		if(sbuf == NULL){
			api_net.pkt_free(send_pkt);
			LOG_WARN("sudp: buf alloc err.\r\n");
			break;
		}

		//����
		api_serial.read(SERIAL2, sbuf, send_len);
		LOG_INFO_BUF("su[spi%u]", send_len);

		//����
		if(sudp->peer_port != 0 && sudp->peer_ip.s_addr != 0)
			api_udp.pkt_send(udp, send_pkt, &(sudp->peer_ip), sudp->peer_port);	//ֱ��Ŀ��
		else
			api_udp.pkt_send(udp, send_pkt, &(sudp->last_peer_ip), sudp->last_peer_port);	//������ʵ�Ŀ��

		sudp->statistic_up_serial_bytes+= send_len;
		sudp->statistic_up_udp_pkts++;

		//�ͷ�
		api_net.pkt_free(send_pkt);

		LOG_INFO_BUF("[no%u]", send_len);


		slen -= send_len;

	}while(slen);

	LOG_INFO_BUF("\r\n");
	LOG_FLUSH();

	sudp->last_peek = slen;

TIMER_EXIT_SKIP:
TIMER_EXIT_CLEAN:
TIMER_EXIT_NULL:

	//1С�ģ�
	//2����һ���ص�����������ֵ��ʶ�Ƿ������ʱ��
	//2�������Ҫ�ٶ���ʱ��ֱ�ӷ���0����

	//���°�װ
	return sudp->hold_time;

}


//���ڽ�����ϢԴ
static int serial_event_src_from_isr(void* sdev, void*ctx, int len)
{
	int ret = STATE_OK;
	udp_info_t * udp =  (udp_info_t*)ctx;
	add_sudp_info_t* sudp = (add_sudp_info_t*)api_udp.info_opt(udp);
	nb_msg_t* msg = sudp->serial_rx_msg;

	add_sudp_msg_t *sudp_msg = api_nb.msg_opt(msg);
	sudp_msg->len = len;


	ret = api_nb.msg_send((nb_info_t *)udp, msg, TRUE);
	//ARCH_ASSERT("",ret == STATE_OK);

	return ret;
}


//��û������peer_ip����������
static int udp_sudp_cb_enter(udp_info_t* udp)
{
	int ret = STATE_OK;
	add_sudp_info_t* sudp = api_udp.info_opt(udp);


//��װsudp�����Ϣ������
	api_nb.hdlr_set((nb_info_t*)udp, MSG_TYPE_SUDP_SERIAL, (fp_nb_msg_handler)msg_sudp_serial_handler);


	sudp->serial_rx_msg = api_nb.msg_alloc(sizeof(add_sudp_msg_t), MSG_TYPE_SUDP_SERIAL, NB_MSG_DO_NOT_DELETE);

	ARCH_ASSERT("",NULL != sudp->serial_rx_msg);




//��������
	api_serial.open(SERIAL2);
	api_serial.ctrl(SERIAL2, SERIAL_CTRL_SETUP, &(sudp->serial_cfg));	//����
	api_serial.set_rx_cb(SERIAL2, serial_event_src_from_isr, udp, sudp->load_max);	//�趨������ֵ


/*

	//����Ҫ����������������������ip
	if(0 == sudp->peer_ip.s_addr){
		if('\0' != sudp->domain[0]){
			ret = api_net.name2ip(sudp->domain, &(sudp->peer_ip));
			if(ret != STATE_OK){
				LOG_WARN("sudp: Domain %s resolve failed.\r\n", sudp->domain);
				ret = STATE_ERROR;
				goto err_exit_resolve;
			}
			LOG_INFO("sudp: Domain %s resolved to %s.\r\n", sudp->domain, api_net.ntoa(&(sudp->peer_ip)));
		}
		else	//�������񣬲���������
			LOG_WARN("sudp: Passive Service.\r\n");
	}
	else
		LOG_INFO("sudp: Use peer ip %s, skip %s.\r\n", api_net.ntoa(&(sudp->peer_ip)), sudp->domain);
*/


	//���ʱ����ֵ�����ö��Ҵ���0����װ���ڶ�ʱ��
	if(sudp->hold_time > 0){
		sudp->tmr = api_nb.timer_attach((nb_info_t*)udp, sudp->hold_time, sudp_timer_holdtime_check, NULL, 0);
		ARCH_ASSERT("",NULL != sudp->tmr);
	}

	return ret;

/*
err_exit_timer:
err_exit_resolve:

	if(sudp->serial_rx_msg)
		api_nb.msg_free(sudp->serial_rx_msg);

	api_nb.hdlr_unset((nb_info_t*)sudp, MSG_TYPE_SUDP_SERIAL);

	return ret;
*/
}




static int udp_sudp_cb_exit(udp_info_t* udp)
{

	add_sudp_info_t* sudp = api_udp.info_opt(udp);

	if(sudp->serial_rx_msg)
		api_nb.msg_free(sudp->serial_rx_msg);


	api_serial.close(SERIAL2);

	api_nb.hdlr_unset((nb_info_t*)udp, MSG_TYPE_SUDP_SERIAL);

	return STATE_OK;
}



static int udp_sudp_cb_recv(udp_info_t* udp)
{
	int ret = STATE_OK;
	net_pkt_t* pkt;
	add_sudp_info_t* sudp = api_udp.info_opt(udp);

	pkt = api_udp.recv(udp);
	if(pkt == NULL){
		LOG_WARN("sudp: recv null.\r\n");
		return STATE_OK;
	}

	do{
		net_addr_t  *paddr;
		u16_t peer_port;
		u16_t pkt_len = 0;
		u8_t *pkt_ptr = NULL; 

		if(STATE_OK != api_net.pkt_data(pkt, (void**)&pkt_ptr, &pkt_len) || pkt_len == 0)//
			break;

		paddr = api_net.pkt_faddr(pkt);
		peer_port = api_net.pkt_fport(pkt);



		LOG_INFO_BUF("su[ni:%u]", pkt_len);	//, api_net.ntoa(paddr), peer_port);

		if(pkt_len != 0){

			//����趨��Ŀ��IP
			if(sudp->peer_ip.s_addr != 0){
				//���IP
				if(paddr->s_addr != sudp->peer_ip.s_addr)
					break;
			}
			//����������IP����¼֮
			else
				sudp->last_peer_ip.s_addr = paddr->s_addr;


			//���������Ŀ��port
			if(sudp->peer_port != 0){
				//���port
				if(peer_port != sudp->peer_port)
					break;
			}
			//����������˿ڣ���¼֮
			else
				sudp->last_peer_port = peer_port;

			//���ڷ���
			pkt_len = api_serial.write(SERIAL2,  (const void*)pkt_ptr, pkt_len);
			LOG_INFO_BUF("[so:%u]", pkt_len);

			sudp->statistic_down_serial_bytes+=pkt_len;
			sudp->statistic_down_udp_pkts++;

		}

	}while(api_net.pkt_next(pkt) >= 0);

	api_net.pkt_free(pkt);

	LOG_INFO_BUF("\r\n");
	LOG_FLUSH();

	return ret;
}

static const udp_if_t udp_sudp_if = {udp_sudp_cb_enter, udp_sudp_cb_exit, udp_sudp_cb_recv};






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
		"dip":"",
		"dport":0,
		"lport":55555}, 
	"serial":{
		"baud":115200,		//��ֵ�� 1200~115200
		"bits":"8bit",		//"8bit" "9bit"
		"sbits":"0.5bit",	//"0.5bit" "1bit" "1.5bit" "2bit"
		"parity":"none",	//"none" "odd" "even"
		"rs485":false
	},
	"holdtime":100,
	"loadmax":1024
}
*/


int main(int argc, char* argv[])
{
	int ret = STATE_OK;
	char* js_buf = NULL;
	size_t js_len;

	udp_info_t* udp_inf = NULL;
	add_sudp_info_t* sudp;

	jsmntok_t tk[32];	//180+ bytes


	//��������json�ļ�	2048bytes max
	js_buf = api_fs.read_alloc("/app/sudp/cfg.json", 2048, &js_len, 0);
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
		LOG_INFO("sudp disabled, abort.\r\n");
		goto load_abort;
	}


	//���������Դ ������֮

	udp_inf = api_udp.info_alloc(sizeof(add_sudp_info_t));
	if(NULL == udp_inf)goto load_abort;
	sudp = api_udp.info_opt(udp_inf);

	u16_t local_port;
	jsmntok_t *jt;

	//�ҵ�net����
	jt = jsmn.key_value(js_buf, tk, "net");
	
	if(STATE_OK != jsmn.key2val_u16(js_buf, jt, "lport", &local_port))
		goto load_abort;

	if(STATE_OK != jsmn.key2val_u16(js_buf, jt, "dport", &(sudp->peer_port)))
		goto load_abort;

	if(STATE_OK != jsmn.key2val_ipv4(js_buf, jt, "dip", &(sudp->peer_ip.s_addr)))
		goto load_abort;


	jt = jsmn.key_value(js_buf, tk, "serial");
	serial_json_cfg(&(sudp->serial_cfg), js_buf, js_len, jt);	//json -> serial_cfg_t


	if(STATE_OK != jsmn.key2val_u16(js_buf, tk, "holdtime", &(sudp->hold_time)))
		goto load_abort;

	if(STATE_OK != jsmn.key2val_u16(js_buf, tk, "loadmax", &(sudp->load_max)))
		goto load_abort;


	//json������� �ͷ�֮
	if(js_buf){
		_free(js_buf);
		js_buf = NULL;
	}

	LOG_INFO("sudp: cfg file parsed...\r\n");




	//����ȫ�������ṹ��Ĵ���
	api_udp.info_preset(udp_inf, "sudp", local_port, (udp_if_t*)&udp_sudp_if);	//����udp���

	//��ܽ�����Ϣ���������˳�
	if(STATE_OK != api_udp.start(udp_inf))
		goto load_abort;

	return ADDON_LOADER_GRANTED;


load_abort:
	LOG_INFO("sudp: aborting...\r\n");


	if(NULL != udp_inf)
		api_udp.info_free(udp_inf);


	if(NULL != js_buf)
		_free(js_buf);
	return ADDON_LOADER_ABORT;
}


//httpd�ӿ�  ��� json
/*

	u32_t statistic_up_serial_bytes;
	u32_t statistic_up_udp_pkts;
	u32_t statistic_down_serial_bytes;
	u32_t statistic_down_udp_pkts;

GET http://192.168.1.105/logic/wifiIO/invoke?target=sudp.sudp_status

{
	"up_pkt":xxxxxxxxxx,
	"up_byte":yyyyyyyyyy,
	"dn_pkt":zzzzzzzzzz,
	"dn_byte":nnnnnnnnnn
}


*/
int __ADDON_EXPORT__ JSON_FACTORY(status)(char*arg, int len, fp_consumer_generic consumer, void *ctx)
{
	char buf[120];
	int n, size = sizeof(buf);

	udp_info_t* udp_inf;
	add_sudp_info_t* sudp;

	udp_inf = (udp_info_t*)api_nb.find("sudp");

	//�ҵ�nb��ܳ���
	if(udp_inf){
		sudp = api_udp.info_opt(udp_inf);

		n = 0;
		n += utl.snprintf(buf + n, size-n, "{\"up_pkt\":%u,\"up_byte\":%u,\"dn_pkt\":%u,\"dn_byte\":%u}", 
					sudp->statistic_up_udp_pkts, sudp->statistic_up_serial_bytes,
					sudp->statistic_down_udp_pkts, sudp->statistic_down_serial_bytes);
	}else{
		n = 0;
		n += utl.snprintf(buf + n, size-n, "{\"err\":\"not running.\"}");
	}

	return  consumer(ctx, (u8_t*)buf, n);
}




