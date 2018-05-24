#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "los_base.h"
#include "los_config.h"
#include "los_typedef.h"
#include "los_hwi.h"
#include "los_task.h"
#include "los_mux.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"
#include "cmsis_os.h"
#include <stdio.h>

#include "bsp_debug_usart.h"
#include "bsp_model_usart.h"
#include "bsp_model_nbiot.h"

UINT32 g_at_mutex;
char recBuf[2048];

UINT8 model_cmd(char *cmd, char *reply, UINT32 waittime )
{       
    printf(cmd);
    
    MODEL_TX(cmd);                    //发送命令
 
    if(reply == 0)                      //不需要接收数据
    {
        return MODEL_TRUE;
    }
    
    return model_cmd_check(reply, waittime);    //对接收数据进行处理
}

UINT8 model_cmd_check(char *reply, UINT32 waittime)
{
    UINT8 len = 0;
    UINT8 n = 0;
    UINT8 off = 0;
    char *redata;
    redata = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 255);
    

    len = MODEL_MSG_QueueRead(redata, waittime);
    if (len > 0) {
        printf(redata);

        n = 0;
        off = 0;
        while( (n + off) <len)
        {
            if(reply[n] == 0)                 //数据为空或者比较完毕
            {
                break;
            }
            
            if(redata[ n + off]== reply[n])
            {
                n++;                //移动到下一个接收数据
            }
            else
            {
                off++;              //进行下一轮匹配
                n=0;                //重来
            }
            //n++;
        }
    }
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, redata);

    if(reply[n]==0)   //刚好匹配完毕
    {
        return MODEL_TRUE;
    }
    
    return MODEL_FALSE;       //跳出循环表示比较完毕后都没有相同的数据，因此跳出
}

UINT8 model_init(void)
{

    UINT8 res = MODEL_FALSE;
    UINT8 try_cnt = 3;
    char *sendcmd;
    char *ackreply;

    LOS_MuxCreate(&g_at_mutex);
    
    MODEL_USART_Config(115200);                 //初始化串口
    MODEL_RESET_Config();
    MODEL_RESET(0);
    MODEL_DELAY(1000);
    MODEL_RESET(1);
    //sendcmd = LOS_MemAlloc(OS_SYS_MEM_ADDR, 100);
    //ackreply = LOS_MemAlloc(OS_SYS_MEM_ADDR, 50); 
    try_cnt = 3;
    sendcmd = "ATI\r";
    ackreply = "OK";
    while (try_cnt--) {
        if (model_cmd(sendcmd, ackreply, 200) == MODEL_TRUE) {
            res = MODEL_TRUE;break;
        } else {
            MODEL_DELAY(1000);
        }
    }

    if (res == MODEL_FALSE) {
        MODEL_USART_Config(9600); 
        try_cnt = 3;
        while (try_cnt--) {
            sendcmd = "AT\r";
            ackreply = "OK";
            if (model_cmd(sendcmd, ackreply, 200) == MODEL_TRUE) {
                sendcmd = "AT+NATSPEED=115200,10,1,2\r";
                ackreply = "OK";
                if (model_cmd(sendcmd, ackreply, 200) == MODEL_TRUE) {
                    MODEL_USART_Config(115200);
                    sendcmd = "ATI\r";
                    ackreply = "OK";
                    if (model_cmd(sendcmd, ackreply, 200) == MODEL_TRUE) {
                        res = MODEL_TRUE;break;
                    }
                }
            }
        }
    }
    //(VOID)LOS_MemFree(OS_SYS_MEM_ADDR, sendcmd);
    //(VOID)LOS_MemFree(OS_SYS_MEM_ADDR, ackreply);
    return res;
}

UINT8 model_iot_init(void)
{
    UINT8 cmd_try_cnt = 0; 

    cmd_try_cnt = 0;  
    while (model_cmd("AT+CFUN=0\r","OK", 2000) != MODEL_TRUE) {
        cmd_try_cnt++;
        if (cmd_try_cnt > 2) {
            return MODEL_FALSE; 
        }
    }

    cmd_try_cnt = 0;  
    while (model_cmd("AT+NCONFIG=AUTOCONNECT,FALSE\r", "OK", 1000) != MODEL_TRUE) {
        cmd_try_cnt++;
        if (cmd_try_cnt > 2) {
            return MODEL_FALSE; 
        }
    }
    
    cmd_try_cnt = 0;  
    while (model_cmd("AT+CFUN=1\r","OK", 2000) != MODEL_TRUE) {
        cmd_try_cnt++;
        if (cmd_try_cnt > 2) {
            return MODEL_FALSE; 
        }
    }

    if(model_cmd("AT+CIMI\r","OK", 100)!= MODEL_TRUE) return MODEL_FALSE;

    if(model_cmd("AT+CGDCONT=1,\"IP\",\"ctnb\"\r","OK", 100)!= MODEL_TRUE) return MODEL_FALSE;

    if(model_cmd("AT+CGATT=1\r","OK", 100)!= MODEL_TRUE) return MODEL_FALSE;
    
    cmd_try_cnt = 0;  
    while (model_cmd("AT+CEREG?\r","1", 2000)!= MODEL_TRUE) {
        cmd_try_cnt++;
        if (cmd_try_cnt > 40) {
            return MODEL_FALSE; 
        }
    }

    if(model_cmd("AT+CSQ\r","OK", 100)!= MODEL_TRUE) return MODEL_FALSE;
    
    return MODEL_TRUE;
    
}

INT8 model_iot_udp_create(char localport)
{
    char *cmd_buf;
    char *redata;
    INT8 res = -1;
    UINT8 len;
    
    while (LOS_MuxPend(g_at_mutex, LOS_WAIT_FOREVER) != LOS_OK);
    cmd_buf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 100); 
    redata = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 255);

    sprintf(cmd_buf,"AT+NSOCR=DGRAM,17,%d,1\r",localport+1);
    printf(cmd_buf);
    MODEL_TX(cmd_buf);
    len = MODEL_MSG_QueueRead(redata, 300); 
    if (len > 6) {

        char *pFind = redata;
        char *pNext=NULL;

        while((*pFind) != '\0'){
            if((*pFind)>='0' && (*pFind)<='9'){
                break;
            }
            
            pFind++;
        }

        pNext = strchr(pFind, '\r');
        if (pNext != NULL) {
            *pNext = 0;
            res = atoi(pFind);
        }
    }
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, cmd_buf); 
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, redata);
    LOS_MuxPost(g_at_mutex);
    
    return res;
}

INT8 model_iot_udp_send(int fd, char * serverip, UINT16 serverport, unsigned char * str, unsigned int datalen)
{
    UINT16 i, curSendLen = 0;
    UINT8 *pSendData = (UINT8 *)str;
    char *cmd_buf, *pTmp;
    UINT8 res = MODEL_FALSE;
    
    while (LOS_MuxPend(g_at_mutex, LOS_WAIT_FOREVER) != LOS_OK);
    cmd_buf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 2048);
    while (datalen > 0) {
    if(datalen > 512){
       curSendLen = 512;
    }else{
        curSendLen = datalen;
    }

    pTmp = cmd_buf;
    
    pTmp += sprintf(pTmp, "AT+NSOST=%d,%s,%u,%d,", fd, serverip, serverport, curSendLen);
    //pTmp += sprintf(pTmp, "AT+NSOST=0,%s,%u,%d,", "120.42.46.98", 2020, curSendLen);
    //pTmp += sprintf(pTmp, "AT+NSOST=0,%s,%u,%d,", "106.15.213.199", 5684, curSendLen);

    for (i = 0; i < curSendLen; i++){
        pTmp += sprintf(pTmp, "%02X", pSendData[i]);
    }

    pTmp += sprintf(pTmp, "\r\n");
    if (model_cmd(cmd_buf,"OK", 200) == MODEL_TRUE) {
        datalen -= curSendLen;
        pSendData += curSendLen;
        res = MODEL_TRUE;
    } 
    }
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, cmd_buf);  
    LOS_MuxPost(g_at_mutex);
    
    return res;

}

UINT8 model_iot_udp_close(int fd)              //IP链接断开
{ 
    char *cmd_buf;
    UINT8 res = MODEL_FALSE;
    
    while (LOS_MuxPend(g_at_mutex, LOS_WAIT_FOREVER) != LOS_OK);

    cmd_buf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 100); 

    sprintf(cmd_buf, "AT+NSOCL=%d\r", fd);
    if (model_cmd(cmd_buf, "OK", 200) == MODEL_TRUE) {
        res = MODEL_TRUE;
    }
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, cmd_buf); 
    LOS_MuxPost(g_at_mutex);
    return res;
}

UINT16 model_iot_udp_recv(int fd, void *pRet, unsigned int maxLen, unsigned char *pRetAddr, int *pRetPort)
{
    char *sendBuf;
    //char *recBuf;
    UINT16 len;
    UINT16 res = 0;

    int realDataLen = 0;
    int i;
    int iCount = 0;
    int remaining_length = 0;
    UINT8 *pRetData = (UINT8 *)pRet;
    //LOS_TaskLock();
    if (LOS_MuxPend(g_at_mutex, LOS_WAIT_FOREVER) != LOS_OK) {
        return 0;
    }
    
    sendBuf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 50);
    //recBuf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 2048);
    
    sprintf(sendBuf, "AT+NSORF=%d,%d\r", fd, maxLen);
    while (1) {
    MODEL_TX(sendBuf);
    len = MODEL_MSG_QueueRead(recBuf, 20); 
    if (len > 6) {
        //printf("[recv]:%s\n", recBuf);
        char *pFind = recBuf;//0,192.168.5.1,1024,2,ABAB,0
        char *pNext=recBuf;
        
        if ((*pFind != '\r') || (*(pFind + 2) < '0') || (*(pFind + 2) > '9')){
            pNext = strchr((recBuf+1), '\r');
            if(pNext == NULL){
                goto back;
            }
        }
        pFind = strchr(pNext, ',');//0,192.168.5.1,1024,2,ABAB,0   
        if(pFind == NULL){
            goto back;
        }

        pFind++;
        
        //192.168.5.1,1024,2,ABAB,0  IP
        pNext = strchr(pFind, ',');
        if(pNext == NULL){
            goto back;
        }
        *pNext = 0;
    
        printf("recv ip %s\r\n", pFind);
        
        if(pRetAddr != NULL){               
            memcpy(pRetAddr, pFind, 16);
        }
        *pNext = ',';

        //1024,2,ABAB,0  port
        pFind = pNext+1;
        pNext = strchr(pFind, ',');
        if(pNext == NULL){
            goto back;
        }
        *pNext = 0;

        printf("port %s\r\n", pFind);
        if(pRetPort != NULL){
            *pRetPort = atoi(pFind);
        }
        *pNext = ',';

        //2,ABAB,0  len
        pFind = pNext+1;
        pNext = strchr(pFind, ',');
        if(pNext == NULL){
            goto back;
        }
        *pNext = 0;

        realDataLen = atoi(pFind);
        printf("data len %d\r\n", realDataLen);
        *pNext = ',';

        //ABAB,0  data
        pFind = pNext+1;
        pNext = strchr(pFind, ',');
        if(pNext == NULL){
            printf("get data erro\r\n");
            printf("%s\n", recBuf);
            goto back;
        }
        *pNext = 0;
        //memcpy(p_data, pFind, realDataLen);
        for(i=0; i<realDataLen && i<maxLen && *pFind!='\0'; pFind++){
            UINT8 hexData = 0;
            
            if(*pFind>='0' && *pFind<='9'){
                hexData= *pFind - '0';
            }else if(*pFind>='A' && *pFind<='F'){
                hexData= *pFind - 'A' + 0x0A;
            }else if(*pFind>='a' && *pFind<='f'){
                hexData= *pFind - 'a' + 0x0A;
            }

            iCount++;
            
            if(iCount==1){
                pRetData[res+i] = hexData;
            }else{
                pRetData[res+i] = (pRetData[res+i]<<4) + hexData;                
                iCount = 0;
                i++;
            }            
        }

        printf("get data Len %d\r\n", i);
        //printf(pRetData);
        //printf("\r\n");
        res += i;

        pFind = pNext+1;
        pNext = strchr(pFind, '\r');
        if(pNext == NULL){
            goto back;
        }
        *pNext = 0;
        remaining_length = atoi(pFind);
        printf("remaining_length %d\r\n", remaining_length);
        //if ((remaining_length == 0)){
            break;
        //}
    } else {
        break;
        }
    }
back:
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, sendBuf);
    //(VOID)LOS_MemFree(OS_SYS_MEM_ADDR, recBuf);
    LOS_MuxPost(g_at_mutex);
    //LOS_TaskUnlock();
    return res;
}
unsigned char *recvdata = NULL;
unsigned char *input = NULL;
void nbiot_tran_send(void)
{
    unsigned int  i = 0;
    input = LOS_MemAlloc(OS_SYS_MEM_ADDR, 512); 
    for (i = 0; i < 510; i++) {
        input[i] = 0x30 + (i % 10);
    }
    input[510] = 0x55;
    input[511] = 0xAA;
    model_iot_udp_create(0);
    model_iot_udp_send(0, "120.42.46.98", 2020, input, 512);
    recvdata = LOS_MemAlloc(OS_SYS_MEM_ADDR ,4096);
    while(1) {
        scanf("%s",input);
        model_iot_udp_send(0, "120.42.46.98", 2020, input, sizeof(input));
    }
}
void nbiot_tran_recv(void)
{
    unsigned char ipaddr[16] = {0};

    unsigned int           len = 0;
    int           remoteport = 0;
    int           linkid = 0;

    if (recvdata == NULL) {
        return;
    }
    memset(recvdata, 0, 1024);
    len = model_iot_udp_recv(linkid, recvdata, 512, ipaddr, &remoteport);
    if (len) {
        printf("nbiot recv msglen : %d \r\n", len);
    }

}
/*---------------------------------------------------------------------*/
