#ifndef __DHT11_NAM_H__
#define __DHT11_NAM_H__
#include <stdint.h>
#define OUTPUT	1
#define INPUT	0
void dht11_init(void);
int8_t read_data(double *tem, double *hum);
double readTemperature(void);
double readHumidity(void);
#endif
