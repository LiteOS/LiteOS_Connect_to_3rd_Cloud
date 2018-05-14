#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "los_base.h"
#include "los_config.h"
#include "los_typedef.h"
#include "los_hwi.h"
#include "los_task.h"
#include "los_sem.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"
#include "cmsis_os.h"
#include <stdio.h>

#include "bsp_debug_usart.h"
#include "bsp_model_usart.h"
#include "bsp_model_nbiot.h"

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
    UINT8 len;
    UINT8 n;
    UINT8 off;
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
    char *redata;
    UINT8 len;
    UINT8 res = MODEL_FALSE;
    UINT8 try_cnt = 3;

    redata = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 255);  

    MODEL_USART_Config();                 //初始化串口
    MODEL_RESET_Config();
    MODEL_RESET(0);
    MODEL_DELAY(300);
    MODEL_RESET(1);
    while (try_cnt--) {
        model_cmd("AT+CGMR\r",0, 0);
        
        len = MODEL_MSG_QueueRead(redata, 2000);   //接收数据

        if (len > 0) {
            printf(redata);
            if (strstr(redata,"HI2110") != 0) {
                res = MODEL_TRUE;break;
            } else if (strstr(redata,"BC95HB") != 0) {
                res = MODEL_TRUE;break;
            } else {
                res = MODEL_FALSE;
            }
        }
    }
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, redata);

    return res;
}

UINT8 model_iot_init(void)
{
    UINT8 cmd_try_cnt = 0;
          
        
    if(model_cmd("AT+NCONFIG=AUTOCONNECT,FALSE\r","OK", 1000) != MODEL_TRUE) return MODEL_FALSE;

    if(model_cmd("AT+CFUN=1\r","OK", 2000) != MODEL_TRUE) return MODEL_FALSE;      

    if(model_cmd("AT+CIMI\r","OK", 200)!= MODEL_TRUE) return MODEL_FALSE;

    if(model_cmd("AT+CGDCONT=1,\"IP\",\"ctnb\"\r","OK", 200)!= MODEL_TRUE) return MODEL_FALSE;

    if(model_cmd("AT+CGATT=1\r","OK", 200)!= MODEL_TRUE) return MODEL_FALSE;
    
    while (model_cmd("AT+CEREG?\r","1", 2000)!= MODEL_TRUE) {
        cmd_try_cnt++;
        if (cmd_try_cnt > 20) {
            return MODEL_FALSE; 
        }
    }

    if(model_cmd("AT+CSQ\r","OK", 200)!= MODEL_TRUE) return MODEL_FALSE;
    return MODEL_TRUE;
    
}

INT8 model_iot_udp_create(char localport)
{
    char *cmd_buf;
    char *redata;
    INT8 res = -1;

    UINT8 len;
    
    cmd_buf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 100); 
    redata = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 255);

    sprintf(cmd_buf,"AT+NSOCR=DGRAM,17,%d,1\r",localport);

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
    
    return res;
}

INT8 model_iot_udp_send(char * serverip, UINT16 serverport, const char * str, unsigned int datalen)
{
    UINT8 i, curSendLen = 0;
    UINT8 *pSendData = (UINT8 *)str;
    char *cmd_buf, *pTmp;
    UINT8 res = MODEL_FALSE;
    
    curSendLen = datalen;//strlen(str);
    
    cmd_buf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 500);
    
    pTmp = cmd_buf;
    
    pTmp += sprintf(pTmp, "AT+NSOST=0,%s,%u,%d,", serverip, serverport, curSendLen);

    for (i = 0; i < curSendLen; i++){
        pTmp += sprintf(pTmp, "%02X", pSendData[i]);
    }

    pTmp += sprintf(pTmp, "\r\n");
    if (model_cmd(cmd_buf,"OK", 200) == MODEL_TRUE) {
        res = MODEL_TRUE;
    } 
    
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, cmd_buf);  
    
    return res;

}

UINT8 model_iot_udp_close(int fd)              //IP链接断开
{ 
    char *cmd_buf;
    cmd_buf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 100); 

    sprintf(cmd_buf, "AT+NSOCL=%d\r", fd);
    if(model_cmd(cmd_buf, "OK", 200) != MODEL_TRUE)
    {
        return MODEL_FALSE;
    }
    return MODEL_TRUE;
}

UINT8 model_iot_udp_recv(int fd, unsigned char *p_data, unsigned int maxLen, char *pRetAddr, unsigned short *pRetPort)
{
    char *sendBuf;
    char *recBuf;
    UINT8 len;
    UINT8 res = 0;

    int realDataLen = 0;
    int i;
    int iCount = 0;
    char *pRetData;
    
    sendBuf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 100);
    recBuf = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 255);
    pRetData = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 255);
    
    sprintf(sendBuf, "AT+NSORF=%d,%d\r", fd, maxLen);
    MODEL_TX(sendBuf);
    len = MODEL_MSG_QueueRead(recBuf, 1000); 
    if (len > 6) {
        //printf("udp_recv:%s\n", recBuf);
        char *pFind = strchr(recBuf, ',');//0,192.168.5.1,1024,2,ABAB,0
        char *pNext=NULL;
            
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

        //2,ABAB,0  len
        pFind = pNext+1;
        pNext = strchr(pFind, ',');
        if(pNext == NULL){
            goto back;
        }
        *pNext = 0;

        realDataLen = atoi(pFind);
        printf("data len %d\r\n", realDataLen);

        //ABAB,0  data
        pFind = pNext+1;
        pNext = strchr(pFind, ',');
        if(pNext == NULL){
            goto back;
        }
        *pNext = 0;
        memcpy(p_data, pFind, realDataLen);

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
                pRetData[i] = hexData;
            }else{
                pRetData[i] = (pRetData[i]<<4) + hexData;                
                iCount = 0;
                i++;
            }            
        }

        printf("get data Len %d, ", i);
        printf(pRetData);
        printf("\r\n");
        res = i; 
    }
back:
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, sendBuf);
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, recBuf);
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, pRetData);
    return res;
}

void net_work_process(void)
{
    UINT8 res = MODEL_FALSE;
    char *str;
    char *serverip;
    UINT16 serverport = 2020;
    UINT8 heart_time = 0;
    UINT32 heart_cnt = 0;

    INT8 fd;
    
    //str = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 80);
    //serverip = (char*)LOS_MemAlloc(OS_SYS_MEM_ADDR, 50);
    //serverip = "120.42.46.98";

    if (model_init() != MODEL_TRUE) {
         return;
    }

    res = model_iot_init();
     
     while(res != MODEL_TRUE)
     {
         LOS_TaskDelay(1000);
     }
    
   /*  fd = model_iot_udp_create(1);
     if (fd >= 0) {
         sprintf(str, "nb-iot udp send msg %d", heart_cnt);
         res = model_iot_udp_send(serverip, serverport, str);
         while(1)
         {
             LOS_TaskDelay(1000);
             if (model_iot_udp_recv(fd, str, 255, serverip, &serverport) > 0)
             {
                break;
             }
         }
     }
     model_iot_udp_close(fd);*/
    //(VOID)LOS_MemFree(OS_SYS_MEM_ADDR, str);
    //(VOID)LOS_MemFree(OS_SYS_MEM_ADDR, serverip);
     
}

/*---------------------------------------------------------------------*/
