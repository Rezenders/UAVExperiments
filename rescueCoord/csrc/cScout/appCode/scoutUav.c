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

double dummyVictims[3][3]={{-27.604593, -48.521134, 10.0},
                         {-27.604531, -48.520998, 10.0},
                         {-27.604522, -48.520952, 10.0}};
const float FSMPeriod = 1.0;
double wayPoints[6][3] = {{-27.604508, -48.521061, 10.0},
                        {-27.604635, -48.521165, 10.0},
                        {-27.604661, -48.521090, 10.0},
                        {-27.604515, -48.520987, 10.0},
                        {-27.604531, -48.520914, 10.0},
                        {-27.604705, -48.520996, 10.0}};
const double scoutAlt = 10;
const double courierAlt = 20;
int buoys = 0;
int nRescued = 0;
int nVictims = 0;
double victims[5][3]; //MAXIMUM OF 5 VICTIMS
bool returningHome = false;
bool onRescue=false;

char * my_name;

time_t timer;

//client connection stuff
int sockfd, n, portno;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void takeOff(int status);
void goToP(double wp[]);
void returnHome();
bool amIScout(char name[]);
void sendState(int st, char name[]);
void sendVictim(double victimLocation[], char name[]);
void rescueVictim(double victimLocation[]);

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
        } else {
          if(message[0]=='v'){
            printf("GOT VICTIM\n");
            char * posChar;
            posChar = strtok(message, "(),");
            int i=0;
            while(posChar != NULL){
              if(strstr(posChar,"v")==NULL){
                victims[nVictims][i] = atof(posChar);
                i++;
              }
              posChar = strtok(NULL, "(),");
            }
            victims[nVictims][2]=courierAlt;
          }
          nVictims++;
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
            if(amIScout(my_name))
            {
              printf("TAKING OFF\n");
              if(status==Flying){
                goToP(wayPoints[0]);
                state=1;
              }else{
                takeOff(status);
              }
            }
            break;
          }

          case 1 :
          {
            if(amIScout(my_name) && (distance(pos, wayPoints[0]) < 3))
            {
              goToP(wayPoints[1]);
              state=2;
            }
            break;
          }

          case 2 :
          {
            if(amIScout(my_name) && (distance(pos, wayPoints[1]) < 3))
            {
              goToP(wayPoints[2]);
              state=3;
            }
            break;
          }

          case 3 :
          {
            if(amIScout(my_name) && (distance(pos, wayPoints[2]) < 3))
            {
              goToP(wayPoints[3]);
              state=4;
            }
            break;
          }

          case 4 :
          {
            if(amIScout(my_name) && (distance(pos, wayPoints[3]) < 3))
            {
              goToP(wayPoints[4]);
              state=5;
            }
            break;
          }

          case 5 :
          {
            if(amIScout(my_name) && (distance(pos, wayPoints[4]) < 3))
            {
              goToP(wayPoints[5]);
              state=6;
            }
            break;
          }

          case 6 :
          {
            if(amIScout(my_name) && (distance(pos, wayPoints[5]) < 3))
            {
              returnHome();
              state=7;
            }
            break;
          }

          case 7 :
          {
            if(amIScout(my_name) && !returningHome)
            {
              returnHome();
            } else state=8;
            break;
          }

          default : ;
        }

        if(amIScout(my_name)){ //if i'm a scout
          if(state>0){
            int i;
            for(i=nVictims; i<sizeof(dummyVictims)/sizeof(dummyVictims[0]); i++){
              if(distance(pos, dummyVictims[i]) < 3){
                sendVictim(dummyVictims[i],"courier");
                nVictims++;
              }
            }
          }
          sendState(state, "courier");
        }else{ //if i'm a courier
          if(state>6 && nRescued>=nVictims){
            returnHome();
            printf("Rescued total of %d victims\n", nRescued);
          } else {
            if(nRescued<nVictims && !onRescue){
              takeOff(status);
              rescueVictim(victims[nRescued]);
            }
          }
          if(status == NotReady){
            buoys=1;
          }
          if(buoys>0 & distance(pos, victims[nRescued]) < 3){
            printf("Delivering Buoy to %d!\n",nRescued+1);
            buoys=0;
            nRescued++;
            onRescue=false;
            returnHome();
          }
          sendState(state, "scout");
        }

        if(status == NotReady){
          returningHome = false;
        }
      }

      usleep(100000);
    }

    close(sockfd);
    return 0;
}

void takeOff(int status){
  if(status != Flying){
    write(sockfd, "!launch\n" , strlen("!launch\n"));
  }
}

void goToP(double wp[]){
  printf("Going to P at %f, %f, %f\n", wp[0], wp[1], wp[2]);
  static char m[100];
  sprintf(m, "!setWaypoint(%f,%f,%f)\n", wp[0], wp[1], wp[2]);
  write(sockfd, m , strlen(m));
}

void returnHome(){
  if(!returningHome){
    printf("Returning Home\n");
    returningHome=true;
    write(sockfd, "!returnHome\n", strlen("!returnHome\n"));
  }
}

bool amIScout(char name[]){
  if(strncmp(name,"scout", 5)==0){
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

void sendVictim(double victimLocation[], char name[]){
  static char m[64];
  sprintf(m, "*%s,v(%f,%f)\n",name,victimLocation[0], victimLocation[1]);
  write(sockfd, m , strlen(m));
}

void rescueVictim(double victimLocation[]){
  if(buoys<1){
    returnHome();
  }else if(!onRescue){
      goToP(victimLocation);
      onRescue=true;
  }
}
