/*
 * Copyright (C) 2017 John M. Harris, Jr. <johnmh@openblox.org>
 *
 * This file is part of OBus.
 *
 * OBus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OBus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OBus.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "conf.h"
#include "obus.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <getopt.h>
#include <unistd.h>

#include <zmq.h>

unsigned char obus_isVerbose = 0;
char* obus_confFile = NULL;
int obus_port = 14452;
char* obus_host = NULL;
char* obus_msg_type = NULL;

#define OBUS_DEBUG

#define OBUS_DEFAULT_HOST "openblox.org"

#ifdef OBUS_DEBUG
	#undef OBUS_DEFAULT_HOST
	#define OBUS_DEFAULT_HOST "0.0.0.0"
#endif

#define OBUS_OPMODE_SEND 0
#define OBUS_OPMODE_RECV 1
#define OBUS_OPMODE_LISTEN 2

int main(int argc, char* argv[]){
	obus_confFile = strdup("obus.conf");
	obus_host = strdup(OBUS_DEFAULT_HOST);
	obus_msg_type = strdup("event:");
	
	unsigned char obus_opMode = 0;
	
    static struct option long_opts[] = {
		{"version", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'},
		{"host", required_argument, 0, 'H'},
		{"port", required_argument, 0, 'p'},
		{"type", required_argument, 0, 't'},
		{"send", no_argument, 0, 's'},
		{"recv", no_argument, 0, 'r'},
		{"listen", no_argument, 0, 'l'},
        {"verbose", no_argument, 0, 'V'},
		{"config", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int opt_idx = 0;

    while(1){
        int c = getopt_long(argc, argv, "vhVsrlt:c:p:H:", long_opts, &opt_idx);

        if(c == -1){
            break;
        }

        switch(c){
            case 'v': {
				puts(PACKAGE_NAME "-cli " PACKAGE_VERSION);
				puts("");
                puts("Copyright (C) 2017 John M. Harris, Jr.");
				puts("");
                puts("This is free software. It is licensed for use, modification and");
                puts("redistribution under the terms of the GNU General Public License,");
                puts("version 3 or later <https://gnu.org/licenses/gpl.html>");
                puts("");
                puts("Please send bug reports to: <" PACKAGE_BUGREPORT ">");
                exit(EXIT_SUCCESS);
                break;
            }
            case 'h': {
                puts(PACKAGE_NAME "-cli - Message bus client");
                printf("Usage: %s [options]\n", argv[0]);
				puts("Connection:");
				puts("   -H, --host                  Sets the host/address to connect to");
				puts("   -p, --port                  Sets the port to connect to");
				puts("");
				puts("Operation Mode:");
				puts("   -s, --send                  Send a message to the bus (Default)");
				puts("   -r, --recv                  Receive a message from the bus");
				puts("   -l, --listen                Listen for messages on the bus");
				puts("");
				puts("   -t, --type                  Type prefix to use");
				puts("");
				puts("   -c, --config                Uses a specified file instead of /etc/obus.conf");
                puts("   -v, --version               Prints version information and exits");
				puts("   -V, --verbose               Print verbose messages throughout operation");
                puts("   -h, --help                  Prints this help text and exits");
                puts("");
                puts("Options are specified by doubled hyphens and their name or by a single");
                puts("hyphen and the flag character.");
                puts("");
                puts("This message bus was originally created for the OpenBlox network, where");
                puts("it handles internal communication of OBNet services.");
                puts("");
                puts("Please send bug reports to: <" PACKAGE_BUGREPORT ">");
                exit(EXIT_SUCCESS);
                break;
            }
			case 's': {
				obus_opMode = OBUS_OPMODE_SEND;
                break;
            }
			case 'r': {
				obus_opMode = OBUS_OPMODE_RECV;
                break;
            }
			case 'l': {
				obus_opMode = OBUS_OPMODE_LISTEN;
                break;
            }
			case 'H': {
                free(obus_host);
				obus_host = strdup(optarg);
                break;
            }
			case 'p': {
				obus_port = atoi(optarg);
                break;
            }
			case 't': {
				free(obus_msg_type);
				size_t newMsgLen = strlen(optarg);
				char* newMsg = malloc(newMsgLen + 2);

				strncpy(newMsg, optarg, newMsgLen);
				strncat(newMsg, ":", 1);
				
				obus_msg_type = newMsg;
			}
			case 'c': {
                free(obus_confFile);
				obus_confFile = strdup(optarg);
                break;
            }
            case 'V': {
                obus_isVerbose = !obus_isVerbose;
                break;
            }
            default: {
                exit(EXIT_FAILURE);
            }
        }
    }

	unsigned char runningInteractive = isatty(fileno(stdin));

    int r = obus_loadConfig(obus_confFile);
	if(r != 0){
		if(runningInteractive || r == 2){
		fputs("Failed to read configuration file.\n", stderr);
		fputs("Using defaults.\n", stderr);
		}
		if(r == 2){
			exit(EXIT_FAILURE);
		}
	}

	{
		obus_ConfigEntry* ent = obus_getConfigEntry("port");
		if(ent){
			if(ent->type == OBUS_CONF_ENT_TYPE_INT){
			    obus_port = ent->data.integer;
			}
			obus_releaseConfigEntry(ent);
			ent = NULL;
		}

		ent = obus_getConfigEntry("host");
		if(ent){
			if(ent->type == OBUS_CONF_ENT_TYPE_STR){
				if(ent->data.str.len > 0){
				    obus_host = strdup(ent->data.str.str);
				}
			}
			obus_releaseConfigEntry(ent);
			ent = NULL;
		}
	}

	void* zmq_ctx = zmq_ctx_new();

	int zmqType = ZMQ_REQ;

	if(obus_opMode != OBUS_OPMODE_SEND){
		obus_port++;
		zmqType = ZMQ_SUB;
	}
	
	void* zmq_req = zmq_socket(zmq_ctx, zmqType);

	//18 is tcp:// + : + 10 for port (lots of buffer, as max port is 5 digits) + 1 '\0'
	int zmq_host_str_maxlen = 18 + strlen(obus_host);
	char* zmq_host_str = malloc(zmq_host_str_maxlen);
	snprintf(zmq_host_str, zmq_host_str_maxlen-1, "tcp://%s:%i", obus_host, obus_port);
		
    r = zmq_connect(zmq_req, zmq_host_str);
	free(zmq_host_str);
    if(r != 0){
		puts("Failed to connect to message bus.\n");
		return EXIT_FAILURE;
	}

	char buffer[OBUS_MAX_MESSAGE_LEN];

	if(obus_opMode == OBUS_OPMODE_SEND){
		buffer[0] = '\0';
		size_t typeLen = strlen(obus_msg_type);
		size_t bufSize = typeLen;
		strncat(buffer, obus_msg_type, typeLen);
		
		if(runningInteractive){
			fputs("Please type your message, and follow it with a blank line or press\n", stderr);
			fputs("C-d (to EOF).\n", stderr);
		}

		char* line = NULL;
		size_t len = 0;
		ssize_t read;

		while((read = getline(&line, &len, stdin)) != -1){
			if(bufSize > typeLen){
				strncat(buffer, "\n", 1);
			}
			
			if(runningInteractive && read == 1){
				break;
			}

			bufSize += (read - 1);
			strncat(buffer, line, read - 1);
			
			if(bufSize > (OBUS_MAX_MESSAGE_LEN - 1)){
				fputs("The message is too long.\n", stderr);
				return EXIT_FAILURE;
			}
		}

		if(ferror(stdin)){
			fputs("Error reading from stdin.", stderr);
		    return EXIT_FAILURE;
		}
		
		r = zmq_send(zmq_req, buffer, bufSize + 1, 0);
		if(r < 0){
			fputs("Failed to send message.\n", stderr);
			return EXIT_FAILURE;
		}
	}else{
		r = zmq_setsockopt(zmq_req, ZMQ_SUBSCRIBE, obus_msg_type, 0);
		if(r != 0){
			fputs("Failed to subscribe.\n", stderr);
			return EXIT_FAILURE;
		}
		
		if(obus_opMode == OBUS_OPMODE_LISTEN){
			while(1){
				r = zmq_recv(zmq_req, buffer, OBUS_MAX_MESSAGE_LEN, 0);
				if(r < 0){
					if(errno == ENOTSUP || errno == ETERM || errno == ENOTSOCK){
						fputs("Failed to receive message.\n", stderr);
						return EXIT_FAILURE;
					}else{
						if(errno == EFSM){
							fputs("EFSM\n", stderr);
						}
					}
				}

				if(r > 0){
					if(buffer[0] != '\0'){
						puts(&buffer[strlen(obus_msg_type)]);
					}
				}
			}
		}else{
			r = zmq_recv(zmq_req, buffer, OBUS_MAX_MESSAGE_LEN, 0);
			if(r < 0){
				if(errno == ENOTSUP || errno == ETERM || errno == ENOTSOCK){
					fputs("Failed to receive message.\n", stderr);
					return EXIT_FAILURE;
				}else{
					fputs("Unknown error", stderr);
					if(errno == EFSM){
						fputs("EFSM\n", stderr);
					}
				}
			}

			if(r > 0){
				if(buffer[0] != '\0'){
					puts(&buffer[strlen(obus_msg_type)]);
				}
			}
		}
	}

	zmq_close(zmq_req);
	zmq_ctx_destroy(zmq_ctx);
	
	return EXIT_SUCCESS;
}
