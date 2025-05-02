#include <stdio.h>
#include "C://Program Files (x86)//mosquitto//devel//mosquitto.h"
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <windows.h>


int main(){
    int d;

    mosquitto_lib_init();
    printf("Started\n");

    char id[5] = "pub1";
    char username1[11]="robot_77_1";
    char pass1[10]="unUfGach)";
    char host[20]="mqtt.ics.ele.tue.nl";

    void *user1 = NULL;
    struct mosquitto *client1; 
    
    client1 = mosquitto_new(id,true,user1);
    if (client1 == NULL) {
        printf("Not connected");
        mosquitto_lib_cleanup();
        scanf(" %d", &d);
        return 1;
    } else {
        printf("Client created.\n");
    }

    int mc = mosquitto_username_pw_set(client1,username1,pass1);
    if (mc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error connecting: %s\n", mosquitto_strerror(mc));
        mosquitto_destroy(client1);
        mosquitto_lib_cleanup();
        return 1;
    } else {
        printf("Logged in.\n");
    }
    
    int rc = mosquitto_connect(client1, host, 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error connecting: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(client1);
        mosquitto_lib_cleanup();
        return 1;
    } else {
        printf("Connection success.\n");
    }


    while(true){
        // Allow time for the connection to establish via the loop
        for (int i = 0; i < 10 && mosquitto_socket(client1) == -1; i++) {
            mosquitto_loop(client1, 100, 1); // Check for connection in the loop
            Sleep(100);
        }

        if (mosquitto_socket(client1) == -1) {
            fprintf(stderr, "Error: Connection failed to establish.\n");
            mosquitto_destroy(client1);
            mosquitto_lib_cleanup();
            scanf(" %d", &d);
            return 1;
        } else {
            printf("Checked.\n");
        }

        //char topic[17]="/pynqbridge/77/#";
        char msg[12]="Hello world";
        int pc = mosquitto_publish(client1,NULL,"/pynqbridge/77/send",sizeof(msg),msg,2,false);
        if (pc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "Error connecting: %s\n", mosquitto_strerror(pc));
            mosquitto_destroy(client1);
            mosquitto_lib_cleanup();
            scanf(" %d", &d);
            return 1;
        } else {
            printf("Sending success.\n");
        }
        Sleep(5000);
    }
    
    mosquitto_destroy(client1);
    mosquitto_lib_cleanup();
    printf("End of program");
    scanf(" %d", &d);
    return 0;
}
