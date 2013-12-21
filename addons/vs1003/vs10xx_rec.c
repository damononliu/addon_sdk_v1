
/**
 * @file			vs10xx.c
 * @brief			vs10xx ¼������
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

//#define SM_DIFF	0x01	
#define SM_JUMP	0x02	
//#define SM_RESET	0x04	
//#define SM_OUTOFWAV	0x08	
//#define SM_PDOWN	0x10	
//#define SM_TESTS	0x20	
//#define SM_STREAM	0x40	
#define SM_PLUSV	0x80	
//#define SM_DACT	0x100	
//#define SM_SDIORD	0x200	
//#define SM_SDISHARE	0x400	
//#define SM_SDINEW	0x800	
//#define SM_ADPCM	0x1000	
//#define SM_ADPCM_HP	0x2000		

//////////////////////////////////////////////////////////////



/* SCI registers */

#define SCI_MODE        0x00
#define SCI_STATUS      0x01
#define SCI_BASS        0x02
#define SCI_CLOCKF      0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA      0x05
#define SCI_WRAM        0x06
#define SCI_WRAMADDR    0x07
#define SCI_HDAT0       0x08 /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_IN0         0x08 /* VS1103 */
#define SCI_HDAT1       0x09 /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_IN1         0x09 /* VS1103 */
#define SCI_AIADDR      0x0A
#define SCI_VOL         0x0B
#define SCI_AICTRL0     0x0C /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_MIXERVOL    0x0C /* VS1103 */
#define SCI_AICTRL1     0x0D /* VS1063, VS1053, VS1033, VS1003, VS1011 */
#define SCI_ADPCMRECCTL 0x0D /* VS1103 */
#define SCI_AICTRL2     0x0E
#define SCI_AICTRL3     0x0F


/* SCI register recording aliases */

#define SCI_RECQUALITY 0x07 /* (WRAMADDR) VS1063 */
#define SCI_RECDATA    0x08 /* (HDAT0)    VS1063 */
#define SCI_RECWORDS   0x09 /* (HDAT1)    VS1063 */
#define SCI_RECRATE    0x0C /* (AICTRL0)  VS1063, VS1053 */
#define SCI_RECDIV     0x0C /* (AICTRL0)  VS1033, VS1003 */
#define SCI_RECGAIN    0x0D /* (AICTRL1)  VS1063, VS1053, VS1033, VS1003 */
#define SCI_RECMAXAUTO 0x0E /* (AICTRL2)  VS1063, VS1053, VS1033 */
#define SCI_RECMODE    0x0F /* (AICTRL3)  VS1063, VS1053 */


/* SCI_MODE bits */

#define SM_DIFF_B             0
#define SM_LAYER12_B          1 /* VS1063, VS1053, VS1033, VS1011 */
#define SM_RECORD_PATH_B      1 /* VS1103 */
#define SM_RESET_B            2
#define SM_CANCEL_B           3 /* VS1063, VS1053 */
#define SM_OUTOFWAV_B         3 /* VS1033, VS1003, VS1011 */
#define SM_OUTOFMIDI_B        3 /* VS1103 */
#define SM_EARSPEAKER_LO_B    4 /* VS1053, VS1033 */
#define SM_PDOWN_B            4 /* VS1003, VS1103 */
#define SM_TESTS_B            5
#define SM_STREAM_B           6 /* VS1053, VS1033, VS1003, VS1011 */
#define SM_ICONF_B            6 /* VS1103 */
#define SM_EARSPEAKER_HI_B    7 /* VS1053, VS1033 */
#define SM_DACT_B             8
#define SM_SDIORD_B           9
#define SM_SDISHARE_B        10
#define SM_SDINEW_B          11
#define SM_ENCODE_B          12 /* VS1063 */
#define SM_ADPCM_B           12 /* VS1053, VS1033, VS1003 */
#define SM_EARSPEAKER_1103_B 12 /* VS1103 */
#define SM_ADPCM_HP_B        13 /* VS1033, VS1003 */
#define SM_LINE1_B           14 /* VS1063, VS1053 */
#define SM_LINE_IN_B         14 /* VS1033, VS1003, VS1103 */
#define SM_CLK_RANGE_B       15 /* VS1063, VS1053, VS1033 */
#define SM_ADPCM_1103_B      15 /* VS1103 */

#define SM_DIFF           (1<< 0)
#define SM_LAYER12        (1<< 1) /* VS1063, VS1053, VS1033, VS1011 */
#define SM_RECORD_PATH    (1<< 1) /* VS1103 */
#define SM_RESET          (1<< 2)
#define SM_CANCEL         (1<< 3) /* VS1063, VS1053 */
#define SM_OUTOFWAV       (1<< 3) /* VS1033, VS1003, VS1011 */
#define SM_OUTOFMIDI      (1<< 3) /* VS1103 */
#define SM_EARSPEAKER_LO  (1<< 4) /* VS1053, VS1033 */
#define SM_PDOWN          (1<< 4) /* VS1003, VS1103 */
#define SM_TESTS          (1<< 5)
#define SM_STREAM         (1<< 6) /* VS1053, VS1033, VS1003, VS1011 */
#define SM_ICONF          (1<< 6) /* VS1103 */
#define SM_EARSPEAKER_HI  (1<< 7) /* VS1053, VS1033 */
#define SM_DACT           (1<< 8)
#define SM_SDIORD         (1<< 9)
#define SM_SDISHARE       (1<<10)
#define SM_SDINEW         (1<<11)
#define SM_ENCODE         (1<<12) /* VS1063 */
#define SM_ADPCM          (1<<12) /* VS1053, VS1033, VS1003 */
#define SM_EARSPEAKER1103 (1<<12) /* VS1103 */
#define SM_ADPCM_HP       (1<<13) /* VS1033, VS1003 */
#define SM_LINE1          (1<<14) /* VS1063, VS1053 */
#define SM_LINE_IN        (1<<14) /* VS1033, VS1003, VS1103 */
#define SM_CLK_RANGE      (1<<15) /* VS1063, VS1053, VS1033 */
#define SM_ADPCM_1103     (1<<15) /* VS1103 */

#define SM_ICONF_BITS 2
#define SM_ICONF_MASK 0x00c0

#define SM_EARSPEAKER_1103_BITS 2
#define SM_EARSPEAKER_1103_MASK 0x3000


/* SCI_STATUS bits */

#define SS_REFERENCE_SEL_B  0 /* VS1063, VS1053 */
#define SS_AVOL_B           0 /* VS1033, VS1003, VS1103, VS1011 */
#define SS_AD_CLOCK_B       1 /* VS1063, VS1053 */
#define SS_APDOWN1_B        2
#define SS_APDOWN2_B        3
#define SS_VER_B            4
#define SS_VCM_DISABLE_B   10 /* VS1063, VS1053 */
#define SS_VCM_OVERLOAD_B  11 /* VS1063, VS1053 */
#define SS_SWING_B         12 /* VS1063, VS1053 */
#define SS_DO_NOT_JUMP_B   15 /* VS1063, VS1053 */

#define SS_REFERENCE_SEL (1<< 0) /* VS1063, VS1053 */
#define SS_AVOL          (1<< 0) /* VS1033, VS1003, VS1103, VS1011 */
#define SS_AD_CLOCK      (1<< 1) /* VS1063, VS1053 */
#define SS_APDOWN1       (1<< 2)
#define SS_APDOWN2       (1<< 3)
#define SS_VER           (1<< 4)
#define SS_VCM_DISABLE   (1<<10) /* VS1063, VS1053 */
#define SS_VCM_OVERLOAD  (1<<11) /* VS1063, VS1053 */
#define SS_SWING         (1<<12) /* VS1063, VS1053 */
#define SS_DO_NOT_JUMP   (1<<15) /* VS1063, VS1053 */

#define SS_SWING_BITS     3
#define SS_SWING_MASK     0x7000
#define SS_VER_BITS       4
#define SS_VER_MASK       0x00f0
#define SS_AVOL_BITS      2
#define SS_AVOL_MASK      0x0003

#define SS_VER_VS1001 0x00
#define SS_VER_VS1011 0x10
#define SS_VER_VS1002 0x20
#define SS_VER_VS1003 0x30
#define SS_VER_VS1053 0x40
#define SS_VER_VS8053 0x40
#define SS_VER_VS1033 0x50
#define SS_VER_VS1063 0x60
#define SS_VER_VS1103 0x70


/* SCI_BASS bits */

#define ST_AMPLITUDE_B 12
#define ST_FREQLIMIT_B  8
#define SB_AMPLITUDE_B  4
#define SB_FREQLIMIT_B  0

#define ST_AMPLITUDE (1<<12)
#define ST_FREQLIMIT (1<< 8)
#define SB_AMPLITUDE (1<< 4)
#define SB_FREQLIMIT (1<< 0)

#define ST_AMPLITUDE_BITS 4
#define ST_AMPLITUDE_MASK 0xf000
#define ST_FREQLIMIT_BITS 4
#define ST_FREQLIMIT_MASK 0x0f00
#define SB_AMPLITUDE_BITS 4
#define SB_AMPLITUDE_MASK 0x00f0
#define SB_FREQLIMIT_BITS 4
#define SB_FREQLIMIT_MASK 0x000f


/* SCI_CLOCKF bits */

#define SC_MULT_B 13 /* VS1063, VS1053, VS1033, VS1103, VS1003 */
#define SC_ADD_B  11 /* VS1063, VS1053, VS1033, VS1003 */
#define SC_FREQ_B  0 /* VS1063, VS1053, VS1033, VS1103, VS1003 */

#define SC_MULT (1<<13) /* VS1063, VS1053, VS1033, VS1103, VS1003 */
#define SC_ADD  (1<<11) /* VS1063, VS1053, VS1033, VS1003 */
#define SC_FREQ (1<< 0) /* VS1063, VS1053, VS1033, VS1103, VS1003 */

#define SC_MULT_BITS 3
#define SC_MULT_MASK 0xe000
#define SC_ADD_BITS 2
#define SC_ADD_MASK  0x1800
#define SC_FREQ_BITS 11
#define SC_FREQ_MASK 0x07ff

/* The following macro is for VS1063, VS1053, VS1033, VS1003, VS1103.
   Divide hz by two when calling if SM_CLK_RANGE = 1 */
#define HZ_TO_SC_FREQ(hz) (((hz)-8000000+2000)/4000)

/* The following macro is for VS1011.
   The macro will automatically set the clock doubler if XTALI < 16 MHz */
#define HZ_TO_SCI_CLOCKF(hz) ((((hz)<16000000)?0x8000:0)+((hz)+1000)/2000)

/* Following are for VS1003 and VS1033 */
#define SC_MULT_03_10X 0x0000
#define SC_MULT_03_15X 0x2000
#define SC_MULT_03_20X 0x4000
#define SC_MULT_03_25X 0x6000
#define SC_MULT_03_30X 0x8000
#define SC_MULT_03_35X 0xa000
#define SC_MULT_03_40X 0xc000
#define SC_MULT_03_45X 0xe000

/* Following are for VS1053 and VS1063 */
#define SC_MULT_53_10X 0x0000
#define SC_MULT_53_20X 0x2000
#define SC_MULT_53_25X 0x4000
#define SC_MULT_53_30X 0x6000
#define SC_MULT_53_35X 0x8000
#define SC_MULT_53_40X 0xa000
#define SC_MULT_53_45X 0xc000
#define SC_MULT_53_50X 0xe000

/* Following are for VS1003 and VS1033 */
#define SC_ADD_03_00X 0x0000
#define SC_ADD_03_05X 0x0800
#define SC_ADD_03_10X 0x1000
#define SC_ADD_03_15X 0x1800

/* Following are for VS1053 and VS1063 */
#define SC_ADD_53_00X 0x0000
#define SC_ADD_53_10X 0x0800
#define SC_ADD_53_15X 0x1000
#define SC_ADD_53_20X 0x1800


/* SCI_WRAMADDR bits */

#define SCI_WRAM_X_START           0x0000
#define SCI_WRAM_Y_START           0x4000
#define SCI_WRAM_I_START           0x8000
#define SCI_WRAM_IO_START          0xC000
#define SCI_WRAM_PARAMETRIC_START  0xC0C0 /* VS1063 */
#define SCI_WRAM_Y2_START          0xE000 /* VS1063 */

#define SCI_WRAM_X_OFFSET  0x0000
#define SCI_WRAM_Y_OFFSET  0x4000
#define SCI_WRAM_I_OFFSET  0x8000
#define SCI_WRAM_IO_OFFSET 0x0000 /* I/O addresses are @0xC000 -> no offset */
#define SCI_WRAM_PARAMETRIC_OFFSET (0xC0C0-0x1E00) /* VS1063 */
#define SCI_WRAM_Y2_OFFSET 0x0000                  /* VS1063 */


/* SCI_VOL bits */

#define SV_LEFT_B  8
#define SV_RIGHT_B 0

#define SV_LEFT  (1<<8)
#define SV_RIGHT (1<<0)

#define SV_LEFT_BITS  8
#define SV_LEFT_MASK  0xFF00
#define SV_RIGHT_BITS 8
#define SV_RIGHT_MASK 0x00FF


/*�SCI_MIXERVOL bits for VS1103 */

#define SMV_ACTIVE_B 15
#define SMV_GAIN3_B  10
#define SMV_GAIN2_B   5
#define SMV_GAIN1_B   0

#define SMV_ACTIVE (1<<15)
#define SMV_GAIN3  (1<<10)
#define SMV_GAIN2  (1<< 5)
#define SMV_GAIN1  (1<< 0)

#define SMV_GAIN3_BITS 5
#define SMV_GAIN3_MASK 0x7c00
#define SMV_GAIN2_BITS 5
#define SMV_GAIN2_MASK 0x04e0
#define SMV_GAIN1_BITS 5
#define SMV_GAIN1_MASK 0x001f


/*�SCI_ADPCMRECCTL bits for VS1103 */

#define SARC_DREQ512_B    8
#define SARC_OUTODADPCM_B 7
#define SARC_MANUALGAIN_B 6
#define SARC_GAIN4_B      0

#define SARC_DREQ512    (1<<8)
#define SARC_OUTODADPCM (1<<7)
#define SARC_MANUALGAIN (1<<6)
#define SARC_GAIN4      (1<<0)

#define SARC_GAIN4_BITS 6
#define SARC_GAIN4_MASK 0x003f


/* SCI_RECQUALITY bits for VS1063 */

#define RQ_MODE_B                   14
#define RQ_MULT_B                   12
#define RQ_OGG_PAR_SERIAL_NUMBER_B  11
#define RQ_OGG_LIMIT_FRAME_LENGTH_B 10
#define RQ_MP3_NO_BIT_RESERVOIR_B   10
#define RQ_BITRATE_BASE_B            0

#define RQ_MODE                   (1<<14)
#define RQ_MULT                   (1<<12)
#define RQ_OGG_PAR_SERIAL_NUMBER  (1<<11)
#define RQ_OGG_LIMIT_FRAME_LENGTH (1<<10)
#define RQ_MP3_NO_BIT_RESERVOIR   (1<<10)
#define RQ_BITRATE_BASE           (1<< 0)

#define RQ_MODE_BITS 2
#define RQ_MODE_MASK 0xc000
#define RQ_MULT_BITS 2
#define RQ_MULT_MASK 0x3000
#define RQ_BITRATE_BASE_BITS 9
#define RQ_BITRATE_BASE_MASK 0x01ff

#define RQ_MODE_QUALITY 0x0000
#define RQ_MODE_VBR     0x4000
#define RQ_MODE_ABR     0x8000
#define RQ_MODE_CBR     0xc000

#define RQ_MULT_10      0x0000
#define RQ_MULT_100     0x1000
#define RQ_MULT_1000    0x2000
#define RQ_MULT_10000   0x3000


/* SCI_RECMODE bits for VS1063 */

#define RM_63_CODEC_B    15
#define RM_63_AEC_B      14
#define RM_63_UART_TX_B  13
#define RM_63_PAUSE_B    11
#define RM_63_NO_RIFF_B  10
#define RM_63_FORMAT_B    4
#define RM_63_ADC_MODE_B  0 

#define RM_63_CODEC    (1<<15)
#define RM_63_AEC      (1<<14)
#define RM_63_UART_TX  (1<<13)
#define RM_63_PAUSE    (1<<11)
#define RM_63_NO_RIFF  (1<<10)
#define RM_63_FORMAT   (1<< 4)
#define RM_63_ADC_MODE (1<< 0)

#define RM_63_FORMAT_BITS       4
#define RM_63_FORMAT_MASK  0x00f0
#define RM_63_ADCMODE_BITS      3
#define RM_63_ADCMODE_MASK 0x0007

#define RM_63_FORMAT_IMA_ADPCM  0x0000
#define RM_63_FORMAT_PCM        0x0010
#define RM_63_FORMAT_G711_ULAW  0x0020
#define RM_63_FORMAT_G711_ALAW  0x0030
#define RM_63_FORMAT_G722_ADPCM 0x0040
#define RM_63_FORMAT_OGG_VORBIS 0x0050
#define RM_63_FORMAT_MP3        0x0060

#define RM_63_ADC_MODE_JOINT_AGC_STEREO 0x0000
#define RM_63_ADC_MODE_DUAL_AGC_STEREO  0x0001
#define RM_63_ADC_MODE_LEFT             0x0002
#define RM_63_ADC_MODE_RIGHT            0x0003
#define RM_63_ADC_MODE_MONO             0x0004


/* SCI_RECMODE bits for VS1053 */

#define RM_53_FORMAT_B    2
#define RM_53_ADC_MODE_B  0 

#define RM_53_FORMAT   (1<< 2)
#define RM_53_ADC_MODE (1<< 0)

#define RM_53_ADCMODE_BITS      2
#define RM_53_ADCMODE_MASK 0x0003

#define RM_53_FORMAT_IMA_ADPCM  0x0000
#define RM_53_FORMAT_PCM        0x0004

#define RM_53_ADC_MODE_JOINT_AGC_STEREO 0x0000
#define RM_53_ADC_MODE_DUAL_AGC_STEREO  0x0001
#define RM_53_ADC_MODE_LEFT             0x0002
#define RM_53_ADC_MODE_RIGHT            0x0003


/* VS1063 definitions */

/* VS1063 / VS1053 Parametric */
#define PAR_CHIP_ID                  0x1e00 /* VS1063, VS1053, 32 bits */
#define PAR_VERSION                  0x1e02 /* VS1063, VS1053 */
#define PAR_CONFIG1                  0x1e03 /* VS1063, VS1053 */
#define PAR_PLAY_SPEED               0x1e04 /* VS1063, VS1053 */
#define PAR_BITRATE_PER_100          0x1e05 /* VS1063 */
#define PAR_BYTERATE                 0x1e05 /* VS1053 */
#define PAR_END_FILL_BYTE            0x1e06 /* VS1063, VS1053 */
#define PAR_RATE_TUNE                0x1e07 /* VS1063,         32 bits */
#define PAR_PLAY_MODE                0x1e09 /* VS1063 */
#define PAR_SAMPLE_COUNTER           0x1e0a /* VS1063,         32 bits */
#define PAR_VU_METER                 0x1e0c /* VS1063 */
#define PAR_AD_MIXER_GAIN            0x1e0d /* VS1063 */
#define PAR_AD_MIXER_CONFIG          0x1e0e /* VS1063 */
#define PAR_PCM_MIXER_RATE           0x1e0f /* VS1063 */
#define PAR_PCM_MIXER_FREE           0x1e10 /* VS1063 */
#define PAR_PCM_MIXER_VOL            0x1e11 /* VS1063 */
#define PAR_EQ5_DUMMY                0x1e12 /* VS1063 */
#define PAR_EQ5_LEVEL1               0x1e13 /* VS1063 */
#define PAR_EQ5_FREQ1                0x1e14 /* VS1063 */
#define PAR_EQ5_LEVEL2               0x1e15 /* VS1063 */
#define PAR_EQ5_FREQ2                0x1e16 /* VS1063 */
#define PAR_JUMP_POINTS              0x1e16 /*         VS1053 */
#define PAR_EQ5_LEVEL3               0x1e17 /* VS1063 */
#define PAR_EQ5_FREQ3                0x1e18 /* VS1063 */
#define PAR_EQ5_LEVEL4               0x1e19 /* VS1063 */
#define PAR_EQ5_FREQ4                0x1e1a /* VS1063 */
#define PAR_EQ5_LEVEL5               0x1e1b /* VS1063 */
#define PAR_EQ5_UPDATED              0x1e1c /* VS1063 */
#define PAR_SPEED_SHIFTER            0x1e1d /* VS1063 */
#define PAR_EARSPEAKER_LEVEL         0x1e1e /* VS1063 */
#define PAR_SDI_FREE                 0x1e1f /* VS1063 */
#define PAR_AUDIO_FILL               0x1e20 /* VS1063 */
#define PAR_RESERVED0                0x1e21 /* VS1063 */
#define PAR_RESERVED1                0x1e22 /* VS1063 */
#define PAR_RESERVED2                0x1e23 /* VS1063 */
#define PAR_RESERVED3                0x1e24 /* VS1063 */
#define PAR_LATEST_SOF               0x1e25 /* VS1063,         32 bits */
#define PAR_LATEST_JUMP              0x1e26 /*         VS1053 */
#define PAR_POSITION_MSEC            0x1e27 /* VS1063, VS1053, 32 bits */
#define PAR_RESYNC                   0x1e29 /* VS1063, VS1053 */

/* The following addresses are shared between modes. */
/* Generic pointer */
#define PAR_GENERIC                  0x1e2a /* VS1063, VS1053 */

/* Encoder mode */
#define PAR_ENC_TX_UART_DIV          0x1e2a /* VS1063 */
#define PAR_ENC_TX_UART_BYTE_SPEED   0x1e2b /* VS1063 */
#define PAR_ENC_TX_PAUSE_GPIO        0x1e2c /* VS1063 */
#define PAR_ENC_AEC_ADAPT_MULTIPLIER 0x1e2d /* VS1063 */
#define PAR_ENC_RESERVED             0x1e2e /* VS1063 */
#define PAR_ENC_CHANNEL_MAX          0x1e3c /* VS1063 */
#define PAR_ENC_SERIAL_NUMBER        0x1e3e /* VS1063 */

/* Decoding WMA */
#define PAR_WMA_CUR_PACKET_SIZE      0x1e2a /* VS1063, VS1053, 32 bits */
#define PAR_WMA_PACKET_SIZE          0x1e2c /* VS1063, VS1053, 32 bits */

/* Decoding AAC */
#define PAR_AAC_SCE_FOUND_MASK       0x1e2a /* VS1063, VS1053 */
#define PAR_AAC_CPE_FOUND_MASK       0x1e2b /* VS1063, VS1053 */
#define PAR_AAC_LFE_FOUND_MASK       0x1e2c /* VS1063, VS1053 */
#define PAR_AAC_PLAY_SELECT          0x1e2d /* VS1063, VS1053 */
#define PAR_AAC_DYN_COMPRESS         0x1e2e /* VS1063, VS1053 */
#define PAR_AAC_DYN_BOOST            0x1e2f /* VS1063, VS1053 */
#define PAR_AAC_SBR_AND_PS_STATUS    0x1e30 /* VS1063, VS1053 */
#define PAR_AAC_SBR_PS_FLAGS         0x1e31 /* VS1063 */


/* Decoding MIDI (VS1053) */
#define PAR_MIDI_BYTES_LEFT          0x1e2a /*         VS1053, 32 bits */

/* Decoding Vorbis */
#define PAR_VORBIS_GAIN 0x1e2a       0x1e30 /* VS1063, VS1053 */


/* Bit definitions for parametric registers with bitfields */
#define PAR_CONFIG1_DIS_WMA_B     15 /* VS1063 */
#define PAR_CONFIG1_DIS_AAC_B     14 /* VS1063 */
#define PAR_CONFIG1_DIS_MP3_B     13 /* VS1063 */
#define PAR_CONFIG1_DIS_FLAC_B    12 /* VS1063 */
#define PAR_CONFIG1_DIS_CRC_B      8 /* VS1063 */
#define PAR_CONFIG1_AAC_PS_B       6 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR_B      4 /* VS1063, VS1053 */
#define PAR_CONFIG1_MIDI_REVERB_B  0 /*         VS1053 */

#define PAR_CONFIG1_DIS_WMA     (1<<15) /* VS1063 */
#define PAR_CONFIG1_DIS_AAC     (1<<14) /* VS1063 */
#define PAR_CONFIG1_DIS_MP3     (1<<13) /* VS1063 */
#define PAR_CONFIG1_DIS_FLAC    (1<<12) /* VS1063 */
#define PAR_CONFIG1_DIS_CRC     (1<< 8) /* VS1063 */
#define PAR_CONFIG1_AAC_PS      (1<< 6) /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR     (1<< 4) /* VS1063, VS1053 */
#define PAR_CONFIG1_MIDI_REVERB (1<< 0) /*         VS1053 */

#define PAR_CONFIG1_AAC_PS_BITS  2      /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_PS_MASK  0x00c0 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR_BITS 2      /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR_MASK 0x0030 /* VS1063, VS1053 */

#define PAR_CONFIG1_AAC_SBR_ALWAYS_UPSAMPLE    0x0000 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR_SELECTIVE_UPSAMPLE 0x0010 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR_NEVER_UPSAMPLE     0x0020 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_SBR_DISABLE            0x0030 /* VS1063, VS1053 */

#define PAR_CONFIG1_AAC_PS_NORMAL              0x0000 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_PS_DOWNSAMPLED         0x0040 /* VS1063, VS1053 */
#define PAR_CONFIG1_AAC_PS_DISABLE             0x00c0 /* VS1063, VS1053 */

#define PAR_PLAY_MODE_SPEED_SHIFTER_ENA_B 6 /* VS1063 */
#define PAR_PLAY_MODE_EQ5_ENA_B           5 /* VS1063 */
#define PAR_PLAY_MODE_PCM_MIXER_ENA_B     4 /* VS1063 */
#define PAR_PLAY_MODE_AD_MIXER_ENA_B      3 /* VS1063 */
#define PAR_PLAY_MODE_VU_METER_ENA_B      2 /* VS1063 */
#define PAR_PLAY_MODE_PAUSE_ENA_B         1 /* VS1063 */
#define PAR_PLAY_MODE_MONO_ENA_B          0 /* VS1063 */

#define PAR_PLAY_MODE_SPEED_SHIFTER_ENA (1<<6) /* VS1063 */
#define PAR_PLAY_MODE_EQ5_ENA           (1<<5) /* VS1063 */
#define PAR_PLAY_MODE_PCM_MIXER_ENA     (1<<4) /* VS1063 */
#define PAR_PLAY_MODE_AD_MIXER_ENA      (1<<3) /* VS1063 */
#define PAR_PLAY_MODE_VU_METER_ENA      (1<<2) /* VS1063 */
#define PAR_PLAY_MODE_PAUSE_ENA         (1<<1) /* VS1063 */
#define PAR_PLAY_MODE_MONO_ENA          (1<<0) /* VS1063 */

#define PAR_VU_METER_LEFT_BITS  8      /* VS1063 */
#define PAR_VU_METER_LEFT_MASK  0xFF00 /* VS1063 */
#define PAR_VU_METER_RIGHT_BITS 8      /* VS1063 */
#define PAR_VU_METER_RIGHT_MASK 0x00FF /* VS1063 */

#define PAR_AD_MIXER_CONFIG_MODE_B 2 /* VS1063 */
#define PAR_AD_MIXER_CONFIG_RATE_B 2 /* VS1063 */

#define PAR_AD_MIXER_CONFIG_MODE_BITS 2      /* VS1063 */
#define PAR_AD_MIXER_CONFIG_MODE_MASK 0x000c /* VS1063 */
#define PAR_AD_MIXER_CONFIG_RATE_BITS 2      /* VS1063 */
#define PAR_AD_MIXER_CONFIG_RATE_MASK 0x0003 /* VS1063 */

#define PAR_AD_MIXER_CONFIG_RATE_192K 0x0000 /* VS1063 */
#define PAR_AD_MIXER_CONFIG_RATE_96K  0x0001 /* VS1063 */
#define PAR_AD_MIXER_CONFIG_RATE_48K  0x0002 /* VS1063 */
#define PAR_AD_MIXER_CONFIG_RATE_24K  0x0003 /* VS1063 */

#define PAR_AD_MIXER_CONFIG_MODE_STEREO 0x0000 /* VS1063 */
#define PAR_AD_MIXER_CONFIG_MODE_MONO   0x0040 /* VS1063 */
#define PAR_AD_MIXER_CONFIG_MODE_LEFT   0x0080 /* VS1063 */
#define PAR_AD_MIXER_CONFIG_MODE_RIGHT  0x00c0 /* VS1063 */

#define PAR_AAC_SBR_AND_PS_STATUS_SBR_PRESENT_B       0 /* VS1063, VS1053 */
#define PAR_AAC_SBR_AND_PS_STATUS_UPSAMPLING_ACTIVE_B 1 /* VS1063, VS1053 */
#define PAR_AAC_SBR_AND_PS_STATUS_PS_PRESENT_B        2 /* VS1063, VS1053 */
#define PAR_AAC_SBR_AND_PS_STATUS_PS_ACTIVE_B         3 /* VS1063, VS1053 */

#define PAR_AAC_SBR_AND_PS_STATUS_SBR_PRESENT       (1<<0) /* VS1063, VS1053 */
#define PAR_AAC_SBR_AND_PS_STATUS_UPSAMPLING_ACTIVE (1<<1) /* VS1063, VS1053 */
#define PAR_AAC_SBR_AND_PS_STATUS_PS_PRESENT        (1<<2) /* VS1063, VS1053 */
#define PAR_AAC_SBR_AND_PS_STATUS_PS_ACTIVE         (1<<3) /* VS1063, VS1053 */



//////////////////////////////////////////////////////////////
																	
 //RIFF��
typedef	struct
{
	u32_t ChunkID;				//chunk id;����̶�Ϊ"RIFF",��0X46464952
	u32_t ChunkSize ;				//���ϴ�С;�ļ��ܴ�С-8
	u32_t Format;					//��ʽ;WAVE,��0X45564157
}__attribute__ ((packed)) ChunkRIFF ;


//fmt��
typedef struct
{
	u32_t ChunkID;				//chunk id;����̶�Ϊ"fmt ",��0X20746D66
	u32_t ChunkSize ;				//�Ӽ��ϴ�С(������ID��Size);����Ϊ:20.
	u16_t AudioFormat;			//��Ƶ��ʽ;0X10,��ʾ����PCM;0X11��ʾIMA ADPCM
	u16_t NumOfChannels;		//ͨ������;1,��ʾ������;2,��ʾ˫����;
	u32_t SampleRate;			//������;0X1F40,��ʾ8Khz
	u32_t ByteRate;			//�ֽ�����; 
	u16_t BlockAlign;			//�����(�ֽ�); 
	u16_t BitsPerSample;		//�����������ݴ�С;4λADPCM,����Ϊ4
//	u16_t ByteExtraData;		//���ӵ������ֽ�;2��; ����PCM,û���������
//	u16_t ExtraData;			//���ӵ�����,�����������ݿ��С;0X1F9:505�ֽ�	����PCM,û���������
}__attribute__ ((packed)) ChunkFMT;


//fact�� 
typedef struct 
{
	u32_t ChunkID;				//chunk id;����̶�Ϊ"fact",��0X74636166;
	u32_t ChunkSize ;			//�Ӽ��ϴ�С(������ID��Size);����Ϊ:4.
	u32_t NumOfSamples;			//����������; 
}__attribute__ ((packed)) ChunkFACT;


//data�� 
typedef struct 
{
	u32_t ChunkID;				//chunk id;����̶�Ϊ"data",��0X61746164
	u32_t ChunkSize ;			//�Ӽ��ϴ�С(������ID��Size);�ļ���С-60.
}__attribute__ ((packed)) ChunkDATA;



//wavͷ
typedef struct
{ 
	ChunkRIFF riff;	//riff��
	ChunkFMT fmt;		//fmt��
	//ChunkFACT fact;	//fact�� ����PCM,û������ṹ��	
	ChunkDATA data;	//data��		
}__attribute__ ((packed)) wave_header_t; 











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
	temp=VS_SPI_ReadWriteByte(0xff);		//��ȡ���ֽ�
	temp=temp<<8;
	temp+=VS_SPI_ReadWriteByte(0xff);		//��ȡ���ֽ�
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





////////////////////////////////////////////////////////////////////////////////		
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





//����PCM ¼��ģʽ
//agc:0,�Զ�����.1024�൱��1��,512�൱��0.5��,���ֵ65535=64��			
void recoder_enter_rec_mode(u16_t agc)
{
	//�����IMA ADPCM,�����ʼ��㹫ʽ����:
	//������=CLKI/256*d;	
	//����d=0,��2��Ƶ,�ⲿ����Ϊ12.288M.��ôFc=(2*12288000)/256*6=16Khz
	//���������PCM,������ֱ�Ӿ�д����ֵ 
	VS_WR_Cmd(SPI_BASS, 0x0000);
	VS_WR_Cmd(SPI_AICTRL0, 8000);	//���ò�����,����Ϊ8Khz
	VS_WR_Cmd(SPI_AICTRL1, agc);		//��������,0,�Զ�����.1024�൱��1��,512�൱��0.5��,���ֵ65535=64��	
	VS_WR_Cmd(SPI_AICTRL2, 0);		//�����������ֵ,0,�������ֵ65536=64X
	VS_WR_Cmd(SPI_AICTRL3, 6);		//��ͨ��(MIC����������)
	VS_WR_Cmd(SPI_CLOCKF, 0X2000);	//����VS10XX��ʱ��,MULT:2��Ƶ;ADD:������;CLK:12.288Mhz
	VS_WR_Cmd(SPI_MODE, 0x1804);		//MIC,¼������
	_tick_sleep(5);					//�ȴ�����1.35ms 
//	VS_Load_Patch((u16_t*)wav_plugin,40);//VS1053��WAV¼����Ҫpatch



}










u8_t adpcmHeader[60] = {
  'R', 'I', 'F', 'F',
  0xFF, 0xFF, 0xFF, 0xFF,
  'W', 'A', 'V', 'E',
  'f', 'm', 't', ' ',
  0x14, 0, 0, 0,          /* 20 */
  0x11, 0,                /* IMA ADPCM */
  0x1, 0,                 /* chan */
  0x0, 0x0, 0x0, 0x0,     /* sampleRate */
  0x0, 0x0, 0x0, 0x0,     /* byteRate */
  0, 1,                   /* blockAlign */
  4, 0,                   /* bitsPerSample */
  2, 0,                   /* byteExtraData */
  0xf9, 0x1,              /* samplesPerBlock = 505 */
  'f', 'a', 'c', 't',     /* subChunk2Id */
  0x4, 0, 0, 0,           /* subChunk2Size */
  0xFF, 0xFF, 0xFF, 0xFF, /* numOfSamples */
  'd', 'a', 't', 'a',
  0xFF, 0xFF, 0xFF, 0xFF
};

void Set32(u8_t *d, u32_t n) {
  int i;
  for (i=0; i<4; i++) {
    *d++ = (u8_t)n;
    n >>= 8;
  }
}

void Set16(u8_t *d, u16_t n) {
  int i;
  for (i=0; i<2; i++) {
    *d++ = (u8_t)n;
    n >>= 8;
  }
}


void adpcm_record_mode(void)
{
  u16_t sampleRate, divReg;

  /* Set clock to a known, high value. */
  VS_WR_Cmd(SCI_CLOCKF, HZ_TO_SC_FREQ(12288000) | SC_MULT_03_40X | SC_ADD_03_00X);

  // Voice quality ADPCM recording at 8 kHz. This will result in a 32.44 kbit/s bitstream.
  sampleRate = 8000;
  // Calculate closest possible value for divider register
  divReg = (int)((4.0*12288000)/256/sampleRate+0.5);
  // Calculate back what sample rate we actually got
  sampleRate = 4*12288000/(256*divReg);

  VS_WR_Cmd(SCI_RECRATE,   divReg);
  VS_WR_Cmd(SCI_RECGAIN,   0); // 1024 = gain 1, 0 = AGC


  Set32(adpcmHeader+24, sampleRate);
  Set32(adpcmHeader+28, (u32_t)sampleRate*256/505);

  // Start the encoder
  VS_WR_Cmd(SCI_MODE, VS_RD_Reg(SCI_MODE) | SM_ADPCM | SM_ADPCM_HP | SM_RESET);//

}



void VS_Record_adpcm(void)
{
	int res, lasttime; 
	int fileSize = 0, adpcmBlocks = 0;
	FIL fobj;
	UINT bw;


	LOG_INFO("Enter record mode.\r\n");

	adpcm_record_mode();				

	//open and write header
	res = api_fs.f_open(&fobj,"/app/vs1003/rec.wav", FA_CREATE_ALWAYS | FA_WRITE);
	res = api_fs.f_write(&fobj,(const void*)adpcmHeader,sizeof(adpcmHeader),&bw);//д��ͷ����
	fileSize = sizeof(adpcmHeader);


	lasttime = api_os.tick_now();

	//¼��10s
	while(api_os.tick_elapsed(lasttime) < 10000){
		u8_t recbuf[512];

		if (VS_RD_Reg(SCI_RECWORDS) > 512) {
			int i;
			
			adpcmBlocks += 2;	// Always writes one or two IMA ADPCM block(s) at a time
			for (i=0; i<256; i++) {
				u16_t w = VS_RD_Reg(SCI_RECDATA);
				recbuf[i*2] = (u8_t)(w >> 8);
				recbuf[i*2+1] = (u8_t)(w & 0xFF);
			}

			if(FR_OK != (res = api_fs.f_write(&fobj, recbuf, 512,&bw)) || bw != 512){
				LOG_WARN("err:%d bw:%d\r\n",res,bw);
				break;	//д�����.		
			}

			fileSize += 512;
		}

	}

	//ʱ�䵽 ���� д�����
//	api_console.printf("\r\n");

	Set32(adpcmHeader+4, adpcmBlocks*256 + sizeof(adpcmHeader) - 8);
	Set32(adpcmHeader+48, adpcmBlocks*505);
	Set32(adpcmHeader+56, adpcmBlocks*256);


	LOG_INFO("block:%u\r\n", adpcmBlocks);

	api_fs.f_lseek(&fobj, 0);							//ƫ�Ƶ��ļ�ͷ.
	res = api_fs.f_write(&fobj,(const void*)adpcmHeader,sizeof(adpcmHeader),&bw);//д��ͷ����
	api_fs.f_close(&fobj);

}


//��ʼ��WAVͷ.
void recoder_wav_init(wave_header_t* wavhead) //��ʼ��WAVͷ				
{
	wavhead->riff.ChunkID=0X46464952;	//"RIFF"
	wavhead->riff.ChunkSize=0;			//��δȷ��,�����Ҫ����
	wavhead->riff.Format=0X45564157;	//"WAVE"
	wavhead->fmt.ChunkID=0X20746D66;	//"fmt "
	wavhead->fmt.ChunkSize=16;			//��СΪ16���ֽ�
	wavhead->fmt.AudioFormat=0X01;		//0X01,��ʾPCM;0X11,��ʾIMA ADPCM
	wavhead->fmt.NumOfChannels=1;		//������
	wavhead->fmt.SampleRate=8000;		//8Khz������ ��������
	wavhead->fmt.ByteRate=wavhead->fmt.SampleRate*2;//16λ,��2���ֽ�
	wavhead->fmt.BlockAlign=2;			//���С,2���ֽ�Ϊһ����
	wavhead->fmt.BitsPerSample=16;		//16λPCM
	wavhead->data.ChunkID=0X61746164;	//"data"
	wavhead->data.ChunkSize=0;			//���ݴ�С,����Ҫ����	
}


void VS_Record_pcm(void)
{
	int res, lasttime, n;
	FIL fobj;
	UINT bw;
	wave_header_t wavhead;


	recoder_enter_rec_mode(1024*0);				
	while(VS_RD_Reg(SPI_HDAT1)>>8);			//�ȵ�buf��Ϊ�����ٿ�ʼ

	LOG_INFO("Enter record mode.\r\n");

	recoder_wav_init(&wavhead);	//��ʼ��wav����

	res = api_fs.f_open(&fobj,"/app/vs1003/rec.wav", FA_CREATE_ALWAYS | FA_WRITE);
	res = api_fs.f_write(&fobj,(const void*)&wavhead,sizeof(wave_header_t),&bw);//д��ͷ����

	lasttime = api_os.tick_now();
	n = 0;

	//¼��10s
	while(api_os.tick_elapsed(lasttime) < 5000){
		u8_t recbuf[512];
		u16_t w = VS_RD_Reg(SPI_HDAT1);		//����512bytes


		if(w < 256 || w >= 896){
			_tick_sleep(1);
			api_console.printf("%d ",w);
		}

		else{
			int i = 0;

			while(i < 512){	//һ�ζ�ȡ512�ֽ�
				w = VS_RD_Reg(SPI_HDAT0);						
				recbuf[i++] = w&0XFF;
				recbuf[i++] = w>>8;
			}

			res = api_fs.f_write(&fobj, recbuf, 512,&bw);//д��512�ֽ�
			if(res){
				LOG_WARN("err:%d bw:%d\r\n",res,bw);
				break;	//д�����.		
			}
			n++;

			api_console.printf("~");
		}



	}

	//ʱ�䵽 ���� д�����
	api_console.printf("\r\n");

	wavhead.riff.ChunkSize = n*512+36;	//�����ļ��Ĵ�С-8;
	wavhead.data.ChunkSize = n*512;		//���ݴ�С

	LOG_INFO("block:%u\r\n", n);

	api_fs.f_lseek(&fobj, 0);							//ƫ�Ƶ��ļ�ͷ.
	api_fs.f_write(&fobj, (const void*)&wavhead,sizeof(wave_header_t),&bw);//д��ͷ����
	api_fs.f_close(&fobj);

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

	{				

		VS_HD_Reset();

		LOG_INFO("Wav recording...\r\n");		

		VS_Record_adpcm();
		//VS_Record();

	}



//	return ADDON_LOADER_GRANTED;
	return ADDON_LOADER_ABORT;


}



