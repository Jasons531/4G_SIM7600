/*
*Function:
*Programed by:Ysheng
*Complete date:
*Modified by:
*Modified date:
*Remarks:
*/

#include "sim7600.h"
#include "usart.h"

#include <stdint.h>
#include <string.h>

commandid_t commandid_reply = REPLY_NONE;

simcom_cmd_t cmds[] = {
	/*{command						len,		timeout,		retry,  expect_reply}*/
	{"AT\r\n",						4,			500,			10,			REPLY_OK},	//���Դ����Ƿ�ͨ
	{"AT+CPIN?\r\n",				10,			500,			20,			REPLY_CPIN},	//��ѯSIM���Ƿ���׼������
	{"AT+CSQ\r\n",					8,			500,			60,			REPLY_CSQ},	//��ѯ�ź�ǿ��
	{"AT+CGREG?\r\n",				11,			500,			60,			REPLY_CGREG},	//��ѯ�����Ƿ����ϣ�������ʼ�����ͨ��
	{"AT+NETOPEN\r\n",				12,			500	,			1,			REPLY_NETOPEN},		//������
	{"AT+CIPCLOSE=0\r\n",			15,			500,			1,			REPLY_CIPCLOSE},		///�ر���������Ҫ��ʱ5S
	{"AT+NETCLOSE\r\n",				13,			500,			1,			REPLY_NETCLOSE},		//�ر�Soucket 10S
	{"AT+CFUN=0\r\n",				11,			5000,			1,			REPLY_INCFUN},  	//�������ģʽ
	{"AT+CFUN=1\r\n",				11,			0,				1,			REPLY_OUTCFUN},  	//�˳�����ģʽ
};


/*InitSimcom����ʼ��SIM7600
*������		  ��
*����ֵ��	  �ɹ�����0�� ʧ�ܷ���1
*/
int8_t InitSimcom(void)
{
	///ע�⣺ģ���ϵ���Ҫ��ʱ�㹻ʱ���ٿ�ʼ��ʼ��
	
	///���ڲ���
	if(REPLY_OK != SimcomExecuteCmd(cmds[AT]))
	{
		Usart1SendData_DMA("Send AT is Error\r\n", strlen("Send AT is Error\r\n"));
		HAL_Delay(5000);
		///ģ��ϵ磬�����ϵ�ʹ��
	}
	
	if(REPLY_CPIN != SimcomExecuteCmd(cmds[CHECK_SIM]))
	{
		HAL_Delay(1000);
		printf("line = %d\r\n",__LINE__);
		
		Usart1SendData_DMA("Send CPIN  is Error Go To CFUN Mode\r\n", strlen("Send CPIN  is Error Go To CFUN Mode\r\n"));
		///��ע����ģʽ����Ҫ����
		SimcomExecuteCmd(cmds[INCFUN]);
		HAL_Delay(6000);
		if(REPLY_OUTCFUN != SimcomExecuteCmd(cmds[OUTCFUN]))
		{
		 ///ģ��ϵ磬�����ϵ�ʹ��

		}
	}
	
	if(REPLY_CSQ != SimcomExecuteCmd(cmds[CSQ]))
	{
		Usart1SendData_DMA("Send CPIN  is Error Go To CFUN Mode\r\n", strlen("Send CPIN  is Error Go To CFUN Mode\r\n"));
		SimcomExecuteCmd(cmds[INCFUN]);
		HAL_Delay(6000);
		if(REPLY_OUTCFUN != SimcomExecuteCmd(cmds[OUTCFUN]))
		{
		 ///ģ��ϵ磬�����ϵ�ʹ��

		}
		///��ȡCSQ�ź�ǿ���ϱ�������
	}
	
	if(REPLY_CGREG != SimcomExecuteCmd(cmds[CGREG]))
	{
		Usart1SendData_DMA("Send CPIN  is Error Go To CFUN Mode\r\n", strlen("Send CPIN  is Error Go To CFUN Mode\r\n"));
		SimcomExecuteCmd(cmds[INCFUN]);
		HAL_Delay(6000);
		if(REPLY_OUTCFUN != SimcomExecuteCmd(cmds[OUTCFUN]))
		{
		 ///ģ��ϵ磬�����ϵ�ʹ��

		}
	}
		
	if(commandid_reply == REPLY_ERROR)
	return INITSIMFAIL;	
	else
	return INITSIMDONE;
}

int8_t SimcomExecuteCmd(simcom_cmd_t cmd)
{	
	commandid_reply = REPLY_NONE;
	
	uint32_t retry_time = 0;
	do
	{
		DEBUG(3,"line = %d\r\n",__LINE__);

		retry_time = HAL_GetTick();
		
		SimcomSendCmd(cmd.data,cmd.len);
		
		while( commandid_reply !=  cmd.expect_retval && commandid_reply != REPLY_ERROR && HAL_GetTick()-retry_time<cmd.timeout);
		
		if( cmd.expect_retval == REPLY_CIPCLOSE)
		HAL_Delay(5000);
		else if(cmd.expect_retval == REPLY_NETCLOSE) ///�ر���������Ҫ��ʱ8S
		HAL_Delay(8000);
		
		if(commandid_reply == cmd.expect_retval)
		{
			DEBUG(3,"----reply----%d\r\n",commandid_reply);

			return commandid_reply;	
		}
				
	}while( --cmd.retry_count>0 );
		
	DEBUG(3,"----reply----%d\r\n",commandid_reply);
	return commandid_reply;
}

/*SimcomOpenNet��������
*������			 ��
*����ֵ��		 ��
*/
void SimcomOpenNet(void)
{
	///ʹ������
	if(REPLY_NETOPEN != SimcomExecuteCmd(cmds[NETOPEN]))
	{
		if(commandid_reply == REPLY_ERROR)
		{
			DEBUG(3,"+IP ERROR: Network is already opened need to close\r\n");
			if(REPLY_NETCLOSE != SimcomExecuteCmd(cmds[NETCLOSE])) 
			{
				DEBUG(2,"NET IS CLOSE\r\n"); 
			}
			HAL_Delay(2000);
			if(REPLY_NETOPEN != SimcomExecuteCmd(cmds[NETOPEN]))
			{
				///�ϵ磬���³�ʼ��
			}
		}
	}
}

void SimcomConnectServer(void)
{
	///������ʧ����ǿ�ƹر����磬�ٴδ����磬���δ�ʧ���������ϵ�
	if(REPLY_CIPOPEN != SimcomSetServer(SERVER_ADDR, SERVER_PORT, 1000, 1))
	{
		if(REPLY_CIPCLOSE != SimcomExecuteCmd(cmds[CIPCLOSE])) 
		{
			DEBUG(2,"CIP IS CLOSE\r\n"); 
		}

		if(REPLY_CIPOPEN != SimcomSetServer(SERVER_ADDR, SERVER_PORT, 1000, 1))
		{
			///ģ��ϵ磬�����ϵ�ʹ��	
		}
	}
}

/*
 *	SimcomConnectServer:ͨ��SIMCOMģ�����ӷ�����
 *	server:				��������ַ��������ip��������
 *	port:				�������˿ڣ�0~65535						
 *	waitMs:				ÿ�ַ��͵ȴ��¼�����λms
 *	retryCount:			����ʧ�����Դ���
 *	����ֵ��			1�ɹ�|0ʧ��	
 */
int8_t SimcomSetServer(char *server, uint16_t port, uint16_t waitMs, int8_t retryCount)
{
	char startConnect[64];
	uint32_t retry_time = 0;

	uint16_t len = sprintf(startConnect,"AT+CIPOPEN=0,\"TCP\",\"%s\",%d\r\n",server,port); ///����0����Ϊ�����0~9
	
	commandid_reply = REPLY_NONE;
	do
	{
		retry_time = HAL_GetTick();
		
		SimcomSendCmd(startConnect,len);
		
	  	while( commandid_reply !=  REPLY_CIPOPEN && commandid_reply != REPLY_ERROR && HAL_GetTick()-retry_time<waitMs);
		if(commandid_reply ==  REPLY_CIPOPEN)
				return commandid_reply;
	}while( --retryCount>0 );

	return commandid_reply;
}


int8_t SimcomSendData(char *buffer, uint16_t waitMs)
{
	char startsend[32];
	
	commandid_reply = REPLY_NONE;
	uint32_t SimcomSendTime = HAL_GetTick();
	
	uint8_t len = sprintf(startsend,"AT+CIPSEND=0,%d\r\n",strlen(buffer));
	SimcomSendCmd(startsend,len);
	while(commandid_reply != REPLY_SEND_START && HAL_GetTick()-SimcomSendTime<100);
	if(commandid_reply == REPLY_ERROR) ///����ʧ�ܣ�������������
	{
		SimcomConnectServer(  );
	}
	
	SimcomSendTime = HAL_GetTick();
	commandid_reply = REPLY_NONE;
	SimcomSendCmd(buffer,strlen(buffer));
	while(commandid_reply != REPLY_SEND_DONE && HAL_GetTick()-SimcomSendTime<waitMs);
	
	return commandid_reply;
}

char uartrx1_buffer[64] = {0};

void UART2_RxDmaCallback(void)
{
	uint8_t tmp_flag = 0;
	uint32_t temp;
			
	tmp_flag =  __HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE);   //�����ж��н������ֽ���ȡ����ֹͣDMA
	if((tmp_flag != RESET))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
		
		HAL_UART_DMAStop(&huart2);
		
		/*       ��ȡDMA��ǰ���ж���û���       */
		temp  = __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
		UART_RX_DATA2.USART_RX_Len =  BUFFER_SIZE - temp; 
		
		char Len = UART_RX_DATA2.USART_RX_Len;
				
		memset(uartrx1_buffer,0, Len);
		memcpy(uartrx1_buffer, UART_RX_DATA2.USART_RX_BUF, Len);
		
		SIMCOM_RECIVE(  ) ;
		
		HAL_UART_Receive_DMA(&huart2,(uint8_t *)UART_RX_DATA2.USART_RX_BUF,BUFFER_SIZE);
		
		for(uint8_t i = 0; i<Len;i++)
			DEBUG(3,"%c",uartrx1_buffer[i]);

		if ( uartrx1_buffer[Len-1]=='>' ) 	// '>'�������ݴ���״̬���������"\r\n"Ϊ��β
		{
			///��ʼ��������
			commandid_reply = REPLY_SEND_START;
		}
		else if ( Len>=4 && uartrx1_buffer[Len-2]=='\r' && uartrx1_buffer[Len-1]=='\n' )	// �յ�"\r\n"����������ظ�
		{
			DEBUG(3,"len : %d\r\n",Len);
			// AT����PING�Ӳ���
			if ( (strncmp(uartrx1_buffer+Len-9, "AT", 2)) == 0 && strncmp(uartrx1_buffer+Len-4, "OK", 2) == 0 ) // ���ص���OK
			{
				commandid_reply = REPLY_OK;
				Usart1SendData_DMA("at\r\n", strlen("at\r\n"));
			}
				
			else if ( Len>=7 && Len<66 && strncmp(uartrx1_buffer+Len-7, "ERROR", 5) == 0 ) // ���ص���ERROR
			{
				commandid_reply = REPLY_ERROR;
			}
				
			///CPIN
			if ( Len>=31 && strncmp(uartrx1_buffer+Len-20, "+CPIN:", 6) == 0 ) 
			{
				if(strstr(uartrx1_buffer, "READY")!=NULL)
				commandid_reply = REPLY_CPIN;
				else
					commandid_reply = REPLY_ERROR;
				
				DEBUG(2,"--- CPIN --- \r\n");
			}
			
			///CSQ
			if ( Len>=27 && strncmp(uartrx1_buffer+Len-19, "+CSQ:", 5) == 0 ) 
			{
				commandid_reply = REPLY_CSQ;
				
				DEBUG(2,"--- CSQ --- %s",uartrx1_buffer);
				///���ٻ���ش��ź�ǿ��  �ϱ���������
				///memcpy(buffer, UART_RX_DATA2.USART_RX_BUF+UART_RX_DATA2.USART_RX_Len-18, 5);
			}
			
			///CGREG 
			if ( Len>=31 && strncmp(uartrx1_buffer+Len-19, "+CGREG:", 7) == 0 ) 
			{
				if(strstr(uartrx1_buffer,"0,1")!=NULL || strstr(uartrx1_buffer,"0,5")!=NULL)
				commandid_reply = REPLY_CGREG;
				DEBUG(2,"--- CGREG --- \r\n");
			}
			
			///NETOPEN
			if ( Len>=9 && Len<16 && strstr(uartrx1_buffer, "+NETOPEN:") != NULL ) 
			{
				if(uartrx1_buffer[Len-3] == '0')
				commandid_reply = REPLY_NETOPEN;
				else
					commandid_reply = REPLY_NETOPEN_ERROR;				
				DEBUG(2,"--- NETOPEN --- \r\n");
			}
			
			///NETCLOSE
			else if ( Len>=16 && strstr(uartrx1_buffer, "+NETCLOSE: 0") != NULL ) 
			{
				printf("---- NETCLOSE --- %c\r\n", uartrx1_buffer[Len-3]);
				commandid_reply = REPLY_NETCLOSE;
			}
			
			///CIPOPEN ----�����ɹ� ///ͨ��0~9���������
			if ( Len>=17 && Len<66 && (strstr(uartrx1_buffer, "+CIPOPEN:") != NULL) ) 
			{
				if(uartrx1_buffer[Len-3] == '0')
				{
					DEBUG(2,"---- CIPOPEN --- \r\n");
					commandid_reply = REPLY_CIPOPEN;
				}
				else
				commandid_reply = REPLY_ERROR;	
			}
			
			///CIPOPEN ----����ʧ��
			else if ( Len>=66 && strncmp(uartrx1_buffer+Len-24, "+CIPOPEN:", 9) == 0) 
			{						
				if(uartrx1_buffer[Len-12] == '4')
				{
					commandid_reply = REPLY_CIPOPEN_ERROR;
					DEBUG(2,"---- CIPOPEN_ERROR --- %c\r\n",uartrx1_buffer[Len-24]);
				}
				else
				commandid_reply = REPLY_ERROR;	
			}	
			
			///REPLY_CIPCLOSE ---- �ر����� 
			if ( Len>=18 && (strstr(uartrx1_buffer, "+CIPCLOSE") != NULL) ) 
			{
				
				if(uartrx1_buffer[Len-3] == '0')
				{
					DEBUG(2,"---+CIPCLOSE:---\r\n");
					commandid_reply = REPLY_CIPCLOSE;
				}
			}
			
			///CIPSEND ---���ݷ������״̬
			if ( strstr(uartrx1_buffer, "+CIPSEND:")!=NULL ) 
			{
				DEBUG(2,"CIPSEND\r\n");
				commandid_reply = REPLY_SEND_DONE;
			}
			
//			///����ģʽ
//			else if ( strstr(uartrx1_buffer, "+SIMCARD: NOT AVAILABLE")!=NULL ) 
//			{
//				commandid_reply = REPLY_INCFUN;
//			}
//			
//			///�˳�����ģʽ
//			else if ( strstr(uartrx1_buffer, "PB DONE")!=NULL ) 
//			{
//				commandid_reply = REPLY_OUTCFUN;
//			}					
					
			///���类�ر� +IPCLOSE: 0,1 
			if ( Len>=13 && strstr(uartrx1_buffer, "+IPCLOSE: 0,1")!=NULL ) 
			{
				DEBUG(3,"REPLY_CIPCLOSE\r\n");
				commandid_reply = REPLY_CIPCLOSE;			
			}			
			
		}
		
		else if(Len>=40 && uartrx1_buffer[Len-1]=='\n') ///��������'\n'��β
		{
			
			commandid_reply = REPLY_REVER;						
			if (strstr(uartrx1_buffer,"RECV FROM:") != NULL) ///ok
			{
				char *p = strstr(uartrx1_buffer,"+IPD23");
				
				char data = p-&uartrx1_buffer[0];
												
				DEBUG(2,"--RECV---%s",&uartrx1_buffer[data+strlen("+IPD23\r\n")]);///��Ч����
			}
			
		}
	}	
		
}
