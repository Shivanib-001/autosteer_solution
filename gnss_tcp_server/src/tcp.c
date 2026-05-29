#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <stdint.h> // Make sure this is at the top
#include <pthread.h>
#include "gnss.h"
//static pthread_mutex_t gnss_lock = PTHREAD_MUTEX_INITIALIZER;
#define SOCKETPORT 8080

int tim=0, dat=0,qua=0,nsati=0,diff_age=0,diff_sat=0;
double lat=0.0,lng=0.0,spd=0.0,hd=0.0;
float alti,hdop;
char valid,ltdir,lngdir,fixt;
char gnss_t[64];
    int hr,se,mi;


void *gnss_handler(void *argnss){
    int sfd = *((int *)argnss);
    free(argnss); // Free the memory allocated in main*/ 
    
    int res;
    char buf[BUFSIZE];
    while((res = read(sfd, buf, sizeof(buf) - 1)) > 0){
        if (res <= 0) break;
          
	if(verifyChecksum(buf)){
//	        pthread_mutex_lock(&gnss_lock);
		rmc_nmeaparser(buf,&tim,&valid,&lat,&ltdir,&lng,&lngdir,&spd,&hd,&dat,&fixt);
		gga_nmeaparser(buf,&tim,&lat,&ltdir,&lng,&lngdir,&qua,&nsati,&hdop,&alti,&diff_age,&diff_sat);
	if(lat != 0.0 && lng != 0.0 && alti !=0.0 ) {
		convert_time_to_UTC((unsigned)tim,&hr,&mi,&se);
		sprintf(gnss_t,"%02d:%02d:%02d",hr,mi,se);
//		pthread_mutex_unlock(&gnss_lock); 
		printf("Time: %02d:%02d:%02d %lf %lf %f %lf %lf %d %d %d %d \r\n",hr,mi,se,lat,lng,alti,spd,hd,qua,diff_sat,nsati,diff_age);

         }
       }
    memset(buf, 0, sizeof(buf));
    }
    close_port(sfd);
return NULL;
}



// Function to handle individual client communication
void *client_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    free(socket_desc); // Free the memory allocated in main*/ 
    
    struct timeval tv = { .tv_sec = 60, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    char shared[255];
    char buffer[1024] = {0};
        // 6. Read data from client
	ssize_t valread;
	while ((valread = read(sock, buffer, sizeof(buffer)))>0){
	    	if (valread > 0) {
	    	buffer[valread] = '\0';
//	  	pthread_mutex_lock(&gnss_lock); 
		// Data received successfully
		sprintf(shared,"%02d:%02d:%02d %lf %lf %f %lf %lf %d %d %d %d \r\n",hr,mi,se,lat,lng,alti,spd,hd,qua,diff_sat,nsati,diff_age);
		printf("client %s %d ",shared,sock);
		send(sock,shared, sizeof(shared), 0);
  //               pthread_mutex_unlock(&gnss_lock); 
	    	} 
	    	else if (valread == 0) {
		// Client performed an orderly shutdown
		printf("Client disconnected.\n");
	       break;
	    	} 
	    	else {
		// valread == -1, an error occurred
		if (errno == EINTR) {
		    continue; // Interrupted by signal, try reading again
		} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
		    // Only if socket is non-blocking
		    break; 
		} else {
		    perror("Read error"); // Prints descriptive error message
		    break;
				}
	    		}
	}
    close(sock);
    return NULL;
}


int main() {
    

    pthread_t ser_id;
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    
    // 1. Create socket
     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
   
    // 2. Set socket options (allows immediate restart after crash)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)); // ADD THIS
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    address.sin_port = htons(SOCKETPORT);

    // 3. Bind to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 4. Listen for clients
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int *fd = malloc(sizeof(int));
    *fd = init_read_port(DEVICE);
    printf("Server listening on port %d...\n", SOCKETPORT);
    int stat;

    // 1. Handle thread creation errors
    stat = pthread_create(&ser_id, NULL,gnss_handler,(void*)fd);
    if (stat != 0) {
        // Use strerror to translate the error code (e.g., EAGAIN)
        fprintf(stderr, "Error: pthread_create failed: %s\n", strerror(stat));
        free(fd);
        return EXIT_FAILURE;
    }
    
    // 5. Accept a connection (blocking call)
    while(1){
         pthread_t thread_id;
    	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
        continue;
    	} 
        printf("New client connected from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
       
	
	int *new_sock = malloc(sizeof(int)); // Use heap to avoid race conditions
        *new_sock = new_socket;
       

        if (pthread_create(&thread_id, NULL, client_handler, (void*)new_sock) < 0) {
            perror("Could not create thread");
            free(new_sock);
            close(new_socket);
            
            continue;
        }

        // Detach the thread so it cleans up after itself
        pthread_detach(thread_id);
 
     }
    //pthread_mutex_destroy(&gnss_lock);
         // 2. Handle thread joining errors
    stat = pthread_join(ser_id, NULL);
    if (stat != 0) {
        fprintf(stderr, "Error: pthread_join failed: %s\n", strerror(stat));
        return EXIT_FAILURE;
    }
    free(fd);
    close(server_fd);
    return 0;
}
