
/**
 * @file			vs1003.c
 * @brief			mp3����ģ����������
 *	
 *	
 * @author			dy@wifi.io
*/




#include "include.h"

#define VS_XDCS_PIN WIFIIO_GPIO_03
#define VS_XCS_PIN WIFIIO_GPIO_04
#define VS_DREQ_PIN WIFIIO_GPIO_02
#define VS_RST_PIN WIFIIO_GPIO_13

#define VS_IS_READY() (api_io.get(VS_DREQ_PIN))

#define VS_XDCS_LOW() api_io.low(VS_XDCS_PIN)
#define VS_XDCS_HIGH() api_io.high(VS_XDCS_PIN)

#define VS_XCS_LOW() api_io.low(VS_XCS_PIN)
#define VS_XCS_HIGH() api_io.high(VS_XCS_PIN)

#define VS_DREQ_LOW() api_io.low(VS_DREQ_PIN)
#define VS_DREQ_HIGH() api_io.high(VS_DREQ_PIN)

#define VS_RST_LOW() api_io.low(VS_RST_PIN)
#define VS_RST_HIGH() api_io.high(VS_RST_PIN)








#define VS_WRITE_COMMAND	0x02
#define VS_READ_COMMAND	0x03
//VS10XX�Ĵ�������
#define SPI_MODE	0x00   
#define SPI_STATUS	0x01   
#define SPI_BASS	0x02   
#define SPI_CLOCKF	0x03   
#define SPI_DECODE_TIME	0x04   
#define SPI_AUDATA	0x05   
#define SPI_WRAM	0x06   
#define SPI_WRAMADDR	0x07   
#define SPI_HDAT0	0x08   
#define SPI_HDAT1	0x09 
  
#define SPI_AIADDR	0x0a   
#define SPI_VOL	0x0b   
#define SPI_AICTRL0	0x0c   
#define SPI_AICTRL1	0x0d   
#define SPI_AICTRL2	0x0e   
#define SPI_AICTRL3	0x0f   
#define SM_DIFF	0x01   
#define SM_JUMP	0x02   
#define SM_RESET	0x04   
#define SM_OUTOFWAV	0x08   
#define SM_PDOWN	0x10   
#define SM_TESTS	0x20   
#define SM_STREAM	0x40   
#define SM_PLUSV	0x80   
#define SM_DACT	0x100   
#define SM_SDIORD	0x200   
#define SM_SDISHARE	0x400   
#define SM_SDINEW	0x800   
#define SM_ADPCM	0x1000   
#define SM_ADPCM_HP	0x2000		






//////////////////////////////////////////////////////////////

typedef struct 
{							
	u8_t mvol;		//������,��Χ:0~254
	u8_t bflimit;		//����Ƶ���޶�,��Χ:2~15(��λ:10Hz)
	u8_t bass;		//����,��Χ:0~15.0��ʾ�ر�.(��λ:1dB)
	u8_t tflimit;		//����Ƶ���޶�,��Χ:1~15(��λ:Khz)
	u8_t treble;		//����,��Χ:0~15(��λ:1.5dB)(ԭ����Χ��:-8~7,ͨ�������޸���);
	u8_t effect;		//�ռ�Ч������.0,�ر�;1,��С;2,�е�;3,���.

	u8_t saveflag;	//�����־,0X0A,�������;����,����δ����	
}__attribute__ ((packed)) _vs10xx_obj;




//VS10XXĬ�����ò���
_vs10xx_obj vsset=
{
	220,	//����:220
	6,		//�������� 60Hz
	15,		//�������� 15dB	
	10,		//�������� 10Khz	
	15,		//�������� 10.5dB
	0,		//�ռ�Ч��	
};











//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�

u8_t VS_SPI_ReadWriteByte(u8_t TxData)
{
	while((SPI3->SR & 1<<1)==0);//�ȴ���������	
	SPI3->DR=TxData;		//����һ��byte 
	while((SPI3->SR&1<<0)==0);//�ȴ�������һ��byte  
	return SPI3->DR;//�����յ�������					
}

void VS_SPI_SpeedLow(void){}
void VS_SPI_SpeedHigh(void){}


//��VS10XXд����
//address:�����ַ
//data:��������
void VS_WR_Cmd(u8_t address,u16_t data)
{  
	while(!VS_IS_READY());//�ȴ�����		
	VS_SPI_SpeedLow();//���� 	
	VS_XDCS_HIGH(); 	
	VS_XCS_LOW(); 	
	VS_SPI_ReadWriteByte(VS_WRITE_COMMAND);//����VS10XX��д����
	VS_SPI_ReadWriteByte(address); //��ַ
	VS_SPI_ReadWriteByte(data>>8); //���͸߰�λ
	VS_SPI_ReadWriteByte(data);	//�ڰ�λ
	VS_XCS_HIGH();		
	VS_SPI_SpeedHigh();//����	
}


//��VS10XXд����
//data:Ҫд�������
void VS_WR_Data(u8_t data)
{
	VS_SPI_SpeedHigh();//����,��VS1003B,���ֵ���ܳ���36.864/4Mhz����������Ϊ9M 
	VS_XDCS_LOW();   
	VS_SPI_ReadWriteByte(data);
	VS_XDCS_HIGH();	
}		
//��VS10XX�ļĴ���		
//address���Ĵ�����ַ
//����ֵ��������ֵ
//ע�ⲻҪ�ñ��ٶ�ȡ,�����
u16_t VS_RD_Reg(u8_t address)
{
	u16_t temp=0;		
	while(!VS_IS_READY());//�ǵȴ�����״̬ 		
	VS_SPI_SpeedLow();//���� 
	VS_XDCS_HIGH();	
	VS_XCS_LOW();		
	VS_SPI_ReadWriteByte(VS_READ_COMMAND);	//����VS10XX�Ķ�����
	VS_SPI_ReadWriteByte(address);		//��ַ
	temp=VS_SPI_ReadWriteByte(0xff); 		//��ȡ���ֽ�
	temp=temp<<8;
	temp+=VS_SPI_ReadWriteByte(0xff); 		//��ȡ���ֽ�
	VS_XCS_HIGH();	
	VS_SPI_SpeedHigh();//����	
   return temp; 
}  

//��ȡVS10xx��RAM
//addr��RAM��ַ
//����ֵ��������ֵ
u16_t VS_WRAM_Read(u16_t addr) 
{ 
	u16_t res;				
	VS_WR_Cmd(SPI_WRAMADDR, addr); 
	res=VS_RD_Reg(SPI_WRAM);  
	return res;
} 




//Ӳ��λMP3
//����1:��λʧ��;0:��λ�ɹ�	
u8_t VS_HD_Reset(void)
{
	u8_t retry=0;
	VS_RST_LOW();
	_tick_sleep(20);
	VS_XDCS_HIGH();//ȡ�����ݴ���
	VS_XCS_HIGH();//ȡ�����ݴ���
	VS_RST_HIGH();	
	while(!VS_IS_READY()&&retry<200)//�ȴ�DREQΪ��
	{
		retry++;
		_tick_sleep(1); //delay_us(50);
	};
	_tick_sleep(20);	
	if(retry>=200)return 1;
	else return 0;			
}


//��λVS10XX
void VS_Soft_Reset(void)
{	
	u8_t retry=0;				
	while(!VS_IS_READY()); //�ȴ������λ����	
	VS_SPI_ReadWriteByte(0Xff);//��������
	retry=0;
	while(VS_RD_Reg(SPI_MODE)!=0x0800)// �����λ,��ģʽ  
	{
		VS_WR_Cmd(SPI_MODE,0x0804);// �����λ,��ģʽ		
		_tick_sleep(2);//�ȴ�����1.35ms 
		if(retry++>100)break;		
	}			
	while(!VS_IS_READY());//�ȴ������λ����	
	retry=0;
	while(VS_RD_Reg(SPI_CLOCKF)!=0X9800)//����VS10XX��ʱ��,3��Ƶ ,1.5xADD 
	{
		VS_WR_Cmd(SPI_CLOCKF,0X9800);//����VS10XX��ʱ��,3��Ƶ ,1.5xADD
		if(retry++>100)break;		
	}													
	_tick_sleep(20);
} 




//�õ���Ҫ��������
//����ֵ:��Ҫ��������
u16_t VS_Get_EndFillByte(void)
{
	return VS_WRAM_Read(0X1E06);//����ֽ�
}  


//����һ����Ƶ����
//�̶�Ϊ32�ֽ�
//����ֵ:0,���ͳɹ�
//		1,VS10xx��ȱ����,��������δ�ɹ�����	
u8_t VS_Send_MusicData(u8_t* buf, size_t n)
{
	u8_t i;
	if(VS_IS_READY())  //�����ݸ�VS10XX
	{				
		VS_XDCS_LOW();  
		for(i=0;i<n;i++)
		{
			VS_SPI_ReadWriteByte(buf[i]);				
		}
		VS_XDCS_HIGH();				
	}
	else 
		return 1;

	return 0;//�ɹ�������
}


//�и�
//ͨ���˺����и裬��������л���������				
void VS_Restart_Play(void)
{
	u16_t temp;
	u16_t i;
	u8_t n;	
	u8_t vsbuf[32];
	for(n=0;n<32;n++)
		vsbuf[n]=0;//����

	temp=VS_RD_Reg(SPI_MODE);	//��ȡSPI_MODE������
	temp|=1<<3;					//����SM_CANCELλ
	temp|=1<<2;					//����SM_LAYER12λ,������MP1,MP2
	VS_WR_Cmd(SPI_MODE,temp);	//����ȡ����ǰ����ָ��

	for(i=0;i<2048;)			//����2048��0,�ڼ��ȡSM_CANCELλ.���Ϊ0,���ʾ�Ѿ�ȡ���˵�ǰ����
	{
		if(VS_Send_MusicData(vsbuf, 32)==0)//ÿ����32���ֽں���һ��
		{
			i+=32;						//������32���ֽ�
			temp=VS_RD_Reg(SPI_MODE);	//��ȡSPI_MODE������
			if((temp&(1<<3))==0)break;	//�ɹ�ȡ����
		}	
	}

	if(i<2048)//SM_CANCEL����
	{
		temp=VS_Get_EndFillByte()&0xff;//��ȡ����ֽ�
		for(n=0;n<32;n++)
			vsbuf[n]=temp;//����ֽڷ�������

		for(i=0;i<2052;)
		{
			if(VS_Send_MusicData(vsbuf, 32)==0)i+=32;//���	
		}	
	}
	else 
		VS_Soft_Reset();	//SM_CANCEL���ɹ�,�����,��Ҫ��λ	

	temp=VS_RD_Reg(SPI_HDAT0); 
	temp+=VS_RD_Reg(SPI_HDAT1);

	if(temp)					//��λ,����û�гɹ�ȡ��,��ɱ���,Ӳ��λ
	{
		VS_HD_Reset();			//Ӳ��λ
		VS_Soft_Reset();		//��λ 
	} 
}

//�趨VS10XX���ŵ������͸ߵ���
//volx:������С(0~254)
void VS_Set_Vol(u8_t volx)
{
	u16_t volt=0;			//�ݴ�����ֵ
	volt=254-volx;			//ȡ��һ��,�õ����ֵ,��ʾ���ı�ʾ 
	volt<<=8;
	volt+=254-volx;			//�õ��������ú��С
	VS_WR_Cmd(SPI_VOL,volt);//������ 
}


//�趨�ߵ�������
//bfreq:��Ƶ����Ƶ��	2~15(��λ:10Hz)
//bass:��Ƶ����			0~15(��λ:1dB)
//tfreq:��Ƶ����Ƶ��	1~15(��λ:Khz)
//treble:��Ƶ����		0~15(��λ:1.5dB,С��9��ʱ��Ϊ����)
void VS_Set_Bass(u8_t bfreq,u8_t bass,u8_t tfreq,u8_t treble)
{
	u16_t bass_set=0; //�ݴ������Ĵ���ֵ
	signed char temp=0;	
	if(treble==0)temp=0;			//�任
	else if(treble>8)temp=treble-8;
	else temp=treble-9;  
	bass_set=temp&0X0F;				//�����趨
	bass_set<<=4;
	bass_set+=tfreq&0xf;			//��������Ƶ��
	bass_set<<=4;
	bass_set+=bass&0xf;				//�����趨
	bass_set<<=4;
	bass_set+=bfreq&0xf;			//��������	
	VS_WR_Cmd(SPI_BASS,bass_set);	//BASS 
}


//�趨��Ч
//eft:0,�ر�;1,��С;2,�е�;3,���.
void VS_Set_Effect(u8_t eft)
{
	u16_t temp;	
	temp=VS_RD_Reg(SPI_MODE);	//��ȡSPI_MODE������
	if(eft&0X01)temp|=1<<4;		//�趨LO
	else temp&=~(1<<5);			//ȡ��LO
	if(eft&0X02)temp|=1<<7;		//�趨HO
	else temp&=~(1<<7);			//ȡ��HO						
	VS_WR_Cmd(SPI_MODE,temp);	//�趨ģʽ	
}	



///////////////////////////////////////////////////////////////////////////////
//��������,��Ч��.
void VS_Set_All(void)				
{			
	VS_Set_Vol(vsset.mvol);
	VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  
	VS_Set_Effect(vsset.effect);
}



//�������ʱ��						
void VS_Reset_DecodeTime(void)
{
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);//��������
}


//FOR WAV HEAD0 :0X7761 HEAD1:0X7665    
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//������Ԥ��ֵ,�ײ�III
const u16_t bitrate[2][16]=
{ 
{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//����Kbps�Ĵ�С
//����ֵ���õ�������
u16_t VS_Get_HeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;  
 	HEAD0=VS_RD_Reg(SPI_HDAT0); 
    HEAD1=VS_RD_Reg(SPI_HDAT1);
  	//printf("(H0,H1):%x,%x\n",HEAD0,HEAD1);
    switch(HEAD1)
    {        
        case 0x7665://WAV��ʽ
        case 0X4D54://MIDI��ʽ 
		case 0X4154://AAC_ADTS
		case 0X4144://AAC_ADIF
		case 0X4D34://AAC_MP4/M4A
		case 0X4F67://OGG
        case 0X574D://WMA��ʽ
		case 0X664C://FLAC��ʽ
        {
			////printf("HEAD0:%d\n",HEAD0);
            HEAD1=HEAD0*2/25;//�൱��*8/100
            if((HEAD1%10)>5)return HEAD1/10+1;//��С�����һλ��������
            else return HEAD1/10;
        }
        default://MP3��ʽ,�����˽ײ�III�ı�
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03; 
            if(HEAD1==3)
				HEAD1=1;
            else 
				HEAD1=0;
            return bitrate[HEAD1][HEAD0>>12];
        }
    }  
}

//�õ�mp3�Ĳ���ʱ��n sec
//����ֵ������ʱ��
u16_t VS_Get_DecodeTime(void)
{ 		
	u16_t dt=0;	 
	dt=VS_RD_Reg(SPI_DECODE_TIME);      
 	return dt;
} 	    					  



//��ʾ����ʱ��,������ ��Ϣ 
//lenth:�����ܳ���
void mp3_msg_show()
{	
	LOG_INFO("Bitrate:%d.\r\n",VS_Get_HeadInfo());	   //��ñ�����
	LOG_INFO("DecodeTime:%d.\r\n",VS_Get_DecodeTime()); //�õ�����ʱ��
}			  		 





//#include "test_mp3.h"

void Play_Music(void)
{

	int ret = STATE_OK;
	u8_t temp[512];
	FIL fobj;
	UINT n;


	VS_HD_Reset();
	VS_Soft_Reset();


	VS_Restart_Play();				//�������� 
	VS_Set_All();					//������������Ϣ			
	VS_Reset_DecodeTime();			//��λ����ʱ��	




	if(FR_OK != (ret = api_fs.f_open(&fobj, "/app/vs1003/demo.mp3", FA_OPEN_EXISTING | FA_READ))){
		LOG_INFO("mp3 file open err:%d.\r\n", ret);
		return;
	}


	//mp3_msg_show();
	while(FR_OK == api_fs.f_read(&fobj, temp, sizeof(temp), &n) && n > 0)   //�������ֵ���ѭ��
	{
		int i = 0;
		while(i < n){
			int j = MIN(n-i, 32);
			while(0 != VS_Send_MusicData(temp + i, j)){};	//��VS10XX������Ƶ����
			i += j;
		};

		//api_console.printf("~");
	}
	//mp3_msg_show();

	api_fs.f_close(&fobj);



/*
	n = sizeof(music);
	{
		int i = 0;
		while(i < n){
			int j = MIN(n-i, 32);
			while(0 != VS_Send_MusicData(music + i, j)){};	//��VS10XX������Ƶ����
			i += j;
			_tick_sleep(1);
		};

		//api_console.printf("~");
	}


*/


	api_console.printf("\r\n");

	VS_HD_Reset();  //Ӳ��λ
	VS_Soft_Reset();//��λ

}


//��ʼ��VS10XX��IO��	

//pin_mosi PB5
//pin_miso PB4
//pin_clock PB3


void VS_Init(void)
{
	//spi3 clock
	stm_rcc.RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	stm_rcc.RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	stm_gpio.GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
	stm_gpio.GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
	stm_gpio.GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	stm_gpio.GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	stm_gpio.GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	stm_gpio.GPIO_Init(GPIOB, &GPIO_InitStructure);

	SPI_InitTypeDef	SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	stm_spi.SPI_Init(SPI3, &SPI_InitStructure);

	stm_spi.SPI_Cmd(SPI3, ENABLE);


}	




//���Ҳ��� 
void VS_Sine_Test(void)
{												
	VS_HD_Reset();	
	VS_WR_Cmd(0x0b,0X2020);	//��������	
	VS_WR_Cmd(SPI_MODE,0x0820);//����VS10XX�Ĳ���ģʽ	
	while(!VS_IS_READY());	//�ȴ�DREQΪ��
	//printf("mode sin:%x\n",VS_RD_Reg(SPI_MODE));
	//��VS10XX�������Ҳ������0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
	//����n = 0x24, �趨VS10XX�����������Ҳ���Ƶ��ֵ��������㷽����VS10XX��datasheet
	VS_SPI_SpeedLow();//���� 
	VS_XDCS_LOW();//ѡ�����ݴ���
	VS_SPI_ReadWriteByte(0x53);
	VS_SPI_ReadWriteByte(0xef);
	VS_SPI_ReadWriteByte(0x6e);
	VS_SPI_ReadWriteByte(0x24);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	_tick_sleep(100);
	VS_XDCS_HIGH(); 
	//�˳����Ҳ���
	VS_XDCS_LOW();//ѡ�����ݴ���
	VS_SPI_ReadWriteByte(0x45);
	VS_SPI_ReadWriteByte(0x78);
	VS_SPI_ReadWriteByte(0x69);
	VS_SPI_ReadWriteByte(0x74);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	_tick_sleep(100);
	VS_XDCS_HIGH();		

	//�ٴν������Ҳ��Բ�����nֵΪ0x44���������Ҳ���Ƶ������Ϊ�����ֵ
	VS_XDCS_LOW();//ѡ�����ݴ���	
	VS_SPI_ReadWriteByte(0x53);
	VS_SPI_ReadWriteByte(0xef);
	VS_SPI_ReadWriteByte(0x6e);
	VS_SPI_ReadWriteByte(0x44);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	_tick_sleep(100);
	VS_XDCS_HIGH();
	//�˳����Ҳ���
	VS_XDCS_LOW();//ѡ�����ݴ���
	VS_SPI_ReadWriteByte(0x45);
	VS_SPI_ReadWriteByte(0x78);
	VS_SPI_ReadWriteByte(0x69);
	VS_SPI_ReadWriteByte(0x74);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	_tick_sleep(100);
	VS_XDCS_HIGH();	
}	



//ram ���� 
//����ֵ:RAM���Խ��
// VS1003����õ���ֵΪ0x807F����������;VS1053Ϊ0X83FF.																				
u16_t VS_Ram_Test(void)
{
	VS_HD_Reset();	
 	VS_WR_Cmd(SPI_MODE,0x0820);// ����VS10XX�Ĳ���ģʽ
	while (!VS_IS_READY()); // �ȴ�DREQΪ��			
 	VS_SPI_SpeedLow();//���� 
	VS_XDCS_LOW();					// xDCS = 1��ѡ��VS10XX�����ݽӿ�
	VS_SPI_ReadWriteByte(0x4d);
	VS_SPI_ReadWriteByte(0xea);
	VS_SPI_ReadWriteByte(0x6d);
	VS_SPI_ReadWriteByte(0x54);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	VS_SPI_ReadWriteByte(0x00);
	_tick_sleep(150);  
	VS_XDCS_HIGH();
	return VS_RD_Reg(SPI_HDAT0);// VS1003����õ���ֵΪ0x807F����������;VS1053Ϊ0X83FF.;	
}						







////////////////////////////////////////
//ÿһ��addon����main���ú����ڼ��غ�����
//������ �� ADDON_LOADER_GRANTED ������main�������غ�addon��ж��
////////////////////////////////////////

int main(int argc, char* argv[])
{

	//IO init 
	api_io.init(VS_DREQ_PIN, IN_PULL_UP);
	api_io.init(VS_XDCS_PIN, OUT_OPEN_DRAIN_PULL_UP);
	api_io.init(VS_XCS_PIN, OUT_OPEN_DRAIN_PULL_UP);
	api_io.init(VS_RST_PIN, OUT_OPEN_DRAIN_PULL_UP);


	//api_tim.tim6_init(100000);	//��ʼ����ʱ��6 10usΪ��λ


	VS_Init();

//	while(1)  
	{				
		LOG_INFO("Ram test.\r\n");
		VS_Ram_Test();

		LOG_INFO("Sin wave test.\r\n");
		VS_Sine_Test();

		LOG_INFO("Mp3 play...\r\n");		

		Play_Music();

	}

//	return ADDON_LOADER_GRANTED;


	return ADDON_LOADER_ABORT;


}

