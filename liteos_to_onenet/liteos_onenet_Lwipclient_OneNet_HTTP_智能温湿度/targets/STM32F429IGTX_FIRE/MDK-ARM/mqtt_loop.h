#include "mqtt.h"
#include <string.h>
#include "utils.h"
#include "stm32f4xx.h"


#ifdef WIN32
#error Not support Windows now.
#endif // WIN32

#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>



#define DS_TO_PUBLISH "mqtt_msg"

#define DS_TO_PUBLISH_T "mqtt_msg_t"

#define DS_TO_PUBLISH_RH "mqtt_msg_rh"

/*#define TOPIC_TO_SUB "39484/nCVNXYCoX68IHG4DgpyNu5aTXfY=/769243/da_test_a"
#define PACK_FALG_UNSUB 11
#define TOPIC_TO_UNSUB "39484/nCVNXYCoX68IHG4DgpyNu5aTXfY=/769243/da_test_a"

#define TIME_OUT 1
#define EVENT 2

#define MQTT_DEVICE_PROJ_ID "39484"
#define MQTT_DEVICE_ID "769832"
#define MQTT_DEVICE_API_KEY  "nCVNXYCoX68IHG4DgpyNu5aTXfY="*/

#define TOPIC_TO_SUB "132589/XbQA0YLjjdzsD=MzbTl4niQ2w30=/29702274/da_test_a"
#define PACK_FALG_UNSUB 11
#define TOPIC_TO_UNSUB "132589/XbQA0YLjjdzsD=MzbTl4niQ2w30=/29702274/da_test_a"

#define TIME_OUT 1
#define EVENT 2

#define MQTT_DEVICE_PROJ_ID "132589"
#define MQTT_DEVICE_ID "29702274"
#define MQTT_DEVICE_API_KEY  "XbQA0YLjjdzsD=MzbTl4niQ2w30="
void MQTT_Loop(void);
