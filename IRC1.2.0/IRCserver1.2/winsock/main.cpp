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

/* Create a SOCKET and bind to TCP listening socket */
void set_socketandbind(SOCKET& ListenSocket, addrinfo*& result){

	/* Create a SOCKET and bind to TCP listening socket */
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    listen(ListenSocket, SOMAXCONN);

}

/* Find an empty game room, return == -1 if room not found, else != 1*/
int findEmptyRoom(){

	// find empty room
	for(int i=0;i<MAX_ROOMS;i++){
		if(serverData[i].getRoomId()==0){
			return i;
		}
	}

	return -1;
}

/* Does room exist at argu2's room id? */
bool doesRoomExist(int foundRoom){

	/* Does room exist? */
	for(int i=0;i<MAX_ROOMS;i++){
		if(serverData[i].getRoomId()==foundRoom){
			return true;
		}
	}

	return false;
}

/* Is user in a specific room? */
int isUserInRoom(int foundRoom, std::string& foundUser){

	for(int i=0;i<serverData[foundRoom].getUserListSize();i++){
		
		if(serverData[foundRoom].getUsername(i)==foundUser){
			return -1;
		}
		
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

/* Is there an available user slot in room? */
bool isRoomUserSlotAvailable(int roomNumber){

	int roomSlot=0;

	// find room slot
	for(int i=0;i<MAX_ROOMS;i++){
		if(serverData[i].getRoomId()==roomNumber){
			roomSlot=i;
		}
	}
	
	//if free user slots exists
	if(serverData[roomSlot].getUserListSize() < MAX_USERS ){
		return true;
	}else{
		return false;
	}
	
}

/* Return room slot of a given room id */
/* precondition: argu1 is an id of one of the rooms*/
int findRoomSlotFromRoomId(int roomId){
	
	int roomSlot=0;

	for(int i=0;i<MAX_ROOMS;i++){
		if(serverData[i].getRoomId()==roomId){
			roomSlot=i;
		}
	}
	
	return roomSlot;

}

/* Return a packet scheme string containing the client-server changes such as users that left and joined room and new messages to room */
std::string prepareUserAndMessageList(int foundRoomSlot, std::string username){

	//variables
	std::string t="";

	//fill buffer with users (U=USERS)
	t += "U:";
	for(int i=0;i<serverData[foundRoomSlot].sizeOfUserQueue(serverData[foundRoomSlot].getUserIndexFromUserList(username));i++){
		t += ",";
		t += serverData[foundRoomSlot].dequeueUserStatus(serverData[foundRoomSlot].getUserIndexFromUserList(username));
		t += ",";
	}
	
	//fill buffer with messages (M=MESSAGES)
	t += "M:";
	for(int i=0;i<serverData[foundRoomSlot].sizeOfMessageQueue(serverData[foundRoomSlot].getUserIndexFromUserList(username));i++){
		cout << "Size of message queue: " << serverData[foundRoomSlot].sizeOfMessageQueue(serverData[foundRoomSlot].getUserIndexFromUserList(username)) << endl;
		t += ",";
		t += serverData[foundRoomSlot].dequeueMessage(serverData[foundRoomSlot].getUserIndexFromUserList(username));
		t += ",";
	}

	//return lists
	return t;
}

/* Once we found a room for user to join or create, send list of all the people in that room at the time of entry so they can populate their user list */
void fillUserList(int foundRoomSlot, std::string username){
	// enqueue a list of user added messages according to the amount of people in that room
	for(int i=0;i<serverData[foundRoomSlot].getUserListSize();i++){
		serverData[foundRoomSlot].enqueueUserStatus(serverData[foundRoomSlot].getUserIndexFromUserList(username), "+"+serverData[foundRoomSlot].getUsername(i));
	}
}

/* Display room debug information */
void printDebug(){

	cout << "~~~~~~~~~~~~ROOMS~~~~~~~~~~~~";
	for(int i=0; i<MAX_ROOMS; i++){
		cout << "Room #" << i << " : " << serverData[i].getRoomId() << " || Num of users: " << serverData[i].getUserListSize() << "/" << MAX_USERS;
	}
	cout << endl << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
	cout << endl;

}

/* Cleanup */
void cleanUp(ClientParams* lpParam){

	/* Cleanup and shutdown the connection since we're done */
	closesocket(lpParam->getSocket());

	/* delete parameter memory */
	delete (lpParam);

	/* Exit thread */
	ExitThread(0);

}

/* Room loop */
void roomLoop(ClientParams* lpParam){

	/* variables */
	bool closeConnection=false;
	char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen=DEFAULT_BUFLEN;	int iResult;

	/* Join room */
	while(!closeConnection){

		/* Non-blocking socket */
		u_long iMode=1;
		ioctlsocket(lpParam->getSocket(),FIONBIO,&iMode);

		/* Send current list of room's users and messages to user if list has updated */
		std::string sendBuffer=prepareUserAndMessageList(lpParam->getHostRoomSlot(), lpParam->getUniqueUserId() );
		if(sendBuffer != DEFAULT_PACKET_SCHEME){
			cout << "Sent: " << sendBuffer << " \\ To: " << lpParam->getUniqueUserId() << endl;
			send( lpParam->getSocket(), sendBuffer.c_str(), (int)(strlen(sendBuffer.c_str())+1), 0 );
		}

		/* Receive user's new message to be added to room */
		iResult = recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);

		// if packet is free of error, else if closed connection by client
		if (iResult > 0) {

			// if packet contains data
			if(recvbuf!=""){
				// add message to all users' message list
				for(int i=0;i<serverData[lpParam->getHostRoomSlot()].getUserListSize();i++){
					serverData[lpParam->getHostRoomSlot()].enqueueMessage(i, recvbuf);
				}
			}
			
		}else if ((iResult == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) || WSAGetLastError()==NULL){
			printf("Connection closing...\n");
			
			//delete user from list in room
			serverData[lpParam->getHostRoomSlot()].deleteUser((lpParam->getUniqueUserId()));
			
			//delete room if user was last to leave room
			if(serverData[lpParam->getHostRoomSlot()].getUserListSize()==0){
				serverData[lpParam->getHostRoomSlot()].deleteRoom(); 
			}

			//close connection
			closeConnection=true;

			//print debug
			printDebug();

		}

		//wait - to ensure send() doesnt send too fast and consequently gets blocked
		Sleep(200);

	}
	
}

/* Connect to room */
void connectToRoom(ClientParams* lpParam){

	/* variables */
	bool exit=false;
	char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen=DEFAULT_BUFLEN;	int iResult;

	/* Menu selection - repeat until user has successfully chosen Join or Host */
	while(!exit){

		// dont exit until menu option is chosen
		exit=false;
		
		// recv (0=host, 1=join)
		recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);
		cout << "Recv: " << recvbuf << endl;

		//0=host room attempt, 1=join room attempt
		if(recvbuf[0]=='0'){
			
			//check to make sure amount of chatrooms is not at capacity
			if(isRoomSlotAvailable()){

				// send successful make room message
				send(lpParam->getSocket(), "0",  3, 0);
				cout << "Sent: " << "0" << endl;

				// receive desired username of client
				recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);
				cout << "Recv: " << recvbuf << endl;

				// set client's username id, room id, and room slot
				lpParam->setUniqueUserId(recvbuf);
				lpParam->setUniqueHostRoomId(serverData);
				lpParam->setHostRoomSlot(findEmptyRoom());

				// add new room and user to server data
				serverData[lpParam->getHostRoomSlot()].setRoomId(lpParam->getHostRoomId());
				serverData[lpParam->getHostRoomSlot()].addUser(lpParam->getUniqueUserId());
				
				// send unique room id to client for GUI display
				std::string intToString = std::to_string((long double)lpParam->getHostRoomId());
				send(lpParam->getSocket(), intToString.c_str(), strlen(intToString.c_str())+2, 0);
				cout << "Sent: " << std::string(intToString) << endl;

				// exit menu and join host room
				exit=true;

			}else{
				
				// send unsuccessful make room message (no free rooms)
				send(lpParam->getSocket(), "1",  3, 0);
				cout << "Sent: " << "1" << endl;

			}

		}else{
			
			// receive join room number
			recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);
			cout << "Recv: " << atoi(recvbuf) << endl;
			
			// save room id
			int roomId=atoi(recvbuf);

			// if room exists and room has user slot available
			if(doesRoomExist(roomId) && isRoomUserSlotAvailable(roomId)){
				
				// send successful join room message
				send(lpParam->getSocket(), "0",  3, 0);
				cout << "Sent: " << "0" << endl;

				// receive desired username of client
				recv(lpParam->getSocket(), recvbuf, recvbuflen, 0);
				cout << "Recv: " << recvbuf << endl;

				// set client's username id, room id, and room slot
				lpParam->setUniqueUserId(recvbuf);
				lpParam->setUniqueHostRoomId(serverData);
				lpParam->setHostRoomSlot(findRoomSlotFromRoomId(roomId));

				// add new user to server data
				serverData[lpParam->getHostRoomSlot()].addUser(lpParam->getUniqueUserId());

				// exit menu and join room
				exit=true;

			}else{
				
				// send unsuccessful join room message (room doesnt exist, or room is full)
				send(lpParam->getSocket(), "1", 3, 0);
				cout << "Sent: " << "1" << endl;

			}

		}
	}

	/* fill user list */
	fillUserList(lpParam->getHostRoomSlot(), lpParam->getUniqueUserId());

	/* DEBUG */
	printDebug();

}

/* Loop menu options until a room with space if found, then connect */
DWORD WINAPI processClient(LPVOID lpParam){

	/* initialize random seed */
    srand(time(NULL));

	/* Connect to room */
	connectToRoom((ClientParams*)lpParam);
	
	/* Game loop */
	roomLoop((ClientParams*)lpParam);

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

	/* Receive until the peer shuts down the connection */
	DWORD i=0;
    do{

		// accept a client socket
		ClientSocket=accept(ListenSocket, NULL, NULL);
        
		// create parameters
		ClientParams *params = new ClientParams();
		params->setSocket(ClientSocket);

		// spawn worker thread for client
		CreateThread(
			NULL,
			0,
			processClient,
			(LPVOID)params,
			0,
			&i
			);

    }while(true);

	system("pause");
    return 0;
}