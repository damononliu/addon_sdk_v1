
/**
 * @file			tcpc.c
 * @brief			tcp�ͻ�����ʾ����
 *	
 *	
 * @author			dy@wifi.io
*/


#include "include.h"


#define DOMAIN_SIZE_MAX 64

//���ﶨ��һ���û�Ӧ�ó�������ݽṹ��֮����β׺��һ��opt
//ͨ��	api_tcpc.info_alloc(sizeof(tcpc_opt_t)) �������ٿռ�
//ͨ��	api_tcpc.info_opt(tcpc_inf)  ��ȡָ��


typedef struct demo_tcpc_opt{
	char domain[DOMAIN_SIZE_MAX];
	u16_t peer_port;
	net_addr_t peer_ip;
//	u8_t pkt_buf[STCPC_LOADMAX_SIZE_MAX];	//���ͻ���
	//u16_t pkt_buf_len;

}demo_tcpc_opt_t;



static int  tcpc_timer_resolve_connect(tcpc_info_t* tcpc, void* ctx)
{
	int ret = STATE_OK;
	demo_tcpc_opt_t* demotcpc;
	demotcpc = (demo_tcpc_opt_t*)api_tcpc.info_opt(tcpc);

	//�����ǰ��û�н����������̣���ô����
	if(TCPC_STATE_IDLE == api_tcpc.state(tcpc)){

		//���ipΪ0������domain�ĵ�ַ
		if(demotcpc->peer_ip.s_addr == 0){
			ret = api_net.name2ip(demotcpc->domain, &(demotcpc->peer_ip));

			if(STATE_OK != ret){
				LOG_WARN("demotcpc: Domain %s resolve failed.\r\n", demotcpc->domain);
				goto retry;
			}
			LOG_INFO("demotcpc: Domain %s resolved to %s.\r\n", demotcpc->domain, api_net.ntoa(&demotcpc->peer_ip));
		}


		//��������
		api_tcpc.req_conn(tcpc, demotcpc->peer_ip.s_addr, demotcpc->peer_port);
		LOG_INFO("demotcpc: Try connecting to %s.\r\n", api_net.ntoa(&demotcpc->peer_ip));


		//�������Ӻ�����׼����������Ϊ ϵͳ�������ڿ���netif��û��up����һ��try����û��Ч��
		//����� ״̬Ϊ TCPC_STATE_CONNECTING����ô�Ͳ�����try�ˣ�ֱ�ӵȴ�CONN_OK ���� CONN_ERR �¼�����
		retry:
			return 3000;	// 3�������

	}
	
	//����Ѿ�������������
	else
		//���ﷵ��0 ��ʶ���ٽ�����ʱ
		return NB_TIMER_RELEASE;

}

static int tcpc_timer_timeout(nb_info_t* pnb, nb_timer_t* ptmr, void *ctx)
{
	tcpc_info_t* tcpc = (tcpc_info_t*)pnb;

	if(TCPC_STATE_CONNECTED == api_tcpc.state(tcpc)){
		api_tcpc.req_disconn(tcpc);	//�����ر�����
	}
	return NB_TIMER_RELEASE;
}


//���� �������� ������Ŀ��� ��ʱ�� ��ʹ֮����ִ��

static int tcpc_demotcpc_cb_enter(tcpc_info_t* tcpc)
{
//	demo_tcpc_opt_t* demotcpc = api_tcpc.info_opt(tcpc);

	LOG_INFO("demotcpc: In tcpc enter().\r\n");

#define NO_WAIT 0
	//�������� ����Ŀ�꣬��nb��ʼ��ת��ִ��
	api_nb.timer_attach((nb_info_t*)tcpc, 0, (fp_nb_timer_handler)tcpc_timer_resolve_connect, NULL, NO_WAIT);

	return STATE_OK;

}




static int tcpc_demotcpc_cb_exit(tcpc_info_t* tcpc)
{
	//demo_tcpc_opt_t* demotcpc = api_tcpc.info_opt(tcpc);
	LOG_INFO("demotcpc: In tcpc exit().\r\n");

	return STATE_OK;
}


#define TIMER_CTX_MAGIC_KEEPALIVE	(void*)0x9527ABCD	//ͨ��ctx ���
#define TIMEOUT_MS_KEEPALIVE	3000

static int tcpc_demotcpc_cb_recv(tcpc_info_t* tcpc)
{
	int ret = STATE_OK;
	net_pkt_t* pkt;
//	demo_tcpc_opt_t* demotcpc = api_tcpc.info_opt(tcpc);

	ret = api_tcpc.recv(tcpc, &pkt);
	if(ret != STATE_OK){
		LOG_WARN("demotcpc: recv err(%d).\r\n",ret);
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
			//���ѵ� pkt_ptr pkt_len
			LOG_INFO("Income %d.\r\n", pkt_len);
		}
	}while(api_net.pkt_next(pkt) >= 0);

	api_net.pkt_free(pkt);



	//��ʱ����ʱ��
	nb_timer_t* ptmr = api_nb.timer_by_ctx((nb_info_t*)tcpc, TIMER_CTX_MAGIC_KEEPALIVE);
	if(NULL == ptmr){
		LOG_WARN("Timer invalid.\r\n");
		return STATE_ERROR;}
	api_nb.timer_restart((nb_info_t*)tcpc, TIMEOUT_MS_KEEPALIVE, ptmr);


	return ret;
}



//���ӳɹ�
int tcpc_demotcpc_cb_conn_ok(tcpc_info_t* tcpc)
{
	int ret = STATE_OK;
	demo_tcpc_opt_t* demotcpc = api_tcpc.info_opt(tcpc);

	LOG_INFO("[demotcpc]:Connected to [%s:%u].\r\n", api_net.ntoa(&(demotcpc->peer_ip)), demotcpc->peer_port);

	//���keepalive��ʱ��
	nb_timer_t* ptmr;
	if(NULL == (ptmr = api_nb.timer_by_ctx((nb_info_t*)tcpc, TIMER_CTX_MAGIC_KEEPALIVE))){	//��û�о������µ�
		ptmr = api_nb.timer_attach((nb_info_t*)tcpc, TIMEOUT_MS_KEEPALIVE, tcpc_timer_timeout, TIMER_CTX_MAGIC_KEEPALIVE, 0);	// 3sû��������ͶϿ�����
	}
	else
		api_nb.timer_restart((nb_info_t*)tcpc, TIMEOUT_MS_KEEPALIVE, ptmr);


	//����
	api_tcpc.send(tcpc, "hello!", sizeof("hello!")-1);


	return ret;
}


//����ʧ��
int tcpc_demotcpc_cb_conn_err(tcpc_info_t* tcpc)
{
	int ret = STATE_OK;
	demo_tcpc_opt_t* demotcpc = api_tcpc.info_opt(tcpc);

	LOG_INFO("[demotcpc]:Connect failed.\r\n");

	//Set IP to 0, invoke new DNS resolve.
	if(demotcpc->domain[0] != '\0')
		 demotcpc->peer_ip.s_addr = 0;

	// 3��������������ӹ���
	api_nb.timer_attach((nb_info_t*)tcpc, 3000, (fp_nb_timer_handler)tcpc_timer_resolve_connect, NULL, 0);

	return ret;
}

//���ӶϿ�
int tcpc_demotcpc_cb_disconn(tcpc_info_t* tcpc)
{
	//demo_tcpc_opt_t* demotcpc = api_tcpc.info_opt(tcpc);

	LOG_INFO("[demotcpc]:Disconnect.\r\n");


	//�����Ҫ�����������������ӹ���
	//api_nb.timer_attach((nb_info_t*)tcpc, 3000, (fp_nb_timer_handler)tcpc_timer_resolve_connect, NULL, 0);
	//Ҳ�����˳�����
	//api_nb.req_exit((nb_info_t *) tcpc);

	return STATE_OK;
}

static const tcpc_if_t tcpc_demotcpc_if = {
	tcpc_demotcpc_cb_enter ,
	tcpc_demotcpc_cb_exit ,
	tcpc_demotcpc_cb_conn_ok ,
	tcpc_demotcpc_cb_conn_err ,
	tcpc_demotcpc_cb_disconn ,
	tcpc_demotcpc_cb_recv
};














int main(int argc, char* argv[])
{

	tcpc_info_t* tcpc_inf = NULL;
	demo_tcpc_opt_t* demotcpc;


	tcpc_inf = api_tcpc.info_alloc(sizeof(demo_tcpc_opt_t));	//�½�tcpc��� ���ҿ����û���������ݽṹ
	if(NULL == tcpc_inf)
		goto load_abort;

	demotcpc = api_tcpc.info_opt(tcpc_inf);	//��ȡ�û����ݽṹָ��
	_memset(demotcpc, 0, sizeof(demotcpc));

	//���ṹ��
	_strcpy(demotcpc->domain, "wifi.io");
	demotcpc->peer_port = 80;

	
	api_tcpc.info_preset(tcpc_inf, "demotcpc", 0, (tcpc_if_t*)&tcpc_demotcpc_if);	//����tcpc���

	//��ܿ�ʼ��Ϣ����
	if(STATE_OK != api_tcpc.start(tcpc_inf))
		goto load_abort;

	//�������ʾ�������������Ҫ�˳������Ҳ��û�б�����ram�еı�Ҫ��
	LOG_INFO("demotcpc: exiting...\r\n");
	return ADDON_LOADER_ABORT;	//ADDON_LOADER_GRANTED;

load_abort:
	LOG_INFO("demotcpc: aborting...\r\n");

	if(NULL != tcpc_inf)
		api_tcpc.info_free(tcpc_inf);

	return ADDON_LOADER_ABORT;
}



