
/**
 * @file			pwcl.c
 * @brief			Pulse Width Code Language ���������������
 *
 *					����鿴read.md
 * @author			dy@wifi.io
*/


#include "include.h"



/*
PWC: Pulse Width Codec


	width:: [+|-]integer

	width_seq:: /(width(,width)* /)

	width_until:: /<width/>

	seg_ref:: /[integer/]

	op_exp:: integer'*'width_seq	|  integer'*'seg_ref	| integer'*'width_until

	pwc_segment:: (width_seq|width_until|seg_ref|op_exp)+	

	pwc:: (pwc_segment;)+

	characters: '('  ')' '['  ']' '<' '>' '*'  ','  '+'  '-'  Integer

*/

#define PWC_PIN WIFIIO_GPIO_01

#define PMC_CODE_MAX 256

#define STAT_PWC_PAUSE	1
#define STAT_PWC_HALT	2

#define PMC_SEG_STACK_DEEPTH 16

#define PWC_STATE_IDLE 0
#define PWC_STATE_RUNNING 1

typedef struct {
	u8_t Head;
	u8_t End;
	u8_t Cyc;
	u8_t Ptr;	
}segment_ctx_t;


typedef struct {
	char pwc_raw[PMC_CODE_MAX];
	segment_ctx_t* segctx_top;
	segment_ctx_t* segctx_base;
	u32_t time;		//ȫ��ʱ��
	u16_t onload;	//���һ�θ��µ�����
	s8_t sign;		//��ǰ����
	s8_t plen;		//pwc_raw ��pattern���ֵĳ���
	char start_flag;
}pwc_t;


#define TOP_CTX (pwc->segctx_top)
#define IS_EMPTY (pwc->segctx_top == pwc->segctx_base)


pwc_t glb_pwc;
segment_ctx_t stack[PMC_SEG_STACK_DEEPTH];

///////////////////////////////////////////////

char* modified_atou(char* c, unsigned int *out)
{
	unsigned int dig = 0;
	while(_isdigit(*c)){
		dig = dig*10 + *c - '0';
		c++;
	}
	*out = dig;
	return c;
}



///////////////////////////////////////////////




static int pwout(pwc_t* pwc, s8_t sign, u16_t width)
{
	s8_t o_sign = (sign == 0)? pwc->sign: sign;
	
	//rt_kprintf(o_sign > 0? ",%d":",-%d",width);
	pwc->onload = width;
	pwc->sign = o_sign;

	pwc->time += width;

	if(width)
		return STAT_PWC_PAUSE;
	else
		return 0;

}

static int pwout_until(pwc_t* pwc, s8_t sign, u16_t width)
{
	s8_t o_sign;

	//<0>��ʾ����ȫ��ʱ��
	if(width == 0){
		pwc->time = 0;
		goto safe_exit;
	}

	//С�������ȫ��ʱ��
	if(width <= pwc->time){
		pwc->time -= width;
		goto safe_exit;
	}

	//����������Ⱥ�width
	else{
		width -= pwc->time;
		pwc->time = 0;
	}

	o_sign = (sign == 0)? pwc->sign: sign;

	pwc->onload = width;
	pwc->sign = o_sign;



safe_exit:
	//�����width��������жϣ��������
	if(pwc->onload)
		return STAT_PWC_PAUSE;
	else
		return 0;
}




static int pwc_segctx_pop(pwc_t* pwc)
{

	segment_ctx_t* tops = pwc->segctx_top;

	//�����ѭ����������ǰCtx
	if(tops->Cyc > 0){
		tops->Cyc--;
		tops->Ptr = tops->Head;
		return 0;
	}

	//�����ջ�������ݣ�����
	if(tops > pwc->segctx_base){
		tops --;
		pwc->segctx_top = tops;
	}
	
	//��
	else
		return STAT_PWC_HALT;

	
	return 0;

}

static int pwc_segctx_push(	pwc_t* pwc,u8_t Head,u8_t End,u8_t Cyc)
{
	segment_ctx_t* tops = pwc->segctx_top;
	if(tops - pwc->segctx_base + sizeof(segment_ctx_t) < PMC_SEG_STACK_DEEPTH * sizeof(segment_ctx_t)){
		tops++;
		tops->Head = tops->Ptr = Head;
		tops->End = End;
		tops->Cyc = (Cyc>0?Cyc-1:Cyc);
		pwc->segctx_top = tops;
		return STATE_OK;
		
	}else
		return STATE_FULL;
}


static int pwc_segment(pwc_t* pwc,int s, u8_t *h,u8_t *e)
{
	u8_t i = 0;
	char* c = pwc->pwc_raw;

	if(s == 0)
		*h = 0;

	while(c[i] != '\0'){
		
		if(s == 0 && c[i] == ';'){
			break;
		}

		if((s > 0) && (c[i] == ';') && (--s == 0)){
			*h = i+1;
		}

		i++;
	}

	*e = i;
	
	if(s > 0)
		return -1;
	else
		return 0;

}




//ret 0:OK
//w, s represent width & sign
//�ֽ� "[+|-]123"    ��ʽ�ַ���
static char* width_parser(int *o_ret,char *i_p, s8_t *o_s, u16_t *o_w)
{
	char* p = i_p;
	u32_t v = 0;
	if(_isdigit(*p)){
		*o_s = 0;
	}
	else if(*p == '+'){
		*o_s = 1;
		p++;
	}
	else if(*p == '-'){
		*o_s = -1;
		p++;
	}
	else{
		*o_ret = -1;
		return p;
	}

	p = modified_atou(p, (unsigned int*)&v);
	*o_w = v;
	return p;

}



int pwc_set_code(pwc_t* pwc, char* c)
{
	char* p = pwc->pwc_raw;
	u8_t i = 0, h = 0;

	_strcpy(pwc->pwc_raw, c);
	pwc->plen = _strlen(c);

	//��λ״̬��
	pwc->time = 0;
	pwc->sign = -1;
	pwc->segctx_top = pwc->segctx_base = stack;
	pwc->onload = 0;

	//��ջ�������ڷ���ʹ�������
 	_memset(stack, 0xA5, sizeof(stack));

	//�ҵ����һ��segment ѹ���ջ��
	while(p[i] != '\0'){
		if(p[i] == ';' && p[i+1] != '\0'){
			h = i+1;
		}
		i++;
	}
	pwc_segctx_push(pwc,h,i,1);
	return 0;
}




int pwc_caculate_next(pwc_t* pwc,char *err)
{
	
	int ret = 0;
	//char* perr = (void*)0;
	char *p, *q;

safe_ctx_begin:

	//�������볤�Ȼ��߶�ջ���ж��Ƿ��ܼ���
	while(TOP_CTX->Ptr == TOP_CTX->End){
		ret = pwc_segctx_pop(pwc);
		if( ret != 0){
			//perr = os_str_push(err, "...Done.");
			goto safe_exit;
		}
	};

	p = pwc->pwc_raw + TOP_CTX->Ptr;
	q = p;


same_ctx_repeat:	//���������������ﶼҪ��֤p == q


		switch(*q){

			case('('):	//width_seq	 like" -> (xx,xx)"
			case(','):	//width_seq	 like"(xx           ->,xx)"
			{
				u16_t w = 0;
				s8_t s = 0;
				q += 1;

				//�ֽ���������
				q = width_parser(&ret, q, &s, &w);//q: "(xx           ->,xx)" or "(xx,xx           ->)"
				TOP_CTX->Ptr += q - p;
				p = q;

				if(0 != ret){
					//perr = os_str_push(err, "Width parser err.");
					goto err_safe_exit;
				}


				//���
				ret = pwout(pwc, s,w);


				//��֮����")"��ָ��ֱ���ƿ�
				if(')' == *q ){
					TOP_CTX->Ptr += 1;	// "(xx ,xx)          ->xx"


					if(STAT_PWC_PAUSE  == ret)
						goto safe_exit;



					//����ctx������ϣ�
					if(TOP_CTX->Ptr == TOP_CTX->End)
						goto safe_ctx_begin;

					q++; p = q;
				}


				if(STAT_PWC_PAUSE  == ret)
					goto safe_exit;

				//Ҫô��","  , q: "(xx           ->,xx)xx",���߻�û�н�����
				//Ҫô��")          ->xx"
				goto same_ctx_repeat;

			}
			//	break;



			case('['):
			{
				u32_t idx = 0;
				u8_t h = 0,e = 0;

				q += 1;	//q: "[     ->xx]xx"

				//p = q;
				q = modified_atou(q, (unsigned int *)&idx) + 1;	//q: "[xx]     ->xx"
				if(	(q-p == 2)||	//in case "[]" or "[asc]"
					(*(q-1) != ']')|| //in case "[00asc]"
					0 != (ret = pwc_segment(pwc,idx, &h,&e))){
					//perr = os_str_push(err, "[] err.");
					goto err_safe_exit;
				}

				//���ctx����β������[]����ôֱ���滻CTX����β
				if(TOP_CTX->Ptr == TOP_CTX->Head && TOP_CTX->Ptr + (q - p) == TOP_CTX->End){
					TOP_CTX->Ptr = TOP_CTX->Head = h;
					TOP_CTX->End = e;

					p = pwc->pwc_raw + TOP_CTX->Ptr;
					q = p;

					goto same_ctx_repeat;
				}

				//����ǰctxָ����һ��Ԫ
				TOP_CTX->Ptr += q - p;
				p = q;

				//��[]�滻����ѹ���ջ
				ret = pwc_segctx_push(pwc,h, e, 1);
				if(ret){
					//perr = os_str_push(err, "stack full.");
					goto err_safe_exit;
				}
				goto safe_ctx_begin;

			}

			//	break;

			case('<'):
			{
				u16_t w = 0;
				s8_t s = 0;
				q += 1;

				//�ֽ���������
				q = width_parser(&ret, q, &s, &w)+1;//q: "<xx>           ->xx" 
				TOP_CTX->Ptr += q - p;
				p = q;
				if(0 != ret){
					//perr = os_str_push(err, "<xx> Width parser err.");
					goto err_safe_exit;
				}


				//���
				ret = pwout_until(pwc,s,w);
				if(STAT_PWC_PAUSE  == ret)
					goto safe_exit;


				if(TOP_CTX->Ptr < TOP_CTX->End)
					goto same_ctx_repeat;
				else
					goto safe_ctx_begin;

			}

			//	break;

			default:

				//��Ϊ��ֵ��ӦΪѭ��
				if(_isdigit(*q)){	//(xxx)     ->2*[xx]

					u32_t cnt = 0;
					q = modified_atou(q, (unsigned int*)&cnt) + 1;	//q: "(xxx)2*     ->[xx]"
					TOP_CTX->Ptr += q - p;
					p = q;

#ifdef NONE_CYCLE_WIDTH_UNTIL
					//�ų�����ѭ��Ϊ0��֮����*��ѭ���ڲ���()or[]
					if(cnt == 0 || *(q-1) != '*' || ((*q != '[')&&(*q != '(')))
#else
					//�ų�����ѭ��Ϊ0��֮����*��ѭ���ڲ���()or[]or<>
					if(cnt == 0 || *(q-1) != '*' || ((*q != '[')&&(*q != '(')&&(*q != '<')))
#endif
					{	//perr = os_str_push(err, "Repeat err.");
						ret = -1;
						goto err_safe_exit;
					}


					//��ѭ����������1����ôѹջ����ȫѭ��
					if(cnt > 1){
						u8_t h = TOP_CTX->Ptr;

						//1 ����Ƚ�Σ�� ��û�г��ȼ��

#ifdef NONE_CYCLE_WIDTH_UNTIL
						while((*q != ']') && (*q != ')'))q++; 
#else
						while((*q != ']') && (*q != ')') && (*q != '>'))q++; 
#endif
						q+=1;

						TOP_CTX->Ptr += q - p;
						p = q;

						ret = pwc_segctx_push(pwc, h, TOP_CTX->Ptr,cnt);
						if(ret){
							//perr = os_str_push(err, "stack full.");
							goto err_safe_exit;
						}
						goto safe_ctx_begin;
					}

					//��ѭ����������1��ֱ�Ӵ���֮���"("
					goto same_ctx_repeat;


				}
				else{
					ret = -1;
					//perr = os_str_push(err, "err char");
					goto err_safe_exit;
				}

				//break;


		}





safe_exit:
err_safe_exit:


	return ret;
}



void io_pwc_start(pwc_t* pwc)
{
	pwc->start_flag = PWC_STATE_RUNNING;
	LOG_INFO("Next pulse %u us.\r\n", pwc->onload);

	api_tim.tim6_start(1000);	//1000us��ʼ
}


void io_pwc_stop(pwc_t* pwc)
{
	pwc->start_flag = PWC_STATE_IDLE;
	api_io.toggle(WIFIIO_GPIO_02);
}


void io_pwc_iteration(pwc_t* pwc) 
{

	while(!api_tim.tim6_is_timeout())
		if(PWC_STATE_IDLE == pwc->start_flag)return;	//�ȵ�ʱ��� �ٶ���

	//��Ҫ�����촦�������
	if(pwc->sign > 0)
		api_io.high(PWC_PIN);	//IO_GPIO->BSRR = IO7_PIN;
	else if(pwc->sign < 0)
		api_io.low(PWC_PIN);	//IO_GPIO->BRR = IO7_PIN;

	if(pwc->onload > 0){
		api_tim.tim6_start(pwc->onload);		//���Ķ�ʱ��ʱ��
 
		//׼���µ���ʱ
		pwc->onload = 0;
		pwc_caculate_next(pwc,(void*)0);

		//�����ڲ������һ��widthʱHALT
		//if(STAT_PWC_HALT == pwc_caculate_next(pwc,(void*)0))			io_pwc_stop();
	}
	else	//�ر�
		io_pwc_stop(pwc);

}



int main(int argc, char* argv[])
{
	api_tim.tim6_init(1000000);	//��ʼ����ʱ��6 1usΪ��λ


	api_io.init(PWC_PIN, OUT_PUSH_PULL);
	api_io.high(PWC_PIN);

	api_io.init(WIFIIO_GPIO_02, OUT_PUSH_PULL);
	api_io.high(WIFIIO_GPIO_02);



	//��װ����
	pwc_set_code(&glb_pwc, "(-10000);(+10000);100*[0];100*[1];[3][2];20*[4];");

	if(STAT_PWC_PAUSE == pwc_caculate_next(&glb_pwc,(void*)0)){	//�����һ������ֵ
		//��ôִ��֮
		io_pwc_start(&glb_pwc);
	}
	else{
		LOG_WARN("pwc:Caculate error.\r\n");
		return ADDON_LOADER_ABORT;
	}

	while(1){
		if(PWC_STATE_RUNNING == glb_pwc.start_flag)
			io_pwc_iteration(&glb_pwc);
		else
			_tick_sleep(100);
	}


	//���̿����˳� ���Ǵ��벻���ͷ�
	return ADDON_LOADER_GRANTED;	//ADDON_LOADER_ABORT;
}






/*
////////////////////////////////////////
Json delegate interface are mostly used for callbacks, and is supported by both Httpd & OTd.

So if you want your code be called from web browser or from cloud(OTd),you write code in the form below.

int JSON_RPC(xxx)(jsmn_node_t* pjn, fp_json_delegate_ack ack, void* ctx){}

httpd will receive POST request like:
{
	"method":"pwc.code",
	"params":"(-10000);(+10000);100*[0];100*[1];[3][2];50*[4];"
}
////////////////////////////////////////
*/

int JSON_RPC(code)(jsmn_node_t* pjn, fp_json_delegate_ack ack, void* ctx)	//�̳���fp_json_delegate_start
{
	int ret = STATE_OK;
	char* err_msg = NULL;
	char pwmcode[256];

	LOG_INFO("DELEGATE pwc.code \r\n");



/*
{
	"method":"pwc.code",
	"params":"(-10000);(+10000);100*[0];100*[1];[3][2];50*[4];"
}
*/

//	pjn->js point to very begining of json string.
//	pjn->tkn is the token object which points to json element "0123456789ABCDEF"
//	Google "jsmn" (super lightweight json parser) for detailed info.

	if(NULL == pjn->tkn || 
	    JSMN_STRING != pjn->tkn->type ||											//must be string
	    STATE_OK != jsmn.tkn2val_str(pjn->js, pjn->tkn, pwmcode, sizeof(pwmcode), NULL)){		//parse string into binary array
		ret = STATE_ERROR;
		err_msg = "param error.";
		LOG_WARN("pwc:%s.\r\n", err_msg);
		goto exit_err;
	}

	if(PWC_STATE_IDLE == glb_pwc.start_flag){	//idle
		LOG_INFO("New code:%s\r\n", pwmcode);
		ret = pwc_set_code(&glb_pwc, pwmcode);
		if(STATE_OK == ret && STAT_PWC_PAUSE == pwc_caculate_next(&glb_pwc,(void*)0)){
			//��ôִ��֮
			io_pwc_start(&glb_pwc);
		}
		else
			LOG_WARN("Start error.\r\n");
	}
	else{
		ret = STATE_ERROR;
		err_msg = "busy.";
		LOG_WARN("pwc:%s.\r\n", err_msg);
		goto exit_err;
	}



//exit_safe:
	if(ack)
		jsmn.delegate_ack_result(ack, ctx, ret);	//just reurn {"result":0}
	return ret;

exit_err:
	if(ack)
		jsmn.delegate_ack_err(ack, ctx, ret, err_msg);	//return {"error":{"code":xxx, "message":"yyyy"}}
	return ret;
}




