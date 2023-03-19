#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include "potato.h"
#include "helperFun.hpp"

using namespace std;

potato dataPo;
int numOfPlayers;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


// connect players and send info of left and right neighbor to every player
void connectToAllPlayer(int socket_fd, vector<int>& playerFd){
  // connect all and store the addrInfo of players
  // store the IP and port of players' receive socket
  vector<pair<string, string>> playerAddrInfos;

  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);

  int clientFd;

  // accept each player and send the ID back
  for(int i = 0; i < numOfPlayers; i++){
    clientFd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if(clientFd == -1){
      cerr << "connect to player " << i << " failed!" << endl;
      i--;
      continue;
    }else{
      cout << "Player " << i << " is ready to play" << endl;
    }

    // translate the addr
    char s[INET6_ADDRSTRLEN];
    inet_ntop(socket_addr.ss_family,
                  get_in_addr((struct sockaddr *)&socket_addr),
                  s, sizeof(s));
    // send id and total num
    send(clientFd, &i, sizeof(int), 0);
    send(clientFd, &numOfPlayers, sizeof(int), 0);

    // receive the portnum
    char port[10];
    recv(clientFd, &port, sizeof(port), 0);
    // cout << "the ip for client is" << s << endl;
    // cout << "the port Num for client is" << port << endl;
    string tempIp = s;
    string tempPort = port;

    playerFd.push_back(clientFd);
    playerAddrInfos.push_back({tempIp, tempPort});

  }

  // send next neighbor the i player
  for(int i = 0; i < numOfPlayers; i++){
    char ip[INET6_ADDRSTRLEN];
    char port[10];
    strcpy(ip, playerAddrInfos[(i + 1) % numOfPlayers].first.c_str());
    strcpy(port, playerAddrInfos[(i + 1) % numOfPlayers].second.c_str());
    // cout << "send "<< playerAddrInfos[(i + 1) % numOfPlayers].first << "to player\n";

    send(playerFd[i], ip, sizeof(ip), 0);
    send(playerFd[i], port, sizeof(port), 0);

  }

}




int main(int argc, char *argv[]) // arguments: port num / num of player / num of hops
{

  // argument process
  if(argc != 4){
    cerr << "wrong arguments!!";
    return EXIT_FAILURE;
  }

  // need more error check here *****************8
  dataPo.numsOfhops = stoi(argv[3], NULL, 10);
  dataPo.lengthOfTrace = 0;
  numOfPlayers = stoi(argv[2], NULL, 10);
  vector<int> playerFd; // store the FD of players
  if(numOfPlayers <= 1){
    return EXIT_FAILURE;
  }
  if(dataPo.numsOfhops < 0 || dataPo.numsOfhops > 512){
    return EXIT_FAILURE;
  }

  cout << "Potato Ringmaster" << endl;
  cout << "Players = " << numOfPlayers << endl;
  cout << "Hops = " << dataPo.numsOfhops << endl;

  // set up socket and listen
  int socketFd;
  socketFd = setUpSocket(argv[1], NULL);


  // accept the connect request from all server and send the right neighbor's port and ip back
  connectToAllPlayer(socketFd, playerFd);
  // cout << "player now in the ring";



  // add all the player fd to the fd_set
  fd_set readfds;
  int max_fd = playerFd[0];
  FD_ZERO(&readfds);
  for(int i = 0; i < numOfPlayers; i++){
    if(playerFd[i] > max_fd)max_fd = playerFd[i];
    FD_SET(playerFd[i], &readfds);
  }



  // randomly find a player and send to potato to him
  srand( (unsigned int) time(NULL));
  int randomPlayer = rand() % numOfPlayers;

  cout << "Ready to start the game, sending potato to player "<< randomPlayer << endl;
  send(playerFd[randomPlayer], &dataPo, sizeof(dataPo), 0);

  // wait for responce
  if(select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1){
    cerr << "select error!!" << endl;
    return EXIT_FAILURE;
  }

  int lastPlayer;// find the responce player and recv
  for(int i = 0; i < numOfPlayers; i++){ 
    if(FD_ISSET(playerFd[i], &readfds)){
      lastPlayer = i;
      break;
    }
  }
  recv(playerFd[lastPlayer], &dataPo, sizeof(dataPo), 0);

  // print the info from potato
  cout <<"Trace of potato:" <<endl;
  for(int i = 0; i < dataPo.lengthOfTrace; i++){
    cout << dataPo.trace[i];
    if(i != dataPo.lengthOfTrace - 1){
      cout << ",";
    }
  }
  if(dataPo.numsOfhops != 0){
    cerr << "respond error!!" << endl;
    return EXIT_FAILURE;
  }
  
  for(int i = 0; i < numOfPlayers; i++){
    send(playerFd[i], &dataPo, sizeof(dataPo), 0);
    close (playerFd[i]);
  }

  close(socketFd);

  return EXIT_SUCCESS;
}
