
#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "leds.h"

#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uiplib.h"

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

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;

static int room_1 = 0;			// people counter in room 1
static int room_2 = 0;			// people counter in room 2
static int movement = 0;		// if movement is detected -> 1
static int door_1 = 0;			// if door 1 is opened -> 1
static int door_2 = 0;			// if door 2 is opened -> 1
static int movement_first = 0;	// if movement is detected first -> 1
static int door_1_first = 0;	// if door 1 is opened first -> 1
static int door_2_first = 0;	// if door 2 is opened first -> 1

process_event_t trigger;

PROCESS(udp_server_process, "Hello to UDP server process");
PROCESS(counter, "Counting incoming and outgoing people");
AUTOSTART_PROCESSES(&udp_server_process, &counter);

void udp_rx_callback(struct simple_udp_connection *c,
		const uip_ipaddr_t *source_addr,
        uint16_t source_port,
        const uip_ipaddr_t *dest_addr,
        uint16_t dest_port,
        const uint8_t *data, uint16_t datalen){

	// Here we will handle the incoming packets from UDP connection:

//	printf("Received packet from: ");
//	uiplib_ipaddr_print(source_addr);
	printf("%s\r", (char*)data);

	if(strcmp((char*)data, "Movement detected") == 0){
		if(door_1 == 0 && door_2 == 0){
			movement_first = 1;
		}
		movement ++;
	}
	else if(strcmp((char*)data, "Door 1 opened") == 0){
		if(movement == 0 && door_2 == 0){
			door_1_first = 1;
		}
		door_1 ++;
	}
	else if(strcmp((char*)data, "Door 2 opened") == 0){
		if(door_1 == 0 && movement == 0){
			door_2_first = 1;
		}
		door_2 ++;
	}

	process_post(&counter, trigger, (char*)data);	// trigger for counter when a sensor sends data
}

// process for communication between server and clients (sensors)
PROCESS_THREAD(udp_server_process, ev, data)
{

	PROCESS_BEGIN();

	printf("Starting UDP Server Process ...\r");

	NETSTACK_ROUTING.root_start();

	simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL, UDP_CLIENT_PORT, udp_rx_callback);

	PROCESS_END();
}

// process for counting people in rooms
PROCESS_THREAD(counter, ev, data)
{

	static struct etimer timer;

	PROCESS_BEGIN();

	printf("Room 1: %d, room 2: %d\n", room_1, room_2);			// initial print (happens only once after powering the cookie)

	while(1){

		PROCESS_WAIT_EVENT_UNTIL(ev == trigger);

		if (door_1 > 0 && movement > 0){
			if(door_1_first == 1){	// someone went in room 1 and maybe in room 2
				etimer_set(&timer, CLOCK_SECOND * 5);
				PROCESS_YIELD_UNTIL(etimer_expired(&timer));	// process waits if door 2 is opened

				room_1 = room_1 + door_1 - door_2;
				room_2 = room_2 + door_2;
				door_1 = 0;
				door_2 = 0;
				movement = 0;
				door_1_first = 0;

				etimer_reset(&timer);

				// preventing negative number of people in the room
				if(room_1 < 0){
					room_2 = room_2 + room_1;
					printf("Error with room 1! \n");
					room_1 = 0;
				}
			}
			else if(movement_first == 1){	// someone went out of the room 1
				etimer_set(&timer, CLOCK_SECOND * 5);
				PROCESS_YIELD_UNTIL(etimer_expired(&timer));

				room_1 = room_1 - door_1;
				movement = 0;
				door_1 = 0;
				movement_first = 0;

				etimer_reset(&timer);

				// preventing negative number of people in the room
				if(room_1 < 0){
					printf("Error with room 1! \n");
					room_1 = 0;
				}
			}
			printf("Room 1: %d, room 2: %d\n", room_1, room_2);
		}
		else if(movement > 0 && door_2 > 0){
			if(door_2_first == 1){		// someone went from room 2 to room 1, maybe outside
				etimer_set(&timer, CLOCK_SECOND * 5);
				PROCESS_YIELD_UNTIL(etimer_expired(&timer));	// waits if door 1 is opened

				room_2 = room_2 - door_2;
				room_1 = room_1 + door_2 - door_1;
				door_2 = 0;
				door_1 = 0;
				movement = 0;
				door_2_first = 0;

				etimer_reset(&timer);

				// preventing negative number of people in the room
				if(room_2 < 0){
					room_1 = room_1 + room_2;
					printf("Error with room 2! \n");
					room_2 = 0;
				}
				if(room_1 < 0){
					printf("Error with room 1! \n");
					room_1 = 0;
				}
			}
			else if(movement_first == 1){	// someone went from room 1 to room 2
				etimer_set(&timer, CLOCK_SECOND * 5);
				PROCESS_YIELD_UNTIL(etimer_expired(&timer));

				room_1 = room_1 - door_2;
				room_2 = room_2 + door_2;
				movement = 0;
				door_2 = 0;
				movement_first = 0;

				etimer_reset(&timer);

				// preventing negative number of people in the room
				if(room_1 < 0){
					room_2 = room_2 + room_1;
					printf("Error with room 1! \n");
					room_1 = 0;
				}
			}
			printf("Room 1: %d, room 2: %d\n", room_1, room_2);
		}
	}

	PROCESS_END();
}


