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
#include <errno.h>

#include <zmq.h>

unsigned char obusd_isVerbose = 0;
char* obusd_confFile = NULL;
int obusd_port = 14452;
char* obusd_host = NULL;

unsigned char obus_processMessage(char* buf, int len, void* zmq_resp, void* zmq_pub){
	puts(buf);

	int r = zmq_send(zmq_pub, buf, len, 0);
	if(r < 0){
		fputs("Failed to send message.\n", stderr);
		return 1;
	}
	
	return 0;
}

int main(int argc, char* argv[]){
	obusd_confFile = strdup("obusd.conf");
	obusd_host = strdup("*");
	
    static struct option long_opts[] = {
		{"version", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'},
		{"host", required_argument, 0, 'H'},
		{"port", required_argument, 0, 'p'},
        {"verbose", no_argument, 0, 'V'},
		{"config", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int opt_idx = 0;

    while(1){
        int c = getopt_long(argc, argv, "vhVc:p:H:", long_opts, &opt_idx);

        if(c == -1){
            break;
        }

        switch(c){
            case 'v': {
				puts(PACKAGE_NAME "_daemon " PACKAGE_VERSION);
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
                puts(PACKAGE_NAME "_daemon - Message bus daemon");
                printf("Usage: %s [options]\n", argv[0]);
				puts("   -H, --host                  Sets the host/address to bind to");
				puts("   -p, --port                  Sets the port to bind to");
				puts("   -c, --config                Uses a specified file instead of obusd.conf");
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
			case 'H': {
                free(obusd_host);
				obusd_host = strdup(optarg);
                break;
            }
			case 'p': {
				obusd_port = atoi(optarg);
                break;
            }
			case 'c': {
                free(obusd_confFile);
				obusd_confFile = strdup(optarg);
                break;
            }
            case 'V': {
                obusd_isVerbose = !obusd_isVerbose;
                break;
            }
            default: {
                exit(EXIT_FAILURE);
            }
        }
    }

    int r = obus_loadConfig(obusd_confFile);
	if(r != 0){
		fputs("Failed to read configuration file.\n", stderr);
		fputs("Using defaults.\n", stderr);
		if(r == 2){
			exit(EXIT_FAILURE);
		}
	}

	{
		obus_ConfigEntry* ent = obus_getConfigEntry("port");
		if(ent){
			if(ent->type == OBUS_CONF_ENT_TYPE_INT){
			    obusd_port = ent->data.integer;
			}
			obus_releaseConfigEntry(ent);
			ent = NULL;
		}

		ent = obus_getConfigEntry("host");
		if(ent){
			if(ent->type == OBUS_CONF_ENT_TYPE_STR){
				if(ent->data.str.len > 0){
				    obusd_host = strdup(ent->data.str.str);
				}
			}
			obus_releaseConfigEntry(ent);
			ent = NULL;
		}
	}

	void* zmq_ctx = zmq_ctx_new();
	void* zmq_resp = zmq_socket(zmq_ctx, ZMQ_ROUTER);
	void* zmq_pub = zmq_socket(zmq_ctx, ZMQ_PUB);

	//18 is tcp:// + : + 10 for port (lots of buffer, as max port is 5 digits) + 1 '\0'
	int zmq_host_str_maxlen = 18 + strlen(obusd_host);
	char* zmq_host_str = malloc(zmq_host_str_maxlen);
	snprintf(zmq_host_str, zmq_host_str_maxlen-1, "tcp://%s:%i", obusd_host, obusd_port);
		
    r = zmq_bind(zmq_resp, zmq_host_str);
	if(r != 0){
		fprintf(stderr, "Failed to bind %s\n", zmq_host_str);
		free(zmq_host_str);
		return EXIT_FAILURE;
	}

	snprintf(zmq_host_str, zmq_host_str_maxlen-1, "tcp://%s:%i", obusd_host, obusd_port+1);
	
	r = zmq_bind(zmq_pub, zmq_host_str);
	if(r != 0){
		fprintf(stderr, "Failed to bind %s\n", zmq_host_str);
		free(zmq_host_str);
		return EXIT_FAILURE;
	}

	free(zmq_host_str);

	char buffer[OBUS_MAX_MESSAGE_LEN];

	while(1){
		zmq_pollitem_t items[] = {
            {zmq_resp, 0, ZMQ_POLLIN, 0},
            {zmq_pub, 0, ZMQ_POLLIN, 0}
        };

		zmq_poll(items, 2, -1);

		if(items[0].revents & ZMQ_POLLIN){
			r = zmq_recv(zmq_resp, buffer, OBUS_MAX_MESSAGE_LEN, 0);
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
					buffer[r] = '\0';
					r = obus_processMessage(buffer, r, zmq_resp, zmq_pub);
					if(r != 0){
						return EXIT_FAILURE;
					}
					memset(buffer, '\0', OBUS_MAX_MESSAGE_LEN);
				}
			}
		}

		if(items[1].revents & ZMQ_POLLIN){
		    
		}
	}
	
	return EXIT_SUCCESS;
}
