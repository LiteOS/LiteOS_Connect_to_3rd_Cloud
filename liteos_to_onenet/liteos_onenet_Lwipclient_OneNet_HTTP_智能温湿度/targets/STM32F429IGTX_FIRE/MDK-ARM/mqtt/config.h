#ifndef ONENET_CONFIG_H
#define ONENET_CONFIG_H

#include <stddef.h>

#ifdef WIN32
#pragma warning(disable:4819)
#pragma warning(disable:4996)
#define inline __inline


#else // UNIX
//#include <sys/uio.h>
#endif // _WIN32


struct iovec {
    void *iov_base;
    size_t iov_len;
};



#define MQTT_DEFAULT_ALIGNMENT sizeof(int)

#define MQTT_HOST	"183.230.40.39"
#define MQTT_PORT	6002

#endif // ONENET_CONFIG_H
