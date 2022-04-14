#ifndef MQTT_H
#define MQTT_H

void mqtt_init();
void connectMQTTServer();
void pubMQTTmsg();
void mqtt_loop();
void mqtt_connect_loop();

#endif