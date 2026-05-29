#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h> // Include cJSON header
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <arpa/inet.h>


FILE *gnss_log();
cJSON *parse_json_file(const char *filename);
char *read_file(const char *filename);
double initial_bearing(double lat_a,double lat_b,double lon_a,double lon_b);
double distance(double lat1,double lat2,double lon1,double lon2);
//double lat,lng,lat_next,lng_next,curr_lat,curr_lng,spd;
double lat_a,lng_a,lat_b,lng_b,curr_lat,curr_lng,spd, lat_c, lng_c, head;
double cross_track_error(double cur_n,double bear_way, double curr_bear);
float along_track_error(double curk,double cte_n);
double ConvertDegtoRad(double degree);
double ConvertRadtoDeg(double radians);
bool flag=true;

int Constrain(int au32_IN, int au32_MIN, int au32_MAX);
int MAP(int au32_IN, long int au32_INmin, long int au32_INmax, int au32_OUTmin,int au32_OUTmax);
int pidCTE(double val,double setp,double kp,double ki,double kd);
int pidHead(double val,double spoint,double kp,double ki,double kd);
void log_json_to_file(FILE *file_ptr,double latj,double lngj,double lata, double lnga, double latb, double lngb, double ctept, double headp, double velp, double headerr, double abang,double stacan, double heacan, double acang);
#define BUFFER_SIZE 5
int fd,sockfd, portno;
struct termios tty;
char buffer[BUFFER_SIZE];
#define PORT 8080
void WPortInit();


int main(void) {
    cJSON *json_data = parse_json_file("../autonomous-screen/public/data/path_points.json"); // Example filename
    const char *portname = "/dev/ttyUSB0";
    
    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Error opening serial port");
    }
    /*-------------------------------------------------------------------------------------------------*/
    
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buf[1024] = {0};
    int hour, min, sec;
    double curr_lat=0.0, curr_lng=0.0, alt=0.0, spd=0.0, head=0.0;
    int id=0,nsat=0,tdiff=0,satid;
    // 1. Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 2. Convert address to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    // 3. Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    // 4. Send and receive data
    send(sock, "++++", 4, 0);
    
    /*--------------------------------------------------------------------------------------------------*/
    
    
    
    FILE *fdf = gnss_log(); 
    WPortInit();
               /*--------------- TCP setup ------------------------*/
              
    if (json_data != NULL) {
        // Accessing a number item:
        cJSON *width_item = cJSON_GetObjectItemCaseSensitive(json_data, "Application_width");
        if (cJSON_IsNumber(width_item)) {
            printf("Width: %f \n", width_item->valuedouble);
        }
        
        cJSON *path = cJSON_GetObjectItemCaseSensitive(json_data, "path_points");
        if (cJSON_IsArray(path)) {
            int chk = cJSON_GetArraySize(path);
            int np = 0;
            cJSON *check;
            cJSON *check_prev;
            while(np<chk){
            check = cJSON_GetArrayItem(path,np );
            /*------------------ PT ---------------------*/
            if (cJSON_IsArray(check)) {
            int pt = cJSON_GetArraySize(check);
            int it = 0;
            cJSON *gcplt;
            while(it < pt){
               gcplt = cJSON_GetArrayItem(check, it);
               if (!cJSON_IsObject(gcplt)) {
                if (cJSON_IsNumber(gcplt)) {
                    if(it == 0){
                    lat_b = (gcplt->valuedouble);
                   
                      }
                   else if(it == 1){  
                   lng_b = (gcplt->valuedouble);
                   
                   }    
                            
                 }
                  
               }      
               it+=1; 
            }      
           }
          
           check_prev = cJSON_GetArrayItem(path,np+1);
           if (cJSON_IsArray(check_prev)) {
            int pt_prev = cJSON_GetArraySize(check_prev);
            int it_prev = 0;
            cJSON *gcplt_prev;
            while(it_prev < pt_prev){
               gcplt_prev = cJSON_GetArrayItem(check_prev, it_prev);
               if (!cJSON_IsObject(gcplt_prev)) {
                if (cJSON_IsNumber(gcplt_prev)) {
                    if(it_prev == 0){
                    lat_c = (gcplt_prev->valuedouble);
                   
                      }
                   else if(it_prev == 1){  
                   lng_c = (gcplt_prev->valuedouble);
   
                   }    
                            
                 }
                  
               }      
               it_prev+=1; 
            }      
           }
      
/*----------------------------------------------------------------*/ 
 while((read(sock, buf, sizeof(buf)-1))>0){
    send(sock, "----", 4, 0);
    // printf("%s",buf);
        // Use sscanf to filter each value
    // %d for integers, %lf for doubles (long floats), and ':' to skip the colons
    int parsed = sscanf(buf, "%d:%d:%d %lf %lf %lf %lf %lf %d %d %d %d \r\n", 
                        &hour, &min, &sec, &curr_lat, &curr_lng, &alt, &spd, &head, &id,&nsat,&satid,&tdiff);
// Check if all 9 fields were parsed successfully
    if (parsed == 12) {
        printf("Time:      %02d:%02d:%02d\n", hour, min, sec);
        printf("Latitude:  %f\n", curr_lat);
        printf("Longitude: %f\n", curr_lng);
        printf("Altitude:  %f\n", alt);
        printf("Speed:     %f\n", spd);
        printf("Heading:   %f\n", head);
        printf("ID:        %d\n", id);
        printf("Number of satellite:  %d\n", nsat);
        printf("Sation ID: %d\n", satid);
        printf("Differential Time:    %d\n",tdiff);
    } else {
        printf("Error: Only parsed %d values.\n", parsed);
    }

           
           if(flag){    
           
           lat_a = curr_lat;
           lng_a = curr_lng;
           flag=false;
           } 
           
            
           //printf(" %f %f \n %f %f \n %f %f \n %f %f  %d\n",lat_a,lng_a,lat_b,lng_b,lat_c, lng_c,curr_lat,curr_lng,np);
           printf("check: %d \n",np);
           double dist_ac = distance(lat_a,curr_lat,lng_a,curr_lng) ;
           
           double bear_ab = initial_bearing(lat_a, lat_b, lng_a, lng_b);
           
           double bear_ac = initial_bearing(lat_a, curr_lat, lng_a, curr_lng);
           double bear_cb=initial_bearing(curr_lat,lat_b, curr_lng,lng_b);
           double bear_bn=initial_bearing(lat_b,lat_c, lng_b, lng_c);
           double cte = cross_track_error(dist_ac,bear_ab, bear_ac);
           printf("cte : %f \n", cte);
           if (bear_cb<180.0){
           bear_cb=bear_cb+180.0;
           }else if(bear_cb>180.0){
            bear_cb=bear_cb-180.0;
           }         
           double diffang_h = head-bear_ab;
           //double diffang_h= bear_ac - bear_ab;
           printf("head error :  %f \n", diffang_h);

           double bear_diff= bear_cb - bear_bn;
           
               
           double dist_bc=distance(lat_b,curr_lat,lng_b,curr_lng);
           
           double theta_half = ConvertDegtoRad(bear_diff/2);

  //         printf("distance: %f\n %f %f \r\n",dist_bc, bear_diff, dist_bc*sin(theta_half));
           
           
           double skip_distance = fabs(3.4 / sin(theta_half));
           
    //       printf("skip distance : %f \n ", skip_distance);
         /*//pid head and estimated cte combined output   
           //can +/- 5 
           double can_cte = 0.2*cte*cte + 14.8* cte +106;
           //printf("can %f\n",can);
           
           //int can_cte = pidCTE(cte,0.0,150.0,0.0,0.0);
           int can_head = pidHead(diffang_h,0.0,300.0,0.0,10.0);
           
 
           int can = 0.35 * can_cte + 0.65 * can_head;*/
         int can_head = pidHead(diffang_h,0.0,90.0,0.00001,10.0);
         // int can_head = pidHead(diffang_h,0.0,90.0,0.0,10.0);
         if (diffang_h<-180.0){
           diffang_h=diffang_h+360.0;
           }else if(diffang_h>180.0){
            diffang_h=diffang_h-360.0;
           }


           int steer = diffang_h + ConvertRadtoDeg(atan2((0.37*cte),(spd + 0.001)));
           int can_stan =(-4.13*0.001)*steer*steer + 1.95 * steer + 106;
          // int can = can_stan;
           int can = 0.30*can_head + 0.70*can_stan;
            //int can = can_head;
            can = Constrain(can, 9, 184);
	    sprintf(buffer,"%d\r\n",can);
	    printf("CAN:%s",buffer);
	    ssize_t z = write(fd,buffer, sizeof(buffer)); // Exclude null terminator

		if (z < 0) {
	    		perror("Error writing to serial port");
		} else {
	    		printf("Wrote %zd bytes\n", z);
		}
		   
           
           if(dist_bc > 0.0 && dist_bc <skip_distance){
             
             printf("achieved");
             lat_a=lat_b;
             lng_a=lng_b;
             np+=1;
             break;
            }
          log_json_to_file(fdf,curr_lat,curr_lng,lat_a, lng_a, lat_b, lng_b, cte , head, spd, diffang_h, bear_ab,bear_diff, np, bear_ac);   

             }   // while file read
            bzero(buffer, BUFFER_SIZE);
            
            usleep(30000);            
            }
         }
         close(fd);
         fclose(fdf);
        cJSON_Delete(json_data); // Free the cJSON object and its children
       }// json file end
   

    return 0;
}




void WPortInit(){


    // 2. Get current terminal attributes
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error from tcgetattr");
        
    }

    // 3. Configure the port for non-canonical mode and raw data
    cfsetospeed(&tty, B115200); // Set output baud rate (e.g., B9600)
    cfsetispeed(&tty, B115200); // Set input baud rate

    // Disable canonical mode (ICANON) and other input processing
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    // Disable output processing (OPOST)
    tty.c_oflag &= ~(OPOST);
    // Set 8-bit characters (CS8) and disable parity (PARENB)
    tty.c_cflag |= (CS8);
    tty.c_cflag &= ~(CSIZE | PARENB);

    // Set VMIN and VTIME for non-blocking read behavior (optional, primarily for reading)
    // MIN=0, TIME=0 makes read() return immediately with available chars or 0/EAGAIN
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    // 4. Apply the attributes
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr");
    }

}


cJSON *parse_json_file(const char *filename) {
    char *content = read_file(filename);
    if (content == NULL) {
        return NULL;
    }

    cJSON *parsed_json = cJSON_Parse(content); // Parse the string into a cJSON object
    free(content); // Free the file content buffer

    if (parsed_json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr(); // Get the error position
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\\n", error_ptr);
        }
        return NULL;
    }

    return parsed_json; // The caller is responsible for freeing the cJSON object
}


char *read_file(const char *filename) {
    FILE *fp = fopen(filename, "rb"); // Open file in binary read mode
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(fp, 0, SEEK_END); // Go to the end of the file
    long length = ftell(fp); // Get the file length
    fseek(fp, 0, SEEK_SET); // Go back to the start

    char *buffer = (char *)malloc(length + 1); // Allocate memory for the file content
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(fp);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1, length, fp); // Read the file into the buffer
    if (read_bytes != (size_t)length) {
        perror("Error reading file");
        free(buffer);
        fclose(fp);
        return NULL;
    }

    buffer[length] = '\0'; // Null-terminate the string
    fclose(fp);

    return buffer; // The caller is responsible for freeing this memory
}



double initial_bearing(double lat_a,double lat_b,double lon_a,double lon_b){
	double delta_lng;
	double lat1=ConvertDegtoRad(lat_a);
	double lat2=ConvertDegtoRad(lat_b);
	double lon1=ConvertDegtoRad(lon_a);
	double lon2=ConvertDegtoRad(lon_b);

	delta_lng = lon2-lon1;
	float y = sin(delta_lng) * cos(lat2);
	float x = cos(lat1)*sin(lat2) -sin(lat1)*cos(lat2)*cos(delta_lng);
	double bearing= ConvertRadtoDeg(atan2(y,x));

	double final;
	if (bearing>0){
		final=bearing;
		}else {
		final=360+bearing;
	}

	return final;
}


double distance(double lat1,double lat2,double lon1,double lon2){


	double  dlon = ConvertDegtoRad(lon2 - lon1) ;
	double dlat = ConvertDegtoRad(lat2 - lat1) ;
	double deg_lat1 = ConvertDegtoRad(lat1);
	double deg_lat2 = ConvertDegtoRad(lat2);
	double a= sin(dlat/2)*sin(dlat/2) + (cos(deg_lat1) * cos(deg_lat2))* (sin(dlon/2)*sin(dlon/2));
	double c=2 * atan2(sqrt(a),sqrt(1-a));
	double R= 6371000.0 ;
	double dist=c*R;
	return dist;
}



double cross_track_error(double cur_n,double bear_way, double curr_bear){
	double delta_bearing =ConvertDegtoRad( curr_bear - bear_way);
	double perd = cur_n*sin(delta_bearing);
	return perd;
}


float along_track_error(double curk,double cte_n){
	float alg = sqrt((curk*curk) - (cte_n*cte_n));
	return alg;
}

double ConvertDegtoRad(double degree) {
	double pi = 3.14159265359;
	return (degree * (pi /180));
}


double ConvertRadtoDeg(double radians) {
	double pi = 3.14159265359;
	return (radians * (180 /pi));
}



int pidCTE(double val,double setp,double kp,double ki,double kd){

    int steer,start_ht ,final_ht=0;
    struct timeval tim; 
    gettimeofday(&tim, NULL);
    start_ht = tim.tv_usec;
    float preverrorh=0.0, error_h=0.0;
    double elapsed_time, head_h=0;
    long int PID_val=0;
    double setpoint_h, integral_h=0.0, derivative_h=0.0;
    setpoint_h=setp;
    
    error_h=(float)(setpoint_h-val);
    
    elapsed_time=(double)(start_ht-final_ht);
    //printf("%f \n",elapsed_time);
    integral_h+=((double)(error_h*elapsed_time));
    derivative_h = ((double)(error_h-preverrorh))/elapsed_time;
    PID_val=(int)(kp*error_h+ki*integral_h+kd*derivative_h);
    preverrorh=error_h;
    final_ht = tim.tv_usec;
    long int dutycycleh=Constrain(PID_val, -4096, 4096);
    int vah = MAP(dutycycleh, -4096,4096, 0,255);
steer = (int)((0.0003*vah*vah)-(0.6543*vah)+185);

    
    //printf("steer:%d \n",steer);
    return steer;
}

int pidHead(double val,double setp,double kp,double ki,double kd){

    int steer,start_ht ,final_ht=0;
    struct timeval tim; 
    gettimeofday(&tim, NULL);
    start_ht = tim.tv_usec;
    float preverrorh=0.0, error_h=0.0;
    double elapsed_time, head_h=0;
    long int PID_val=0;
    double setpoint_h, integral_h=0.0, derivative_h=0.0;
    setpoint_h=setp;
    
    error_h=(float)(setpoint_h-val);
    
    if (error_h<-180.0){
        error_h = error_h+360.0;
    }
    else if(error_h>180){
        error_h= error_h - 360.0;
    }
    
    elapsed_time=(double)(start_ht-final_ht);
    //printf("%f \n",elapsed_time);
    integral_h+=((double)(error_h*elapsed_time));
    derivative_h = ((double)(error_h-preverrorh))/elapsed_time;
    PID_val=(int)(kp*error_h+ki*integral_h+kd*derivative_h);
    preverrorh=error_h;
    final_ht = tim.tv_usec;
    long int dutycycleh=Constrain(PID_val, -4096, 4096);
    int vah = MAP(dutycycleh, -4096,4096, 0,255);
    steer = (int)((0.0003*vah*vah)-(0.6543*vah)+185);

    return steer;

}
int MAP(int au32_IN, long int au32_INmin, long int au32_INmax, int au32_OUTmin,int au32_OUTmax) {
	return ((((au32_IN - au32_INmin) * (au32_OUTmax - au32_OUTmin))
	/ (au32_INmax - au32_INmin)) + au32_OUTmin);
}

int Constrain(int au32_IN, int au32_MIN, int au32_MAX) {
	if (au32_IN < au32_MIN) {
		return au32_MIN;
		} else if (au32_IN > au32_MAX) {
		return au32_MAX;
		} else {
		return au32_IN;
	}
}

FILE *gnss_log(){

 time_t current_time;
    struct tm *local_time_info;

    // Get the current calendar time as a time_t object
    // Passing NULL to time() also stores the value in current_time
    time(&current_time); 

    // Convert to local time format (struct tm)
    local_time_info = localtime(&current_time);

    // Format and print the time
    // strftime is a more flexible and safer alternative to asctime/ctime
    char time_string[80];
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", local_time_info);

    //printf("Current local time: %s\n", time_string);

    char filename[100];
    sprintf(filename,"logdata/log_%s.json", time_string);
    for(size_t b=0; b<strlen(filename); b+=1){
    if(filename[b] == ' '){
       filename[b] = '_';
       }
    }
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Could not open %s for writing\n", filename);
        fclose(file);
    }
    return file;

}


void log_json_to_file(FILE *file_ptr,double latj,double lngj,double lata, double lnga, double latb, double lngb, double ctept, double headp, double velp, double headerr, double abang, double stacan, double heacan, double acang) {
    cJSON *monitor = cJSON_CreateObject();
    cJSON_AddNumberToObject(monitor,"Latitude", latj);
    cJSON_AddNumberToObject(monitor,"Longitude", lngj);
    cJSON_AddNumberToObject(monitor,"Latitude_A", lata);
    cJSON_AddNumberToObject(monitor,"Longitude_A", lnga);
    cJSON_AddNumberToObject(monitor,"Latitude_B", latb);
    cJSON_AddNumberToObject(monitor,"Longitude_B", lngb);
    cJSON_AddNumberToObject(monitor,"CTE", ctept);
    cJSON_AddNumberToObject(monitor,"Heading", headp);
    cJSON_AddNumberToObject(monitor,"velocity", velp);
    cJSON_AddNumberToObject(monitor,"Heading error", headerr);
    cJSON_AddNumberToObject(monitor,"AB Bearing", abang);
    cJSON_AddNumberToObject(monitor,"AC Bearing", acang);
    cJSON_AddNumberToObject(monitor,"diff angle", stacan);
    cJSON_AddNumberToObject(monitor,"checkpoint", heacan);
    char *json_string = cJSON_Print(monitor);
    if (file_ptr != NULL) {
        fputs(json_string, file_ptr);
        fputs("\r\n", file_ptr);
        //fclose(file_ptr);
    } else {
        perror("Error opening file");
    }
    //fprintf(file_ptr, "%s", json_string);
    cJSON_Delete(monitor);
    free(json_string);
}
