/*
@author Shawn Andrews
@version 1.1.0

Terminology:
state = The current state of the game.
state::STATE_MENU = Diplay menu of Host, Join, or Exit options.
state::STATE_USERNAME = Display screen for user to enter username and press enter, or Back to previous screen.
state::STATE_KEY =  Display screen for user to enter room key and press enter, or Back to previous screen.
state::STATE_ROOM = Display chatroom to chat with other users.
DEFAULT_BUFLEN = Default buffer length for Winsock's Send() and Recv().
MAX_USERS = Maximum amount of users per chatroom.
MAX_DISPLAY_USERS = Maximum amount of users displayed at any one time in GUI user list. Use scroll bar to view more users.
MAX_MESSAGES = Maximum amount of messages in a chatroom log before the latest post is deleted to make room for new messages.
MAX_DISPLAY_MESSAGES = Maximum amount of messages displayed at any one time in GUI message list. Use scroll bar to view more messages.
RANDOM_KEY_RANGE = The number of possible keys available to allocate to various chatrooms.
FPS (Frames per Second) = The number of updates to server and screen per second.
A chatroom's key = The integer or string of numerals, if its about to be sent accross the network, in the range of 0-(RANDOM_KEY_RANGE-1) (inclusive).
An id = In terms of a room, it means the key associated to that room; in terms of a user, it means their username.

Description:
This program can create chatrooms(up to MAX_ROOMS) that holds communication between multiple users(up the MAX_USERS). 
Each chatroom is only accessible by having knowledge of the server's IP which is managing the chatroom and that chatroom's 
key. Messages in a chatroom are transmitted to other users via unencrypted and uncompressed TCP packets.

How it works:
i) Start the server, automatically assigned to port 3307. 
ii) Run the client. 
iii) Enter the desired server's WAN IP address, or LAN IP if you're in the same network as the server. 
iv) Select Host, Join, or Exit.
v.) If you selected Join, enter the chatroom's key and press enter. Enter your desired username and press enter.
vi.) You are now in a chatroom.

*/


//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

//includes
#define WIN32_LEAN_AND_MEAN
#define GLEW_STATIC
#include "glew.h"
#include <string.h>
#include <sstream>
#include <fstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <time.h>
#include <GL/glut.h>
#include <queue>

#include "GUI.h"
#include "text3d.h"
#include "imageloader.h"
#include "SOCKETclient.h"


#define DEFAULT_BUFLEN 2048
#define MAX_USERS 100
#define MAX_DISPLAY_USERS 10
#define MAX_MESSAGES 170
#define MAX_DISPLAY_MESSAGES 17
#define FPS 20

state gameState; 
GLuint aboutTexture, roomTexture, scrollTexture, backTexture;
bool hostBoxAlive, joinBoxAlive, exitBoxAlive, roomBoxAlive, backBoxAlive, userBoxAlive, textEnterLogActive, hostnameBoxAlive;
bool roomIdTextClicked, userIdTextClicked, hostnameIdTextClicked;
int room_id, user_id;
std::string room_input_id, user_input_id, hostname_input_id;
int room_id_len, user_id_len, hostname_id_len;
std::string yo;
WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo *result = NULL,
                *ptr = NULL,
                hints;
int lastSocketErrorCode;
int iResult;
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;
std::string HostOrJoin;
bool textEnterLogLineOneActive=true;
float textEnterLogOneTotalWidth, textEnterLogTwoTotalWidth;
int lastMouseX, lastMouseY;
bool userScrollActive, chatScrollActive;
int chatScrollPos = 419;
int userScrollPos = 419;
int chatScrollCounter=0;
int userScrollCounter=0;
vector<std::string> messageList;
std::string messageListShortList[MAX_DISPLAY_MESSAGES];
vector<std::string> userList;
std::string userShortList[MAX_DISPLAY_USERS];
std::string textEnterLogOne, textEnterLogTwo;
float textEnterLogTextLen;
std::string textEnterLog, lastSentTextEnterLog;
bool sendCurrentMessage;
bool serverUp;
bool firstTimeUserListSent=true;


/* Add a line to the chat box */
void addToChat(std::string line){
	cout << "Add to chat: " << line << endl;
	//if message is not a server message
	if(line.find(':')!=string::npos){

		//find if string requires split into 2 parts
		float lineFontLength=0.0f;
		for(int i=0;i<line.size();i++){
			lineFontLength+=getFontCharWidth(line[i]);
		}
		
		//more than one line
		if(lineFontLength>MAX_ONE_LINE_CHARACTER_LENGTH){

			vector<std::string> lineList;

			//find where to split the line into 2 smaller lines
			float splitLength=0.0f;
			int startCounterPos=0;
			int splitTotalCounterPos=0;
			int splitCounterPos=0;
						
			
			while( (splitTotalCounterPos!=line.size()) ){

				while( (splitLength<=MAX_ONE_LINE_CHARACTER_LENGTH) && (splitTotalCounterPos!=line.size()) ){
					splitLength+=getFontCharWidth(line[splitCounterPos]);
					splitTotalCounterPos++;
					splitCounterPos++;
				}

				//dequeue chat log if chat log is at max messages to make room for new message
				if(messageList.size()==MAX_MESSAGES){
					messageList.erase(messageList.begin());
				}
				messageList.push_back(line.substr(startCounterPos, splitCounterPos));

				splitLength=0.0f;
				startCounterPos=splitTotalCounterPos;
				splitCounterPos=0;
			}
			

		}else{

			//push line onto chat stack
			messageList.push_back(line);
		}

		float t=0.0f;
		for(int i=0;i<line.size();i++){
			t+=getFontCharWidth(line[i]);
		}

	}else{

		//push line onto chat stack
		messageList.push_back(line);

	}

	//move chat log down
	if( (messageList.size()>MAX_DISPLAY_MESSAGES) && ((messageList.size()<(MAX_MESSAGES-MAX_DISPLAY_MESSAGES))) && (chatScrollPos>113.0f) ){
		chatScrollPos-=CHAT_SCROLL_INCREMENT;
	}
	
}

/* Parse comma deliminated string and store contents */
void parseCommaDeliminatedServerData(char recvbuf[DEFAULT_BUFLEN]){
	
	//convert char[] to std::string
	std::string recvstrbuf(recvbuf);

	cout << "Parseing: '" << recvstrbuf << "'" << endl;
	
	//if data is not blank
	if(recvstrbuf!=""){

		int parseCurrent=2;
		std::size_t firstPos=0;
		std::size_t secondPos=0;
		
		//if data is an updated user list
		if( (recvstrbuf[0]=='U') & (recvstrbuf[1]==':') ){
			
			//while there is another entry
			while(recvstrbuf[parseCurrent]==','){
				
				//if enqueue or dequeue
				if(recvstrbuf[parseCurrent+1]=='+'){
					
					//enqueue user status
					firstPos = recvstrbuf.find('+', parseCurrent);
					secondPos = recvstrbuf.find(',', firstPos+1);

					//username added
					std::string addedUsername=recvstrbuf.substr(firstPos+1, (secondPos-1)-firstPos);

					//add user
					userList.push_back( addedUsername );

					//on first new user display, dont display intial list users, only yourself
					if(firstTimeUserListSent){
						if(addedUsername==user_input_id){
							addToChat("~"+addedUsername+" has entered the room!~");
							cout << "Added: '" << "~"+addedUsername+" has entered the room!~" << "'" << endl;
						}
					}else{
						addToChat("~"+addedUsername+" has entered the room!~");
						cout << "Added: '" << "~"+addedUsername+" has entered the room!~" << "'" << endl;
					}
					


				}else if(recvstrbuf[parseCurrent+1]=='-'){
					
					std::string addedUsername=recvstrbuf.substr(firstPos+1, (secondPos-1)-firstPos);

					//dequeue user status
					firstPos = recvstrbuf.find('-', parseCurrent);
					secondPos = recvstrbuf.find(',', firstPos+1);

					//username deleted
					std::string deletedUsername=recvstrbuf.substr(firstPos+1, (secondPos-1)-firstPos);

					int counter=0;
					for(int i=0;i<userList.size();i++){
						
						//if found username to be deleted
						if( (userList.at(i)==deletedUsername) ){
							
							//delete username
							userList.erase(userList.begin()+counter);

							//display that username left room
							addToChat("~"+deletedUsername+" has left the room!~");
							cout << "Added: '" << "~"+deletedUsername+" has left the room!~" << "'" << endl;
						}
						counter++;
					}

				}

				//increase position to next entry
				parseCurrent=secondPos+1;

			}

			firstTimeUserListSent=false;
		}
		
		//if data is a queue of messageList
		if( (recvstrbuf[parseCurrent]=='M') && (recvstrbuf[parseCurrent+1]==':') ){
			
			//advance in preparation for the loop
			parseCurrent=parseCurrent+2;

			//while more message to be added to message queue
			while( (parseCurrent<recvstrbuf.length()) && (recvstrbuf[parseCurrent]==',') ){
				
				firstPos = recvstrbuf.find(',', parseCurrent);
				secondPos = recvstrbuf.find(',', firstPos+1);
				addToChat(recvstrbuf.substr(firstPos+1, (secondPos-1)-firstPos));
				cout << "Added: '" << recvstrbuf.substr(firstPos+1, (secondPos-1)-firstPos) << "'" << endl;
				parseCurrent=secondPos+1;
			}

		}


	}
	
	//cout << "BYE" << endl;

}

/* Connect to server */
bool connectToServer(){

	cout << "~CLIENT~\n" << endl;

    // Initialize Winsock
    WSAStartup(MAKEWORD(2,2), &wsaData);

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(hostname_input_id.c_str(), DEFAULT_PORT, &hints, &result);

    // Attempt to connect to address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
        }

        // Connect to server
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
			lastSocketErrorCode=WSAGetLastError();
			cout << "CAN'T CONNECT!" << ", Socket Error:" << lastSocketErrorCode << endl;
            closesocket(ConnectSocket);
			return false;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

	return true;
}

/* Join room */
bool joinServerRoom(){
	

	//host or join
	if(HostOrJoin=="host"){

		//send 0
		send( ConnectSocket, "0", 3, 0 );
		cout << "Sent: " << "0" << endl;

		//recv
		recv(ConnectSocket, recvbuf, recvbuflen, 0);
		cout << "Recv: " << recvbuf << endl;

		if(recvbuf[0]=='0'){
			
			//send desired username to server
			send( ConnectSocket, user_input_id.c_str(), strlen(user_input_id.c_str())+2, 0 );
			cout << "Sent: " << user_input_id.c_str() << endl;

			//recv
			recv(ConnectSocket, recvbuf, recvbuflen, 0);
			cout << "Recv: " << recvbuf << endl;

			//set room and user id
			room_id=atoi(recvbuf);
			user_id=atoi(user_input_id.c_str());

			return true;

		}else{
			//no available rooms, repeat menu selection
			return false;
		}

	}else{

		//send 1
		send( ConnectSocket, "1", 3, 0 );
		cout << "Sent: " << "1" << endl;
		

		//send room number
		send(ConnectSocket, room_input_id.c_str(), strlen(room_input_id.c_str()), 0 );
		cout << "Sent: " << room_input_id << endl;

		//recv
		recv(ConnectSocket, recvbuf, recvbuflen, 0);
		cout << "Recv: " << recvbuf << endl;
		
		if(recvbuf[0]=='0'){

			//send desired username to server
			send( ConnectSocket, user_input_id.c_str(), strlen(user_input_id.c_str())+2, 0 );
			cout << "Sent: " << user_input_id.c_str() << endl;

			//set room and user id
			room_id=atoi(room_input_id.c_str());
			user_id=atoi(user_input_id.c_str());

			return true;

		}else{
			//room doesnt exist or is full, repeat menu selection
			return false;
		}


	}

	// set GUI room info
	yo="";
	if(HostOrJoin=="host"){
		yo="host";
	}else{
		yo+=room_id+","+user_id;
	}

}

void serverUpdate(){

	// if user pressed enter to send a message this frame, send it
	if(sendCurrentMessage){
			
		// prepare message string
		std::string sentBuffer=user_input_id+": "+lastSentTextEnterLog;

		// send message
		iResult = send( ConnectSocket, sentBuffer.c_str(), (int)(strlen(sentBuffer.c_str())+2), 0 );
		cout << "Sent: '" << sentBuffer.c_str() << "'" <<endl;

		// reset message flag
		sendCurrentMessage=false;
	}

	//receive and store user data
	iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
				
	if(iResult>0){
		cout << "Received: " << recvbuf << endl << endl;
			
		//update user list with new server data
		parseCommaDeliminatedServerData(recvbuf);
	}

	
}

void initRendering(){

	//seed
	srand(time(NULL));

	//init font
	t3dInit();

	//hostname_input_id="108.174.165.195";

	//init game state at menu
	gameState=STATE_HOSTNAME;

	//init message short list
	for(int i=0;i<MAX_DISPLAY_MESSAGES;i++){
		messageListShortList[i]="";
	}

	//init user short list
	for(int i=0;i<MAX_DISPLAY_USERS;i++){
		userShortList[i]="";
	}

	//textures
	Image* aboutImage = loadBMP("title.bmp");
	aboutTexture = loadTexture(aboutImage);
	Image* roomImage = loadBMP("border.bmp");
	roomTexture = loadTexture(roomImage);
	Image* scrollImage = loadBMP("scrollbar.bmp");
	scrollTexture = loadTexture(scrollImage);
	Image* backImage = loadBMP("backbutton.bmp");
	backTexture = loadTexture(backImage);

	//enables/disables
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void mouseMovement(int x, int y) {
	
	if(gameState==STATE_MENU){

		//check if mouse is over host box
		hostBoxAlive=checkHostBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over join box
		joinBoxAlive=checkJoinBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over exit box
		exitBoxAlive=checkExitBoxAlive(x, y, windowWidth, windowHeight);

	}else if(gameState==STATE_KEY){

		//check if mouse is over room box
		roomBoxAlive=checkRoomBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

	}else if(gameState==STATE_USERNAME){

		//check if mouse is over user box
		userBoxAlive=checkUserBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

	}else if(gameState==STATE_HOSTNAME){

		//check if mouse is over hostname box
		hostnameBoxAlive=checkHostnameBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

	}

}

void mouseClicked(int button, int state, int x, int y){

	if(gameState==STATE_MENU){

		// if host box clicked
		if( checkHostBoxClicked(x, y, windowWidth, windowHeight, button, state) ){
			HostOrJoin="host";
			gameState=STATE_USERNAME;
		}
		
		// if join box clicked
		checkJoinBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

		// if exit box clicked
		checkExitBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

	}else if(gameState==STATE_ROOM){

		if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){

			// if clicked inside user scroll, return alive flag
			userScrollActive=isUserScrollActive(x,y,userScrollPos);

			// if clicked inside chat scroll, return alive flag
			chatScrollActive=isChatScrollActive(x,y,chatScrollPos);

			// if clicked inside text enter log, return alive flag
			textEnterLogActive=isTextEnterLogActive(x,y);

			// check "leave" button
			if(checkBackButtonClicked(x,y)){
				gameState=STATE_MENU;
				serverUp=false;

				//reset to blocking socket
				u_long iMode=0;
				ioctlsocket(ConnectSocket,FIONBIO,&iMode);

				while(messageList.size()!=0){
					messageList.pop_back();
				}
				for(int i=0;i<MAX_DISPLAY_MESSAGES;i++){
					messageListShortList[i]="";
				}
				while(userList.size()!=0){
					userList.pop_back();
				}
				for(int i=0;i<MAX_DISPLAY_USERS;i++){
					userShortList[i]="";
				}

				//reset socket
				shutdown(ConnectSocket, SD_BOTH);
				WSACleanup();
				closesocket(ConnectSocket);
				connectToServer();
				firstTimeUserListSent=false;
			}

		}

	}else if(gameState==STATE_KEY){
	
		// if room box clicked
		checkRoomBoxClicked(x, y, windowWidth, windowHeight, button, state, roomIdTextClicked);
	
		// if back box clicked
		checkBackBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

	}else if(gameState==STATE_USERNAME){

		// if user box clicked
		checkUserBoxClicked(x, y, windowWidth, windowHeight, button, state, userIdTextClicked);
	
		// if back box clicked
		checkBackBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

	}else if(gameState==STATE_HOSTNAME){

		// if hostname box clicked
		checkHostnameBoxClicked(x, y, windowWidth, windowHeight, button, state, hostnameIdTextClicked);
	
		// if back box clicked
		checkBackBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

	}
	
}

void mouseClickedActive(int x, int y){

	if(gameState==STATE_ROOM){

		if(userScrollActive){

			if( ((y-lastMouseY)<=1) && ((y-lastMouseY)>=-1) ){
			
				userScrollPos-=(y-lastMouseY);

				if( (userScrollPos>419) || (userScrollPos<119) ){
					userScrollPos+=(y-lastMouseY);
				}
			}
		}else if(chatScrollActive){

			if( ((y-lastMouseY)<=1) && ((y-lastMouseY)>=-1) ){
			
				chatScrollPos-=(y-lastMouseY);

				if( (chatScrollPos>419) || (chatScrollPos<119) ){
					chatScrollPos+=(y-lastMouseY);
				}
			}
		}

	}
	

	lastMouseX=x;
	lastMouseY=y;

}

void handleKeypress(unsigned char key, int x, int y){

	//smooth camera movement
	if(gameState==STATE_ROOM){

		if(textEnterLogActive){
			
			//delete key
			if(key==8){

				//if in line one, else line two
				if( textEnterLogLineOneActive ){

					//if more characters can fit on first line, add character
					if(textEnterLogOne.length()!=NULL){

						//delete previpus character
						textEnterLogOneTotalWidth-=getFontCharWidth(textEnterLogOne[textEnterLogOne.length()-1]);
						textEnterLogOne.erase(textEnterLogOne.length()-1);
					}

				}else{

					//if more characters can fit on second line, add character
					if(textEnterLogTwo.length()!=NULL){

						//delete previpus character
						textEnterLogTwoTotalWidth-=getFontCharWidth(textEnterLogTwo[textEnterLogTwo.length()-1]);
						textEnterLogTwo.erase(textEnterLogTwo.length()-1);
						if(textEnterLogTwo.length()==NULL){
							textEnterLogLineOneActive=true;
						}
					}

				}

			//enter key
			}else if(key==13){
				
				//send message
				sendCurrentMessage=true;
				lastSentTextEnterLog=textEnterLog;

				//reset text enter box
				textEnterLogOne.clear();
				textEnterLogTwo.clear();
				textEnterLogOneTotalWidth=0.0f;
				textEnterLogTwoTotalWidth=0.0f;
				textEnterLogLineOneActive=true;

			//comma or colon key
			}else if( (key==44) || (key==58)){
				

			//other key
			}else{
				
				//if in line one, else line two
				if( textEnterLogLineOneActive ){
					
					//add key
					textEnterLogOne+=key;
					textEnterLogOneTotalWidth+=getFontCharWidth(key);
					
					if( ((textEnterLogTextLen+textEnterLogOneTotalWidth)>=TEXT_ENTER_WIDTH_MAX) ){
						textEnterLogLineOneActive=false;
					}
				}else{

					//if user-entered text for line two is within limit, add character
					if( ((textEnterLogTwoTotalWidth)<TEXT_ENTER_WIDTH_MAX) ){
						//add key
						textEnterLogTwo+=key;
						textEnterLogTwoTotalWidth+=getFontCharWidth(key);
					}

				}
			}
			
		}

		//update line one+two into full text string
		textEnterLog=textEnterLogOne+textEnterLogTwo;

	}else if(gameState==STATE_KEY){
		
		if(roomIdTextClicked){
			
			if(key==8){
				//clear string
				room_input_id.clear();
				room_id_len=0;
			}else if(key==13){
				//enter key
				if(checkRoomId(room_input_id)){
					HostOrJoin="join";
					gameState=STATE_USERNAME;
				}
			}else{
				//if room id's char limit not exceeded, add key
				if(room_id_len<MAX_ROOM_ID_CHARS){
					//add key
					room_input_id+=key;
					room_id_len++;
				}
			}

		}

	}else if(gameState==STATE_USERNAME){

		if(userIdTextClicked){

			if(key==8){
				//clear string
				user_input_id.clear();
				user_id_len=0;
			}else if(key==13){
				//enter key
				if(joinServerRoom()){
					serverUp=true;
					gameState=STATE_ROOM;

					//change to non-blocking socket
					u_long iMode=1;
					ioctlsocket(ConnectSocket,FIONBIO,&iMode);
				}
			}else{
				//if room id's char limit not exceeded, add key
				if(user_id_len<MAX_USER_ID_CHARS){
					//add key
					user_input_id+=key;
					user_id_len++;
				}
			}

		}

	}else if(gameState==STATE_HOSTNAME){

		if(key==8){
			//clear string
			hostname_input_id.clear();
			hostname_id_len=0;
		}else if(key==13){
			//enter key
			if(connectToServer()){
				gameState=STATE_MENU;
			}else{
				char buffer[7];
				itoa(lastSocketErrorCode,buffer,10);
				std::string temp(buffer);
				std::string strPlusErrorCode = "Invalid server, socket error: "+temp;
				MessageBox(NULL,strPlusErrorCode.c_str(),"Server error",MB_OK);
			}
		}else{
			//if room id's char limit not exceeded, add key
			if(hostname_id_len<MAX_HOSTNAME_CHARS){
				//add key
				hostname_input_id+=key;
				hostname_id_len++;
			}
		}

	}

	//escape key
	if (key==27){
		exit(0);
	}

}

void handleKeypressUp(unsigned char key, int x, int y){



}

void drawTexture(GLuint texture){

	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	//enables/disables
	glEnable(GL_TEXTURE_2D);

	//bind texture
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//draw about screen
	glBegin(GL_QUADS); 
		glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f, 0.0f); //top-left
		glTexCoord2f( 1.0f, 0.0f );glVertex2f( 1.0f, 0.0f); //top-right
		glTexCoord2f( 1.0f, 1.0f );glVertex2f( 1.0f,  1.0f); //bottom-right
		glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f,  1.0f); //bottom-left
	glEnd();

	//enables/disables
	glDisable(GL_TEXTURE_2D);

}

void drawTextEnterLog(){

	glViewport(20, 19, 510, 80);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	if(textEnterLogActive){
		glColor4f(0.5f,0.5f,0.5f,0.5f);
	}else{
		glColor4f(0.35f,0.35f,0.35f,0.5f);
	}

	glPushMatrix();
		glBegin(GL_QUADS);
			glVertex2f(0.0,0.0);
			glVertex2f(1.0,0.0);
			glVertex2f(1.0,1.0);
			glVertex2f(0.0,1.0);
		glEnd();
	glPopMatrix();


	glPushMatrix();
		glColor4f(0.15f,0.15f,0.15f,0.5f);
		glScalef(0.02, 0.15, 0.0f);
		glTranslatef(1.0, 5.0, 0.0);
		float ttemp=t3dDraw2D(user_input_id+":  ", -1, 1, 0.2f);
		textEnterLogTextLen=ttemp;
	glPopMatrix();
	
	glPushMatrix();
		glColor4f(0.15f,0.15f,0.15f,0.5f);
		glScalef(0.02, 0.15, 0.0f);
		glTranslatef(ttemp+0.75, 5.0, 0.0);
		float tttemp=t3dDraw2D(textEnterLogOne, -1, 1, 0.2f);
	glPopMatrix();

	glPushMatrix();
		glColor4f(0.15f,0.15f,0.15f,0.5f);
		glScalef(0.02, 0.15, 0.0f);
		glTranslatef(0.75, 3.25, 0.0);
		float ttttemp=t3dDraw2D(textEnterLogTwo, -1, 1, 0.2f);
	glPopMatrix();

}

void drawChatLog(){

	glViewport(21, 119, 449, 339);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glColor4f(0.35f,0.35f,0.35f,0.5f);

	glPushMatrix();
		glTranslatef(0,0,0.0f);
		glBegin(GL_QUADS);
			glVertex2f(0.0,0.0);
			glVertex2f(1.0,0.0);
			glVertex2f(1.0,1.0);
			glVertex2f(0.0,1.0);
		glEnd();
		//Added
	glPopMatrix();

	glColor4f(1.0f,1.0f,1.0f,1.0f);




	glViewport(470, 119, 20, 339);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glColor4f(0.2f,0.2f,0.2f,0.5f);

	glPushMatrix();
		glBegin(GL_QUADS);
			glVertex2f(0.0,0.0);
			glVertex2f(1.0,0.0);
			glVertex2f(1.0,1.0);
			glVertex2f(0.0,1.0);
		glEnd();
	glPopMatrix();


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawChatScroll(){

	glViewport(470, chatScrollPos, 20, 40);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	//enables/disables
	glEnable(GL_TEXTURE_2D);

	//bind texture
	glBindTexture(GL_TEXTURE_2D, scrollTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//draw about screen
	glBegin(GL_QUADS); 
		glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f, 0.0f); //top-left
		glTexCoord2f( 1.0f, 0.0f );glVertex2f( 1.0f, 0.0f); //top-right
		glTexCoord2f( 1.0f, 1.0f );glVertex2f( 1.0f,  1.0f); //bottom-right
		glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f,  1.0f); //bottom-left
	glEnd();

	//enables/disables
	glDisable(GL_TEXTURE_2D);

}

void drawMessageList(){

	float j=440.0f;
	for(int i=0;i<MAX_DISPLAY_MESSAGES;i++){

		glViewport(24, j, 443, 20);
		glClearColor(1,1,1,0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluOrtho2D(0, 1, 0, 1);

		glColor4f(0.9f,0.9f,0.9f,0.5f);

		//if(messageListShortList[i]!=""){
		//	glPushMatrix();
		//		glBegin(GL_QUADS);
		//			glVertex2f(0.0,0.0);
		//			glVertex2f(1.0,0.0);
		//			glVertex2f(1.0,1.0);
		//			glVertex2f(0.0,1.0);
		//		glEnd();
		//	glPopMatrix();
		//}
	
		glPushMatrix();
			glColor4f(0.15f,0.15f,0.15f,0.5f);
			glScalef(0.025, 0.75, 0.0f);
			glTranslatef(0.15, 0.0, 0);
			float temp=t3dDraw2D(messageListShortList[i], -1, 1, 0.2f);
		glPopMatrix();

		//space between messages in message list
		j-=20.0f;
	}


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawUserLog(){

	glColor4f(1.0f,1.0f,1.0f,1.0f);

	glViewport(510, 119, 89, 339);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glColor4f(0.35f,0.35f,0.35f,0.5f);

	glPushMatrix();
		glTranslatef(0,0,0.0f);
		//Your code for drawing.
		glBegin(GL_QUADS);
			glVertex2f(0.0,0.0);
			glVertex2f(1.0,0.0);
			glVertex2f(1.0,1.0);
			glVertex2f(0.0,1.0);
		glEnd();
		//Added
	glPopMatrix();

	glColor4f(1.0f,1.0f,1.0f,1.0f);



	glViewport(599, 119, 21, 339);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glColor4f(0.2f,0.2f,0.2f,0.5f);

	glPushMatrix();
		//Your code for drawing.
		glBegin(GL_QUADS);
			glVertex2f(0.0,0.0);
			glVertex2f(1.0,0.0);
			glVertex2f(1.0,1.0);
			glVertex2f(0.0,1.0);
		glEnd();
		//Added
	glPopMatrix();


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawUserScroll(){

	glViewport(599, userScrollPos, 20, 40);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	//enables/disables
	glEnable(GL_TEXTURE_2D);
	
	//bind texture
	glBindTexture(GL_TEXTURE_2D, scrollTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//draw about screen
	glBegin(GL_QUADS); 
		glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f, 0.0f); //top-left
		glTexCoord2f( 1.0f, 0.0f );glVertex2f( 1.0f, 0.0f); //top-right
		glTexCoord2f( 1.0f, 1.0f );glVertex2f( 1.0f,  1.0f); //bottom-right
		glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f,  1.0f); //bottom-left
	glEnd();

	//enables/disables
	glDisable(GL_TEXTURE_2D);

}

void drawBackBox(){

	glViewport(556, 19, 64, 80);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	//enables/disables
	glEnable(GL_TEXTURE_2D);

	//bind texture
	glBindTexture(GL_TEXTURE_2D, backTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//draw about screen
	glBegin(GL_QUADS); 
		glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f, 0.0f); //top-left
		glTexCoord2f( 1.0f, 0.0f );glVertex2f( 1.0f, 0.0f); //top-right
		glTexCoord2f( 1.0f, 1.0f );glVertex2f( 1.0f,  1.0f); //bottom-right
		glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f,  1.0f); //bottom-left
	glEnd();

	//enables/disables
	glDisable(GL_TEXTURE_2D);

	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawMenu(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	
	// title image
	drawTexture(aboutTexture);

	// host box
	drawHostBox(hostBoxAlive);

	// join box
	drawJoinBox(joinBoxAlive);

	// exit box
	drawExitBox(exitBoxAlive);

	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

void drawUserHeader(){
	
	glViewport(514, 430, 80, 23);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glColor4f(0.9f,0.9f,0.9f,0.5f);

	glPushMatrix();
		glBegin(GL_QUADS);
			glVertex2f(0.0,0.0);
			glVertex2f(1.0,0.0);
			glVertex2f(1.0,1.0);
			glVertex2f(0.0,1.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glColor4f(0.3f, 0.3f, 0.3f, 0.25f);
		glScalef(0.2, 1.0, 0.0f);
		glTranslatef(0.85, 0.0, 0);
		float temp=t3dDraw2D("Users", -1, 1, 0.2f);
	glPopMatrix();


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawUserList(){

	float j=400.0f;
	for(int i=0;i<MAX_DISPLAY_USERS;i++){

		glViewport(514, j, 80, 20);
		glClearColor(1,1,1,0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluOrtho2D(0, 1, 0, 1);

		glColor4f(0.9f,0.9f,0.9f,0.5f);

		if(userShortList[i]!=""){
			glPushMatrix();
				glBegin(GL_QUADS);
					glVertex2f(0.0,0.0);
					glVertex2f(1.0,0.0);
					glVertex2f(1.0,1.0);
					glVertex2f(0.0,1.0);
				glEnd();
			glPopMatrix();
		}

		glPushMatrix();
			glColor4f(0.3f, 0.3f, 0.3f, 0.25f);
			glScalef(0.125, 0.85, 0.0f);
			glTranslatef(0.15, 0.0, 0);
			float temp=t3dDraw2D(userShortList[i], -1, 1, 0.2f);
		glPopMatrix();

		j-=30.0f;
	}


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawRoomAndUserText(){

	glViewport(20, 460, 320, 20);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glPushMatrix();
		glColor4f(0.75f,0.75f,0.75f,0.5f);
		glScalef(0.04, 0.65, 0.0f);
		glTranslatef(0.0, 0.4, 0.0);
		float ttemp=t3dDraw2D("User: "+user_input_id+" | ", -1, 1, 0.2f);
	glPopMatrix();

	glPushMatrix();
		glColor4f(0.75f,0.75f,0.75f,0.5f);
		glScalef(0.04, 0.65, 0.0f);
		glTranslatef(ttemp, 0.4, 0);
		float tttemp=t3dDraw2D("Room #: "+to_string((long double)room_id), -1, 1, 0.2f);
	glPopMatrix();
}

void drawRoom(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawTexture(roomTexture);

	drawRoomAndUserText();

	drawChatLog();

	drawChatScroll();

	drawUserLog();

	drawUserScroll();

	drawUserHeader();

	drawUserList();

	drawTextEnterLog();

	drawMessageList();

	drawBackBox();

	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

void drawRoomKey(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	// title image
	drawTexture(aboutTexture);

	// room box
	drawRoomBox(roomBoxAlive, room_input_id);

	// back box
	drawBackBox(backBoxAlive);

	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

void drawUsername(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	// title image
	drawTexture(aboutTexture);

	// user box
	drawUserBox(userBoxAlive, user_input_id);

	// back box
	drawBackBox(backBoxAlive);

	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawHostname(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	// title image
	drawTexture(aboutTexture);

	// user box
	drawHostnameBox(hostnameBoxAlive, hostname_input_id);

	// back box
	drawBackBox(backBoxAlive);

	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawScene() {

	if(gameState==STATE_MENU){
		drawMenu();
	}else if(gameState==STATE_ROOM){
		drawRoom();
	}else if(gameState==STATE_KEY){
		drawRoomKey();
	}else if(gameState==STATE_USERNAME){
		drawUsername();
	}else if(STATE_HOSTNAME){
		drawHostname();
	}

	//swap
	glutSwapBuffers();
}

void update(int value) {

	/* Update server if in room */
	if(serverUp){
		serverUpdate();
	}

	/* Update user list */
	for(int i=0;i<MAX_DISPLAY_USERS;i++){
		if( ((-(((userScrollPos-119)/3)-100)+i)<userList.size()) ){
			userShortList[i]=userList.at( -(((userScrollPos-119)/3)-100)+i );
		}else{
			userShortList[i]="";
		}
		
	}

	/* Update message list */
	for(int i=0;i<MAX_DISPLAY_MESSAGES;i++){
		if( ((-(((chatScrollPos-119)/3)-100)+i)<messageList.size()) ){
			messageListShortList[i]=messageList.at( -(((chatScrollPos-119)/3)-100)+i );
		}else{
			messageListShortList[i]="";
		}
		
	}

	glutPostRedisplay();
	glutTimerFunc(1000/FPS, update, 0);
}

void handleResize(int w, int h) {
	//for resizeable
	//glViewport(0, 0, w/1.3, h);
	//windowWidth=w;
	//windowHeight=h;

	glutReshapeWindow(windowWidth,windowHeight);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Polished IRC");

	//glutGameModeString( "640x480:32@60" );
	//glutEnterGameMode();
	
	glewInit();
	GLenum err = glewInit();
    if(GLEW_OK != err) {
		cout << "glewInit failed." << endl;
    }

	//glutWarpPointer(windowWidth/2, windowHeight/2);
	initRendering();
	glutDisplayFunc(drawScene);
	glutMouseFunc(mouseClicked);
	glutPassiveMotionFunc(mouseMovement);
	glutMotionFunc(mouseClickedActive);
	glutKeyboardFunc(handleKeypress);
	glutKeyboardUpFunc(handleKeypressUp);
	glutReshapeFunc(handleResize);
	glutTimerFunc(FPS, update, 0);

	glutMainLoop();

	return 0;
}