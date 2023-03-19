#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "helperFun.hpp"
#include <string>
#include "potato.h"
using namespace std;






int main(int argc, char *argv[])// machine name / port name
{
  int masterFd;
  int leftFd;
  int leftTranFd;
  int rightFd;
  int id;
  int numOfPlayer;
  potato dataPo;

  
  if (argc != 3) {
      cerr << "wrong argument!!" << endl;
      return EXIT_FAILURE;
  }


  // master socket
  masterFd = setUpSocket(argv[2], argv[1]);
  recv(masterFd, &id, sizeof(id), 0);
  recv(masterFd, &numOfPlayer, sizeof(numOfPlayer), 0);
  cout << "Connected as player " << id <<" out of " << numOfPlayer <<" total players" << endl;

  // left neighbor socket
  // cout << "socket for left" << endl;
  leftFd = setUpSocket("0", NULL); // server for left
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if(getsockname(leftFd, (struct sockaddr *)&sin, &len) == -1){
        cerr << "get server addr error";
        return EXIT_FAILURE;
  }
  int port = ntohs(sin.sin_port); ////////////////////////////
  char portNum[10];
  strcpy(portNum, to_string(port).c_str()); // convert int to char[]
  send(masterFd, portNum, sizeof(portNum), 0); // send the port num
  // cout << "my port num is" << portNum <<endl;

  // right neighbor socket
  // cout << "socketFor right\n";
  char ip[INET6_ADDRSTRLEN];
  char portForRight[10];

  recv(masterFd, ip, sizeof(ip), 0);
  recv(masterFd, portForRight, sizeof(portForRight), 0);
  // cout << "the port num on my right is" << portForRight <<endl;
  

  
  rightFd = setUpSocket(portForRight, ip);
  // cout << "connect to the right neighbor" << endl;

  // accept connection from left
  leftTranFd = accept(leftFd, (struct sockaddr *)&sin, &len);
  if(leftTranFd == -1){
    cerr << "connect failed" << endl;
  }else{
    // cout << "connect to left neighbor" << endl;
  }


  // select the source



  while(true){

    fd_set readfds;
    int max_fd = max(masterFd, max(leftTranFd, rightFd));
    FD_ZERO(&readfds);
    FD_SET(masterFd, &readfds);
    FD_SET(leftTranFd, &readfds);
    FD_SET(rightFd, &readfds);
    // cout << "waiting for something" << endl;
    if(select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1){
      cerr << "select error" << endl;
      return EXIT_FAILURE;
    }
    // choose the res fd
    // cout << "select fd" << endl;
    int resFd;
    if(FD_ISSET(masterFd, &readfds)){
      resFd = masterFd;
    }else if(FD_ISSET(leftTranFd, &readfds)){
      resFd = leftTranFd;
    }else if(FD_ISSET(rightFd, &readfds)){
      resFd = rightFd;
    }
    // cout << "finish fd" << endl;

    recv(resFd, &dataPo, sizeof(dataPo), 0);
    // cout << "finish revc" << endl;
    if(dataPo.numsOfhops == 0){ // when num is 0, this potato is close signal
      break;
    }


    
    if(dataPo.numsOfhops > 1){ // when num > 1, choose left or right to send
      if(numOfPlayer == 1){ // no need to send when there is only one player
        while(dataPo.numsOfhops != 1){
          dataPo.trace[dataPo.lengthOfTrace] = id; // attach id
          dataPo.lengthOfTrace++;
          dataPo.numsOfhops--;  // reduce num of hop
          cout << "Sending potato to " << id;
        }
        
        
      }else{

        dataPo.trace[dataPo.lengthOfTrace] = id; // attach id
        dataPo.lengthOfTrace++;
        dataPo.numsOfhops--;  // reduce num of hop

        srand( (unsigned int) time(NULL) + id);
        int random = rand() % 2;
        if(random = 0){
          cout << "Sending potato to " << (id - 1 + numOfPlayer) % numOfPlayer << endl;
          send(leftTranFd, &dataPo, sizeof(dataPo), 0);
        }else{
          cout << "Sending potato to " << (id + 1) % numOfPlayer << endl;
          send(rightFd, &dataPo, sizeof(dataPo), 0);
        }
        continue;
      }
    }



    if(dataPo.numsOfhops == 1){
      dataPo.trace[dataPo.lengthOfTrace] = id; // attach id
      dataPo.lengthOfTrace++;
      dataPo.numsOfhops--;  // reduce num of hop
      cout << "I'm it" << endl;
      send(masterFd, &dataPo, sizeof(dataPo), 0);
    }
  }

  close(masterFd);
  close(leftTranFd);
  close(leftFd);
  close(rightFd);




  return EXIT_SUCCESS;
}
