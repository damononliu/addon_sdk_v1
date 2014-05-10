/**
 * @file			aes_test.c
 * @brief			AES���ܲ��Գ���
 *	���ļ�����AES���ܣ����ܽ�����Աȶ�openssl���ܽ��
 *	openssl.exe enc -aes-128-cbc -in file_in -K 0123456789ABCDEF0123456789ABCDEF -iv FEDCBA9876543210FEDCBA9876543210 -out file_out
 * @author			yizuoshe@gmail.com
*/


#include "include.h"






size_t PKCS7_padding(u8_t* buf, size_t len, u8_t base)
{
	u8_t padding_len;

	//ȡ����ֽ���(������ ��ȡ16)
	padding_len = base - len%base;
	if(0 == padding_len)padding_len = base;

	//�����Ҫ������
	int i;
	for(i = 0; i < padding_len; i++)
		buf[len+i] = padding_len;

	return padding_len;
}




int main(int argc, char* argv[])
{
	aes_context_t aes_ctx;
	u8_t* buf;

	size_t fsize = 0;

	buf = api_fs.read_alloc("/raw", 10240, &fsize, 16);	//���10K ���������16�ֽ�

	if(NULL == buf)
		return ADDON_LOADER_ABORT;

	LOG_INFO("file size:%u.Start encrypt.\r\n", fsize);


	fsize += PKCS7_padding(buf, fsize, 16);		//����16

	u8_t iv[16], key[16];
	utl.axtobuf_detail("0123456789ABCDEF0123456789ABCDEF", 32, key, 16, NULL);
	utl.axtobuf_detail("FEDCBA9876543210FEDCBA9876543210", 32, iv, 16, NULL);
	

	api_crypto.aes_setkey_enc(&aes_ctx, key, 128);
	api_crypto.aes_crypt_cbc(&aes_ctx, AES_ENCRYPT_, fsize, iv, buf, buf);

	LOG_INFO("Saving...\r\n");

	api_fs.write_buf("/raw.aes", buf, fsize);

	_free(buf);

	LOG_INFO("Done...\r\n");

	return ADDON_LOADER_ABORT;	//ɾ����addon (����פ��)
}

