#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include "read_line.h"
#include "geo_distance.h"

#define NotReady 0
#define Ready 1
#define Flying 2

const float FSMPeriod = 5.0;
double wayPoint[] = {-27.604635, -48.521165, 0};
const double aliceAlt = 10;
const double bobAlt = 20;

char * my_name;

time_t timer;

//client connection stuff
int sockfd, n, portno;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void takeOff();
void goToP(double wp[]);
void returnHome();
bool amIAlice(char name[]);
void sendState(int st, char name[]);

int main(int argc, char *argv[])
{
    usleep(5000000);
    printf("Started\n");
    //CLIENT CONNECTION STUFF
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if(argc<4){
      fprintf(stderr, "usage: %s hostname port\n", argv[0]);
    }

    my_name=argv[3];
    strcat(my_name,"\n");

    if(amIAlice(my_name)){
      wayPoint[2] = aliceAlt;
      printf("Using Alice Alt\n");
    } else{
      wayPoint[2] = bobAlt;
      printf("Using Bob Alt\n");
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) error("ERROR opening socket\n");

    server = gethostbyname(argv[1]);
    if(server == NULL) {
      fprintf(stderr, "ERROR, no such host\n");
      exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0) error("ERROR connecting\n");

    write(sockfd, my_name, strlen(my_name));

    //UAV STUFF
    double pos[] = {0, 0, 0};
    int status = NotReady;
    int state = 0;
    bool connected = false; //connected to the other UAV?

    timer = time(NULL);

    printf("Connection established\n");

    while(1)
    {
      //receive and parse message
      n=readLine(sockfd,buffer,sizeof(buffer));


      if(strstr(buffer, "pos") != NULL) //if these are percepts
      {
        unsigned int nToSep = strcspn(buffer,";");
        unsigned int nToNextSep;
        char percept[256];

        strncpy(percept, buffer, nToSep);

        //printf("got a percept: %s\n", buffer);
        while (percept[0] != 0)
        {
          if(strstr(percept, "pos"))
          {
            char * posChar;
            posChar = strtok(percept, "(),");
            int i=0;
            while(posChar != NULL){
              if(strstr(posChar,"pos")==NULL){
                pos[i] = atof(posChar);
                i++;
              }
              posChar = strtok(NULL, "(),");
            }
          }

          else if(strstr(percept, "status"))
          {
            char * posChar;
            posChar = strtok(percept, "(),");
            while(posChar != NULL){
              if(strstr(posChar,"notReady")!=NULL){
                status = NotReady;
              }else if(strstr(posChar,"ready")!=NULL){
                status = Ready;
              }else if(strstr(posChar,"flying")!=NULL){
                status = Flying;
              }
              posChar = strtok(NULL, "(),");
            }
          }

          else if(strstr(percept,"connected"))
          {
            connected=true;
          }else connected=false;

          if(nToSep<strlen(buffer)-2){//disregard the last ';\n'
            nToNextSep = strcspn(buffer+nToSep+1,";");
            strncpy(percept, buffer+nToSep+1, nToNextSep);
            nToSep += nToNextSep+1;
          }else{
            memset(percept,0,sizeof(percept));
          }

        }

      }else if(strstr(buffer, "*") != NULL)
      {
        printf("Got message %s\n",buffer);
        static char message[100];
        strncpy(message, buffer+1, strlen(buffer));
        if(isdigit(message[0])){
          if(atoi(message) > state){
              printf("STATE IS NOW IS %d\n", atoi(message));
              state = atoi(message);
          }
        }
      }

      //FSM
      //if state X -> do Y;
      if((time(NULL)-timer)>FSMPeriod && connected){
        timer = time(NULL);
        printf("running FSM with state %d\n",state);
        switch(state){
          case 0 :
          {
            if(amIAlice(my_name))
            {
              printf("TAKING OFF\n");
              if(status==Flying){
                goToP(wayPoint);
                state=1;
              }else{
                takeOff();
              }
            }
            break;
          }

          case 1 :
          {
            if(amIAlice(my_name) && (distance(pos, wayPoint) < 5))
            {
              state=2;
            }
            break;
          }

          case 2 :
          {
            if(!amIAlice(my_name))
            {
              if(status==Flying){
                goToP(wayPoint);
                state=3;
              }else{
                takeOff();
              }
            }
            break;
          }

          case 3 :
          {
            if(!amIAlice(my_name) && (distance(pos, wayPoint) < 5))
            {
              state=4;
            }
            break;
          }

          case 4 :
          {
            if(amIAlice(my_name)){
                     returnHome();
                     state=5;
            }
            break;
          }

          case 5 :
          {
            if(amIAlice(my_name) && (status != Flying)){
                     state=6;
            }
            break;
          }

          case 6 :
          {
            if(!amIAlice(my_name)){
                     returnHome();
                     state=7;
            }
            break;
          }

          case 7 :
          {
            if(!amIAlice(my_name) && (status != Flying)){
                     state=8;
            }
            break;
          }

          default : printf("Defaulting FSM!!!\n");
        }

        if(amIAlice(my_name)){
          sendState(state, "bob");
        }else{
          sendState(state, "alice");
        }
      }

      usleep(100000);
    }

    close(sockfd);
    return 0;
}

void takeOff(){
  write(sockfd, "!launch\n" , strlen("!launch\n"));
}

void goToP(double wp[]){
  printf("Going to P at %f, %f, %f\n", wp[0], wp[1], wp[2]);
  static char m[100];
  sprintf(m, "!setWaypoint(%f,%f,%f)\n", wp[0], wp[1], wp[2]);
  write(sockfd, m , strlen(m));
}

void returnHome(){
  printf("Returning Home\n");
  write(sockfd, "!returnHome\n", strlen("!returnHome\n"));
}

bool amIAlice(char name[]){
  if(strncmp(name,"alice", 3)==0){
    return true;
  } else {
    return false;
  }
}

void sendState(int st, char name[]){
  static char m[10];
  sprintf(m, "*%s,%d\n",name,st);
  write(sockfd, m , strlen(m));
}
