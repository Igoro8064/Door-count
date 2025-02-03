
#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "leds.h"

#include "serial-line.h"
#include "string.h"

#include "efr32mg12p432f1024gl125.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "si7021.h"
#include "board_cookie.h"
#include "util.h"
#include "imu.h" // For IMU
#include "icm20648.h" // For IMU

#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uiplib.h"

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define MY_MESSAGE "Movement detected"		// mesage that is sent if sensor is triggered

static struct simple_udp_connection udp_conn;

void udp_rx_callback(struct simple_udp_connection *c,
        const uip_ipaddr_t *source_addr,
        uint16_t source_port,
        const uip_ipaddr_t *dest_addr,
        uint16_t dest_port,
        const uint8_t *data, uint16_t datalen){

	// Here we will handle the incoming packets from UDP connection:
	/*
	printf("Received packet from: ");
	uiplib_ipaddr_print(source_addr);
	printf(", Data: %s\r", (char*)data);
	*/
}

process_event_t trigger;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "Hello to UDP client process");
PROCESS(PIR, "PIR sensor");
AUTOSTART_PROCESSES(&udp_client_process, &PIR);

// proces for communication between server and client (sensor)
PROCESS_THREAD(udp_client_process, ev, data)
{

	uip_ipaddr_t dest_address;

	PROCESS_BEGIN();

	printf("Starting UDP client Process ...\r");

	simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);

	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == trigger);

		if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_address)){
			printf("Message sent. \n");
			simple_udp_sendto(&udp_conn, MY_MESSAGE, sizeof(MY_MESSAGE), &dest_address);
		}
		else{
			printf("Node not reachable yet ...\r");
		}
	}

	PROCESS_END();
}

PROCESS_THREAD(PIR, ev, data)
{
	int value = 0;
	static int block = 0;

	static struct etimer timer;

	PROCESS_BEGIN();

	GPIO_PinModeSet(gpioPortB, 7, gpioModeInput, 1);		// sensor pin set to input

	etimer_set(&timer, CLOCK_SECOND / 5);

	while(1){

		value = GPIO_PinInGet(gpioPortB, 7);

		if(value == 1 && block == 0){
			leds_on(LEDS_RED);
			process_post(&udp_client_process, trigger, (char*)data);
			block = 1;
			printf("Motion detected \n");
		}
		else if(value == 0 && block == 1){
			leds_off(LEDS_RED);
			printf("no motion \n");
			block = 0;
		}

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		etimer_reset(&timer);
	}

	PROCESS_END();
}
