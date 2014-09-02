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

	for(int i=0;i<serverData[foundRoom].getRoomSize();i++){
		
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
	if(serverData[roomSlot].getRoomSize() < MAX_USERS ){
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
	for(int i=0;i<serverData[foundRoomSlot].getRoomSize();i++){
		serverData[foundRoomSlot].enqueueUserStatus(serverData[foundRoomSlot].getUserIndexFromUserList(username), "+"+serverData[foundRoomSlot].getUsername(i));
	}

}

/* Display room debug information */
void printDebug(){

	cout << "~~~~~~~~~~~~ROOMS~~~~~~~~~~~~" << endl;
	for(int i=0; i<MAX_ROOMS; i++){
		cout << "Room #" << i << " : " << serverData[i].getRoomId() << " || " << "Num of users: " << serverData[i].getRoomSize() << "/" << MAX_USERS << " || ";
		for(int j=0; j<MAX_USERS; j++){
			if(serverData[i].getUsername(j)!=""){
				cout << serverData[i].getUsername(j) << " (" << j << ")";
				cout << ", ";
			}
		}
		cout << endl;
	}
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
	cout << endl;

}

/* Attempt to connect to room */
bool attemptConnectToRoom(char hostOrJoin, string username, string roomId, ClientParams& client){

	//variables
	string result="";

	//if 0=host room attempt, or 1=join room attempt
	if(hostOrJoin=='0'){
			
		//check to make sure amount of chatrooms is not at capacity
		if(isRoomSlotAvailable()){

			// send successful make room message
			result+=",0,";

			// set client's username id, room id, and room slot
			client.setUniqueUserId(username);
			client.setUniqueHostRoomId(serverData);
			client.setHostRoomSlot(findEmptyRoom());
			cout << "Host room slot: '" << client.getHostRoomSlot() << "'" << endl;
			cout << "USER = " << client.getUniqueUserId() << endl;

			// add new room and user to server data
			serverData[client.getHostRoomSlot()].setRoomId(client.getHostRoomId());
			serverData[client.getHostRoomSlot()].addUser(client.getUniqueUserId());
				
			// send unique room id to client for GUI display
			std::string intToString = std::to_string(client.getHostRoomId());
			result+=intToString.append(",");

		}else{
				
			// send unsuccessful make room message (no free rooms)
			result+=",1,";

		}

	}else{

		// if room exists, username is unique to other users in room, and room has user slot available
		if(doesRoomExist(atoi(roomId.c_str())) && isRoomUserSlotAvailable(atoi(roomId.c_str()))){
				
			// send successful join room message
			result+=",0,";

			// set client's username id, room id, and room slot
			client.setUniqueUserId(username);
			client.setHostRoomId(atoi(roomId.c_str()));
			client.setHostRoomSlot(findRoomSlotFromRoomId(atoi(roomId.c_str())));

			// add new user to server data
			serverData[client.getHostRoomSlot()].addUser(client.getUniqueUserId());

			// send unique room id to client for GUI display
			std::string intToString = std::to_string(client.getHostRoomId());
			result+=intToString.append(",");

		}else{
				
			// send unsuccessful join room message (room doesnt exist, or room is full)
			result+=",1,";

		}

	}

	/* DEBUG */
	printDebug();

	// send result of attempt to join room
	send(client.getSocket(), result.c_str(), (int)(strlen(result.c_str())+2), 0);
	cout << "Sent2: '" << result << "'" << endl;

	// if successful, fill user list
	if(result[1]=='0'){

		/* fill user list */
		fillUserList(client.getHostRoomSlot(), client.getUniqueUserId());

		return true;

	}else{
		return false;
	}

}

/* Parse client menu packet for HostOrJoin flag, username, and room id */
void parseClientMenuPacket(string& recvbufString, char& tempHostOrJoin, string& tempUsername, string& tempRoomId, int& commaPos){

	tempHostOrJoin=recvbufString[1]; // 0=host, 1=join
	recvbufString.erase(0,3);
	if(tempHostOrJoin=='0'){
		commaPos=recvbufString.find(',', 0);
		tempUsername=recvbufString.substr(0,commaPos);
		recvbufString.erase(0, tempUsername.length()+1);
	}else if(tempHostOrJoin=='1'){
		commaPos=recvbufString.find(',', 0);
		tempRoomId=recvbufString.substr(0,commaPos);
		recvbufString.erase(0, tempRoomId.length()+1);
		commaPos=recvbufString.find(',', 0);
		tempUsername=recvbufString.substr(0,commaPos);
		recvbufString.erase(0, tempUsername.length()+1);
	}

}

int main(void){
	
	/* Server variables */
    WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	vector<ClientParams> ClientConnections;
    struct addrinfo *result = NULL;
    struct addrinfo hints;
	FD_SET WriteSet;
	FD_SET ReadSet;
	char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen=DEFAULT_BUFLEN;
	int iResult;

	printf("~SERVER~\n");

	/* initialize random seed */
    srand(time(NULL));

	/* initialize rooms */
	initialize_rooms(serverData);
    
    /* Initialize Winsock*/
	initialize_winsock(wsaData, hints, &result);

	/* Set socket and bind to TCP listening socket */
	set_socketandbind(ListenSocket, result);

	/* Server loop */
	while(TRUE){

		// clear read and write sets and add listen socket to read set
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);
		FD_SET(ListenSocket, &ReadSet);

		// add read and write sets of client connections
		for(std::vector<ClientParams>::iterator currentClientSocket = ClientConnections.begin(); currentClientSocket != ClientConnections.end(); ++currentClientSocket) {
			FD_SET(currentClientSocket->getSocket(), &WriteSet);
			FD_SET(currentClientSocket->getSocket(), &ReadSet);
		}

		// select
		if (select(0, &ReadSet, &WriteSet, NULL, NULL) == SOCKET_ERROR){
			printf("select() returned with error %d\n", WSAGetLastError());
			return 1;
		}

		// acccept new connections
		if (FD_ISSET(ListenSocket, &ReadSet)){
			ClientParams tempClientConnection;
			tempClientConnection.setSocket(accept(ListenSocket, NULL, NULL));
			ClientConnections.push_back(tempClientConnection);
			cout << "Added client!" << endl;
		}


		// for all clients
		for(std::vector<ClientParams>::iterator currentClientSocket = ClientConnections.begin(); currentClientSocket != ClientConnections.end();) {

			/* Variables */
			bool toBeDeleted=false;

			// if read available
			if(FD_ISSET(currentClientSocket->getSocket(), &ReadSet)){

				// handle data for each state
				if(currentClientSocket->getState()==STATE_MENU){

					// receive data
					recv(currentClientSocket->getSocket(), recvbuf, recvbuflen, 0);

					// variables
					string recvbufString(recvbuf);
					string tempUsername="";
					string tempRoomId="";
					char tempHostOrJoin=NULL;
					int commaPos=NULL;

					// parse data
					parseClientMenuPacket(recvbufString, tempHostOrJoin, tempUsername, tempRoomId, commaPos);
					cout << "Received: '" << recvbuf << "'" << endl;

					// attempt room connection
					if(attemptConnectToRoom(tempHostOrJoin, tempUsername, tempRoomId, *currentClientSocket)){
						/* Non-blocking socket */
						u_long iMode=1;
						ioctlsocket(currentClientSocket->getSocket(),FIONBIO,&iMode);

						// set state to room
						currentClientSocket->setState(STATE_ROOM);
					}
					

				}else if(currentClientSocket->getState()==STATE_ROOM){

					/* variables */
					char recvbuf[DEFAULT_BUFLEN];
					int recvbuflen=DEFAULT_BUFLEN;
					int iResult=NULL;

					/* Receive user's new message to be added to room */
					iResult = recv(currentClientSocket->getSocket(), recvbuf, recvbuflen, 0);

					// if packet is free of error, else if closed connection by client
					if (iResult > 0) {

						// if packet contains data
						if(recvbuf!=""){
							// add message to all users' message list
							for(int i=0;i<serverData[currentClientSocket->getHostRoomSlot()].getRoomSize();i++){
								serverData[currentClientSocket->getHostRoomSlot()].enqueueMessage(i, recvbuf);
							}
						}
					
					}else if ((iResult == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) || WSAGetLastError()==NULL){
						printf("Connection closing...\n");
						
						//delete connection from vector list
						toBeDeleted=true;

					}

				}
				

			}

			// if write available
			if(FD_ISSET(currentClientSocket->getSocket(), &WriteSet)){

				// handle data for each state
				if(currentClientSocket->getState()==STATE_MENU){

					

				}else if(currentClientSocket->getState()==STATE_ROOM){

					/* variables */
					char recvbuf[DEFAULT_BUFLEN];
					int recvbuflen=DEFAULT_BUFLEN;
					int iResult=NULL;
					std::string sendBuffer="";

					/* If last frame's packet wasn't blocked and needs to be resent, send current list of room's users and messages to user if list has updated */
					sendBuffer=prepareUserAndMessageList(currentClientSocket->getHostRoomSlot(), currentClientSocket->getUniqueUserId() );
					
					/* If there is an update to chatroom, attempt to send it */
					if(sendBuffer != DEFAULT_PACKET_SCHEME){
						cout << "Sent: " << sendBuffer << " || To: " << currentClientSocket->getUniqueUserId() << endl;
						iResult = send( currentClientSocket->getSocket(), sendBuffer.c_str(), (int)(strlen(sendBuffer.c_str())+1), 0 );

						/* If there was an error sending packet, attempt to resend it next frame until it sends successfully */
						if (iResult == SOCKET_ERROR) {
							cout << "Send() failed with error: " << WSAGetLastError() << endl;
						}
					}


				}

			}//end writes

			//if client disconnected, delete connection
			if(toBeDeleted){

				//delete user from list in room
				serverData[currentClientSocket->getHostRoomSlot()].deleteUser((currentClientSocket->getUniqueUserId()));

				//delete room if user was last to leave room
				if(serverData[currentClientSocket->getHostRoomSlot()].getRoomSize()==0){
					serverData[currentClientSocket->getHostRoomSlot()].deleteRoom();
					cout << "Room index " << currentClientSocket->getHostRoomSlot() << " has been deleted." << endl;
				}

				//erase from vector list
				currentClientSocket=ClientConnections.erase(currentClientSocket);

				//print debug
				printDebug();

			}else{
				++currentClientSocket;
			}

		}//end for all clients

		/* Sleep */
		Sleep(50);

	}//end while

}//end main