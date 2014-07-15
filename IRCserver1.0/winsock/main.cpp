#include "SOCKET.h"

using namespace std;


/* Global variables */
Room serverData[MAX_ROOMS];

/* Set all array elements in argu1 to 0 */
void initialize_rooms(Room serverData[MAX_ROOMS]){
	for(int i=0;i<MAX_ROOMS;i++){
		serverData[i].setRoomId(0);
	}
}

/* Prepare winsock for socket use */
void initialize_winsock(WSADATA& wsaData, addrinfo& hints, addrinfo** result){

	/* Set winsock2 version */
    WSAStartup(MAKEWORD(2,2), &wsaData);

	/* Clear memory */
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    /* Resolve the server address and port */
    getaddrinfo(NULL, DEFAULT_PORT, &hints, result);

}

/* Reset buffers to zero */
void initialize_buffers(char recvbuf[DEFAULT_BUFLEN], int& recvbuflen, int& iResult, int& iSendResult){

	// set buffer array elements to zeros
	for(int i=0;i<DEFAULT_BUFLEN;i++){
		recvbuf[i]=0;
	}

	// set buffer len to zero
	recvbuflen=DEFAULT_BUFLEN;

	// set result and send set to zero
	iResult=0;
	iSendResult=0;

}

/* Create a SOCKET and bind to TCP listening socket */
void set_socketandbind(SOCKET& ListenSocket, addrinfo*& result){

	/* Create a SOCKET and bind to TCP listening socket */
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    listen(ListenSocket, SOMAXCONN);

}

/* Find an empty game room, return == -1 if room not found, else != 1*/
int findEmptyRoom(Room serverData[MAX_ROOMS]){

	// find empty room
	for(int i=0;i<MAX_ROOMS;i++){
		if(serverData[i].getRoomId()==0){
			return i;
		}
	}

	return -1;
}

/* Does room exist at argu2's room id? if exists, return != -1, else, return == -1  */
int isThisRoomAlive(Room serverData[MAX_ROOMS], int foundRoom){

	/* Does room exist? */
	for(int i=0;i<MAX_ROOMS;i++){
		cout << "i: " << i << " | " << "=" << serverData[i].getRoomId() << " || looking for: " << foundRoom << endl;
		if(serverData[i].getRoomId()==foundRoom){
			return i;
		}
	}

	return -1;
}

/* Return a new unique random room id */
int getUniqueRoomId(){

	int id;

	bool passed=false;
	while(!passed){

		id=rand()%999999+1;

		passed=true;
		for(int i=0;i<MAX_ROOMS;i++){
			if(serverData[i].getRoomId()==id){
				passed=false;
			}
		}

	}

	return id;
}

/* Is user in a specific room? */
int isUserInRoom(int foundRoom, std::string& foundUser){
	for(int i=0;i<serverData[foundRoom].users.size();i++){
		cout << "strt: " << i << " || room #: " << foundRoom << endl; 
		if(serverData[foundRoom].users.at(i).getUsername()==foundUser){
			cout << "end: " << i << endl; 
			return -1;
		}
		cout << "finish: " << i << endl; 
	}

	return 0;
}

/* Is there room to create another room? */
bool isRoomSlotAvailable(){

	for(int i=0;i<MAX_ROOMS;i++){
		if(serverData[i].getRoomId()==0){
			return true;
		}
	}

	return false;
}

/* Return player one data to server */
std::string PrepareUserList(int foundRoom, std::string username){
	cout << "founddd: " << foundRoom << " || username: " << username << endl;
	if(foundRoom==2){
		cout << "begin" << endl;
	}

	//variables
	std::string t="";

	//fill buffer with users (U=USERS)
	t += "U:";
	for(int i=0;i<serverData[foundRoom].users.at(serverData[foundRoom].getCounterFromUsername(username)).sizeOfUserQueue();i++){
		t += ",";
		t += serverData[foundRoom].users.at(serverData[foundRoom].getCounterFromUsername(username)).dequeueUser();
		t += ",";
	}
	
	if(foundRoom==2){
		cout << "middle" << endl;
	}

	//fill buffer with messages (M=MESSAGES)
	t += "M:";
	for(int i=0;i<serverData[foundRoom].users.at(serverData[foundRoom].getCounterFromUsername(username)).sizeOfMessageQueue();i++){
		cout << "Size of message queue: " << serverData[foundRoom].users.at(serverData[foundRoom].getCounterFromUsername(username)).sizeOfMessageQueue() << endl;
		t += ",";
		t += serverData[foundRoom].users.at(serverData[foundRoom].getCounterFromUsername(username)).dequeue();
		t += ",";
	}

	if(foundRoom==2){
		cout << "begin-end" << endl;
	}

	//cout << "Sent buffer: " << t << endl;
	return t;
}

/* Cleanup */
void cleanUp(ClientParams* lpParam){

	/* Cleanup and shutdown the connection since we're done */
	//shutdown(lpParam->getSocket(), SD_SEND);
	closesocket(lpParam->getSocket());
	//WSACleanup();

	/* delete parameters */
	delete (lpParam);

	/* Exit thread */
	//ExitThread(0);

}

/* Game loop */
void gameLoop(bool& closeConnection, ClientParams* lpParam, int& foundRoom, int iResult, int iSendResult, char recvbuf[DEFAULT_BUFLEN], int recvbuflen, int& tem){
	cout << "found3: " << foundRoom << endl;
	// connect to room
	while(!closeConnection){
		//cout << "Counter: " << tem << endl;
		//debug
		//cout << "\nClient's User id: " << ((ClientParams*)lpParam)->getUniqueUserId() << endl;
		//cout << "\nRoom id: " << ((ClientParams*)lpParam)->getHostRoomId() << endl;
		
		// clear the set ahead of time
		//FD_ZERO(&readfds);
		// add our descriptors to the set
		//FD_SET(((ClientParams*)lpParam)->getSocket(), &readfds);

		// wait until either socket has data ready to be recv()d (timeout 10.5 secs)
		//tv.tv_usec = 100000;

		u_long iMode=1;
		ioctlsocket(lpParam->getSocket(),FIONBIO,&iMode);

		////select7
		//int rv = select(((ClientParams*)lpParam)->getSocket()+1, &readfds, NULL, NULL, &tv);
		//
		//if (rv == -1) {
		//	//perror("select"); // error occurred in select()
		//}else if (rv == 0){
		//	//printf("Timeout occurred! No data after 0.1 seconds.\n");
		//}else{
		//	perror("success");
		//	// one or both of the descriptors have data
		//	if (FD_ISSET(((ClientParams*)lpParam)->getSocket(), &readfds)) {
		//		//receive and store user data
		//		iResult = recv(((ClientParams*)lpParam)->getSocket(), recvbuf, recvbuflen, 0);
		//	}
		//}

		std::string sendBuffer;
		sendBuffer=PrepareUserList(foundRoom, lpParam->getUniqueUserId() );
		iSendResult = send( lpParam->getSocket(), sendBuffer.c_str(), (int)(strlen(sendBuffer.c_str())+1), 0 );
		cout << "Sent: " << sendBuffer.c_str() << endl;

		//receive and store user data
		iResult = recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);
		if (iResult > 0) {

			//cout << "Received: " << recvbuf << " || Length: " << iResult << endl << endl; 
			if(recvbuf!=""){	

				//add message to all users' message list
				for(int i=0;i<serverData[foundRoom].users.size();i++){
					//serverData[foundRoom].users.at(i).enqueue(recvbuf);
				}
				
			}

		}else if (iResult == 0){
			printf("Connection closing...\n");
			
			//delete user from lists
			serverData[foundRoom].deleteUser((lpParam->getUniqueUserId()));

			//delete room if no users in room
			if(serverData[foundRoom].users.size()==0){
				serverData[foundRoom].deleteRoom();
			}

			//close connection
			closeConnection=true;

		}else{
			//printf("recv failed with error: %d\n", WSAGetLastError());

			if(WSAGetLastError()==WSAECONNRESET){

				//delete user from lists
				serverData[foundRoom].deleteUser((lpParam->getUniqueUserId()));

				//delete room if no users in room
				if(serverData[foundRoom].users.size()==0){
					serverData[foundRoom].deleteRoom();
				}

				//close connection
				closeConnection=true;
			}
		}

		//cout << endl;
		tem++;
		Sleep(500);
	}
	
}

/* Connect to room */
void connectToRoom(int& iResult, bool& exit, ClientParams* lpParam, char recvbuf[DEFAULT_BUFLEN], int recvbuflen,std::string& tempFoundUser, int& tempFoundRoom, int& foundRoom, int& userCounter, int& iSendResult){

	// repeat until Host or Join is chosen correctly
	while(!exit){

		// Dont exit until menu option is chosen
		exit=false;
		
		// Recv (host, join)
		iResult=recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);
		cout << "Recv: " << recvbuf << endl;

		int commaPos=std::string(recvbuf).find(',');
		tempFoundRoom=atoi(std::string(recvbuf).substr(0,commaPos).c_str());
		tempFoundUser=std::string(recvbuf).substr(commaPos+1,std::string(recvbuf).size());

		//cout << "User: " << tempFoundUser << " | Room: " << tempFoundRoom << endl;

		//std::stringstream ss(recvbuf);
		//int x;
		//ss >> x;
		//tempFoundRoom=x;

		//get tempFoundUser
		lpParam->setHostRoomId(getUniqueRoomId());
		int temp=lpParam->getHostRoomId();
		std::string tempStr;
		itoa(temp,(char*)tempStr.c_str(),10);

		send(lpParam->getSocket(), tempStr.c_str(), strlen(tempStr.c_str())+2, 0);
		cout << "Sent: " << tempStr.c_str() << endl;

		if(iResult>0){
			
			if((std::string)recvbuf=="host"){
				cout << "Selected Host \n";

				if(isRoomSlotAvailable()){

					iResult=recv(((ClientParams*)lpParam)->getSocket(), recvbuf, recvbuflen, 0);
					cout << "Recv: " << recvbuf << endl;

					// set additional parameters
					//lpParam->setHostRoomId(getUniqueRoomId());
					lpParam->setUniqueUserId(recvbuf);
				
					// find empty room if available
					int found_empty_room=findEmptyRoom(serverData);
				
					//set room and user id
					foundRoom=found_empty_room;

					// add room id
					//cout << "Room: " << found_empty_room << " || " << lpParam->getHostRoomId() << endl;
					serverData[found_empty_room].setRoomId(lpParam->getHostRoomId());

					// add this user to room and set it's id
					userCounter=serverData[found_empty_room].addUser( (lpParam->getUniqueUserId()) );

					/* Initialize buffers */
					initialize_buffers(recvbuf, recvbuflen, iResult, iSendResult);

					send(lpParam->getSocket(), "1", 3, 0);
					cout << "Sent: " << "1" << endl;

					// exit menu
					exit=true;	

				}else{

					iResult=recv(((ClientParams*)lpParam)->getSocket(), recvbuf, recvbuflen, 0);
					cout << "Recv: " << recvbuf << endl;

					send(lpParam->getSocket(), "3", 3, 0);

				}

			}else{
				cout << "Selected Join: " << tempFoundRoom << " \n";

				// check if room exists, if so, set room id
				foundRoom=isThisRoomAlive(serverData, tempFoundRoom);
				int foundUser=0;
				if(foundRoom!=-1){
					foundUser=isUserInRoom(foundRoom,tempFoundUser);
				}
				cout << "Foundroom='" << foundRoom << "' || Founduser: '" << foundUser << "'" << endl;

				// if room found, continue, else, error
				if( (foundRoom!=-1) && (foundUser!=-1) ){

					// if space in room found, connect, else, error
					if(serverData[foundRoom].isAvailableUserSlot()){
						
						iResult=recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);

						// create parameters
						((ClientParams*)lpParam)->setHostRoomId(atoi(recvbuf));
						((ClientParams*)lpParam)->setUniqueUserId(recvbuf);

						// add this user to room and setting it's id
						userCounter=serverData[foundRoom].addUser( (lpParam->getUniqueUserId()) );

						/* Initialize buffers */
						initialize_buffers(recvbuf, recvbuflen, iResult, iSendResult);
						
						//send(lpParam->getSocket(), recvbuf, recvbuflen, 0);
						send(lpParam->getSocket(), "1", 3, 0);
						cout << "Sent: " << "1" << endl;

						// exit menu
						exit=true;

					}else{
						iResult=recv(((ClientParams*)lpParam)->getSocket(), recvbuf, recvbuflen, 0);
						/* Initialize buffers */
						//initialize_buffers(recvbuf, recvbuflen, iResult, iSendResult);

						send(lpParam->getSocket(), "2", 3, 0);
					}

				}else{
					iResult=recv(((ClientParams*)lpParam)->getSocket(), recvbuf, recvbuflen, 0);
					cout << "Recv: " << recvbuf << endl;

					/* Initialize buffers */
					//initialize_buffers(recvbuf, recvbuflen, iResult, iSendResult);
					
					send(lpParam->getSocket(), "3", 3, 0);
				}

			}
			
		}else if (iResult == 0){
			//printf("Connection closing...\n");
		}else{
			printf("recv failed with error: %d\n", WSAGetLastError());

			if(WSAGetLastError()==WSAECONNRESET){
				cleanUp(lpParam);

				/* Exit thread */
				ExitThread(0);
			}

			//closesocket(ClientSocket);
			//WSACleanup();
			//return 1;
		}
		
		if(exit){
			cout << "Fill start" << endl;
			//fill initial list of user
			for(int i=0;i<serverData[foundRoom].users.size();i++){
				cout << "Fill: " << i << endl;
				serverData[foundRoom].users.at( serverData[foundRoom].getCounterFromUsername((lpParam->getUniqueUserId())) ).enqueueUserStatus("+"+serverData[foundRoom].users.at(i).getUsername());
			}
			cout << "Fill end" << endl;
		}
	}
	cout << "START - Room: " << foundRoom << " || Room ID: " << ((ClientParams*)lpParam)->getHostRoomId() << endl;

	cout << "~~~~~~~~~~~~ROOMS~~~~~~~~~~~~" << endl;
	cout << "Room #1: " << serverData[0].getRoomId() << " || Num of users: " << serverData[0].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #2: " << serverData[1].getRoomId() << " || Num of users: " << serverData[1].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #3: " << serverData[2].getRoomId() << " || Num of users: " << serverData[2].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #4: " << serverData[3].getRoomId() << " || Num of users: " << serverData[3].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #5: " << serverData[4].getRoomId() << " || Num of users: " << serverData[4].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #6: " << serverData[5].getRoomId() << " || Num of users: " << serverData[5].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #7: " << serverData[6].getRoomId() << " || Num of users: " << serverData[6].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #8: " << serverData[7].getRoomId() << " || Num of users: " << serverData[7].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #9: " << serverData[8].getRoomId() << " || Num of users: " << serverData[8].users.size() << "/" << MAX_USERS << endl;
	cout << "Room #10: "<< serverData[9].getRoomId() << " || Num of users: " << serverData[9].users.size() << "/" << MAX_USERS << endl;
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

/* Loop menu options until a room with space if found, then connect */
DWORD WINAPI processClient(LPVOID lpParam){

	/* variables */
    int iResult, iSendResult; // iSend??
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
	bool exit=false;
	int foundRoom, tempFoundRoom;
	std::string tempFoundUser;
	fd_set readfds;	struct timeval tv;
	int userCounter;
	bool closeConnection=false;

	int tem=0;

	/* initialize random seed */
    srand(time(NULL));


	/* Connect to room with room+user credentials */
	connectToRoom(iResult,exit,((ClientParams*)lpParam),recvbuf,recvbuflen,tempFoundUser,tempFoundRoom,foundRoom,userCounter,iSendResult);
	
	/* Game loop */
	gameLoop(closeConnection,((ClientParams*)lpParam),foundRoom,iResult,iSendResult,recvbuf,recvbuflen,tem);

	/* Cleanup */
	cleanUp(((ClientParams*)lpParam));

	return 0;
}

int main(void){
	
	/* Server variables */
    WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo hints;

	printf("~SERVER~\n");

	/* initialize rooms */
	initialize_rooms(serverData);
    
    /* Initialize Winsock*/
	initialize_winsock(wsaData, hints, &result);

	/* Set socket and bind to TCP listening socket */
	set_socketandbind(ListenSocket, result);

	//int flag = 1;
 //   setsockopt(ListenSocket,            /* socket affected */
 //       IPPROTO_TCP,     /* set option at TCP level */
 //       TCP_NODELAY,     /* name of option */
 //       (char *) &flag,  /* the cast is historical cruft */
 //       sizeof(int));    /* length of option value */

	/* Receive until the peer shuts down the connection */
	DWORD i=0;
    do{

		// Accept a client socket
		ClientSocket=accept(ListenSocket, NULL, NULL);
        
		// create parameters
		ClientParams *params = new ClientParams();
		params->setSocket(ClientSocket);

		// join room
		CreateThread(
			NULL,                       // don't inherit handle
			0,                          // use default size for the executable
			processClient,
			(LPVOID)params,				// thread data
			0,                          // run right away
			&i							// return thread id
			);

    }while(true);

	/* Unset server socket */
    closesocket(ListenSocket);

	system("pause");
    return 0;
}