//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

#define WIN32_LEAN_AND_MEAN

//includes
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

GLuint aboutTexture, roomTexture, scrollTexture, backTexture;
const int FPS=20;
int gameState=0;
bool hostBoxAlive, joinBoxAlive, exitBoxAlive, roomBoxAlive, backBoxAlive, userBoxAlive, textEnterLogActive;
bool roomIdTextClicked, userIdTextClicked;
int room_id;
std::string room_input_id, user_input_id;
int room_id_len, user_id_len;
std::string yo;
WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo *result = NULL,
                *ptr = NULL,
                hints;
int iResult;
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;
fd_set readfds, writefds;struct timeval tv;
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

int tem=0;


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
			//cout << endl;

		}else{

			//push line onto chat stack
			messageList.push_back(line);
		}

		float t=0.0f;
		for(int i=0;i<line.size();i++){
			t+=getFontCharWidth(line[i]);
			//cout << "Char: '" << line[i] << "'" << endl;
		}

		//cout << "Length: " << t << endl;

	}else{

		//push line onto chat stack
		messageList.push_back(line);

	}

	//move chat log down
	if( (messageList.size()>MAX_DISPLAY_MESSAGES) && ((messageList.size()<(MAX_MESSAGES-MAX_DISPLAY_MESSAGES))) ){
		chatScrollPos-=CHAT_SCROLL_INCREMENT;
	}
	//cout << "Chat: " << chatScrollPos << endl;
	

}

/* Parse comma deliminated string and store contents */
void parseCommaDeliminatedServerData(char recvbuf[DEFAULT_BUFLEN]){
	//cout << "HI" << endl;
	
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
			//cout << "1" << endl;
			//while there is another entry
			while(recvstrbuf[parseCurrent]==','){
				//cout << "11" << endl;
				//if enqueue or dequeue
				if(recvstrbuf[parseCurrent+1]=='+'){
					//cout << "111" << endl;
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
					//cout << "1111" << endl;
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
		//cout << "2" << endl;
		//if data is a queue of messageList
		if( (recvstrbuf[parseCurrent]=='M') && (recvstrbuf[parseCurrent+1]==':') ){
			
			//advance in preparation for the loop
			parseCurrent=parseCurrent+2;

			//while more message to be added to message queue
			while( (parseCurrent<recvstrbuf.length()) && (recvstrbuf[parseCurrent]==',') ){
				//cout << "222" << endl;
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

void connectToServer(){

	cout << "~CLIENT~\n" << endl;

    // Initialize Winsock
    WSAStartup(MAKEWORD(2,2), &wsaData);

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(inet_ntoa(*(struct in_addr *)*(gethostbyname("irc.polishedgames.com"))->h_addr_list), DEFAULT_PORT, &hints, &result);

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            //ConnectSocket = INVALID_SOCKET;
			cout << "CAN'T CONNECT!" << endl;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

}

bool joinServerRoom(){
	
	//do {
		yo="";
		//cout << "Host-To host a game" << endl << "Join-To join a game" << endl;
		if(HostOrJoin=="host"){
			yo="host";
		}else{
			yo+=room_input_id+","+user_input_id;
		}
		const char *sendbuf=yo.c_str();
		cout << "Room id: '" << room_input_id << "'" << " || yo: '" << yo << "'" << endl;


		//u_long iMode=1;
		//ioctlsocket(ConnectSocket,FIONBIO,&iMode);

		// Send an initial buffer
		send( ConnectSocket, sendbuf, (int)strlen(sendbuf)+2, 0 );
		cout << "Sent: " << sendbuf << endl;
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		room_id=atoi(recvbuf);
		cout << "Recv: " << recvbuf << endl;
		send( ConnectSocket, user_input_id.c_str(), strlen(user_input_id.c_str())+2, 0 );
		cout << "Send: " << user_input_id.c_str() << endl;
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		cout << "recv: " << recvbuf << " | Atoi: " << atoi(recvbuf) << endl << endl;

		if( (atoi(recvbuf)==1) ){
			if(HostOrJoin=="host"){
				//room_id=atoi(recvbuf);
			}else{
				room_id=atoi(room_input_id.c_str());
			}
			memset(recvbuf, 0, sizeof(recvbuf));
	
			cout << "room_id: " << room_id << endl;
	
			serverUp=true;

			u_long iMode=1;
			ioctlsocket(ConnectSocket,FIONBIO,&iMode);
			cout << "START" << endl;

			return true;
		}

	return false;

	//}while(atoi(recvbuf)!=1);

	//cout << "end loop" << endl;
	//iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	//cout << "recv" << recvbuf << endl;

}

void initRendering(){

	//seed
	srand(time(NULL));

	//init font
	t3dInit();

	//init message list
	//for(int i=0;i<MAX_MESSAGES;i++){
	//	messageList[i]="i: "+std::to_string((long double)i);
	//}

	//init message short list
	for(int i=0;i<MAX_DISPLAY_MESSAGES;i++){
		messageListShortList[i]="";
	}

	//init user list
	//for(int i=0;i<MAX_USERS;i++){
	//	userList[i]="";
	//}

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

	//connect to server
	connectToServer();

}

void serverUpdate(){
	/*SERVER*/
	if(serverUp){
		cout << "Counter: " << tem << endl;
		
		//std::ostringstream sstream;
		//sstream << x11 << "," << x22 << "," << y11 << "," << y22;
		//std::string str = sstream.str();
		//const char* sendbuf=str.c_str();

		//printf("Bytes Sent: %ld\n", iResult);

		// clear the set ahead of time
		FD_ZERO(&readfds);		FD_ZERO(&writefds);

		// add our descriptors to the set
		FD_SET(ConnectSocket, &readfds);
		FD_SET(ConnectSocket, &writefds);
		
		// wait until either socket has data ready to be recv()d (timeout 10 secs)
		tv.tv_sec = 10;

		//select
		int rv = select(ConnectSocket+1, &readfds, &writefds, NULL, &tv);
		
		if (rv == -1) {
			perror("select"); // error occurred in select()
		}else if (rv == 0){
			printf("Timeout occurred! No data after 10 seconds.\n");
		}else{
			//perror("success");
			// one or both of the descriptors have data
			

			if (FD_ISSET(ConnectSocket, &writefds)) {
				//receive and store user data
				
				if(sendCurrentMessage){
					std::string sentBuffer=user_input_id+": "+lastSentTextEnterLog;
					iResult = send( ConnectSocket, sentBuffer.c_str(), (int)(strlen(sentBuffer.c_str())+2), 0 );
					cout<<"Sent: '"<<sentBuffer.c_str()<<"'"<<endl;
					sendCurrentMessage=false;
				}
				
			}
			
			if (FD_ISSET(ConnectSocket, &readfds)) {
				
				//receive and store user data
				iResult = recv(ConnectSocket, recvbuf, 2048, 0);
				
				cout << "Received: " << recvbuf << endl << endl;

				//update user list with new server data
				parseCommaDeliminatedServerData(recvbuf);

			}

		}

        if ( iResult > 0 ){
            //printf("Bytes received: %d\n", iResult);
		}else if ( iResult == 0 ){
            printf("Connection closed\n");
		}else{
            printf("recv failed with error: %d\n", WSAGetLastError());
		}
		tem++;
	}
	
}

void mouseMovement(int x, int y) {
	
	if(gameState==0){

		//check if mouse is over host box
		hostBoxAlive=checkHostBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over join box
		joinBoxAlive=checkJoinBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over exit box
		exitBoxAlive=checkExitBoxAlive(x, y, windowWidth, windowHeight);

	}else if(gameState==2){

		//check if mouse is over room box
		roomBoxAlive=checkRoomBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

	}else if(gameState==3){

		//check if mouse is over user box
		userBoxAlive=checkUserBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

	}

}

void mouseClicked(int button, int state, int x, int y){

	if(gameState==0){

		// if host box clicked
		if( checkHostBoxClicked(x, y, windowWidth, windowHeight, button, state) ){
			HostOrJoin="host";
			gameState=3;
		}
		
		// if join box clicked
		checkJoinBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

		// if exit box clicked
		checkExitBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

	}else if(gameState==1){

		if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){

			// if clicked inside user scroll, return alive flag
			userScrollActive=isUserScrollActive(x,y,userScrollPos);

			// if clicked inside chat scroll, return alive flag
			chatScrollActive=isChatScrollActive(x,y,chatScrollPos);

			// if clicked inside text enter log, return alive flag
			textEnterLogActive=isTextEnterLogActive(x,y);

			// check button button
			if(checkBackButtonClicked(x,y)){
				gameState=0;
				serverUp=false;
				//for(std::vector<std::string>::iterator currentUser = messageList.begin(); currentUser != messageList.end(); ++currentUser){
				//	//messageList.at(messageList.begin()+currentUser)="";
				//	*currentUser.p;
				//}
				while(messageList.size()!=0){
					messageList.pop_back();
				}
				for(int i=0;i<MAX_DISPLAY_MESSAGES;i++){
					messageListShortList[i]="";
				}
				//for(std::vector<std::string>::iterator currentUser = userList.begin(); currentUser != userList.end(); ++currentUser){
				//	//userList.at(userList.begin()+currentUser)="";
				//	*currentUser="";
				//}
				while(userList.size()!=0){
					userList.pop_back();
				}
				for(int i=0;i<MAX_DISPLAY_USERS;i++){
					userShortList[i]="";
				}
				closesocket(ConnectSocket);
				connectToServer();
				firstTimeUserListSent=false;
			}

		}

	}else if(gameState==2){
	
		// if room box clicked
		checkRoomBoxClicked(x, y, windowWidth, windowHeight, button, state, roomIdTextClicked);
	
		// if back box clicked
		checkBackBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

	}else if(gameState==3){

		// if user box clicked
		checkUserBoxClicked(x, y, windowWidth, windowHeight, button, state, userIdTextClicked);
	
		// if back box clicked
		checkBackBoxClicked(x, y, windowWidth, windowHeight, button, state, gameState);

	}
	
}

void mouseClickedActive(int x, int y){

	if(gameState==1){

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
	if(gameState==1){

		if(textEnterLogActive){
			
			//delete key
			if(key==8){

				//if in line one, else line two
				if( textEnterLogLineOneActive ){

					//if more characters can fit on first line, add character
					if(textEnterLogOne.length()!=NULL){

						//delete previpus character
						textEnterLogOneTotalWidth-=getFontCharWidth(textEnterLogOne[textEnterLogOne.length()-1]);
						//cout << "-" << getFontCharWidth(textEnterLogOne[textEnterLogOne.length()-1]) << " | " << textEnterLogOne[textEnterLogOne.length()-1] << endl; 
						textEnterLogOne.erase(textEnterLogOne.length()-1);
						//cout << "-2" << getFontCharWidth(textEnterLogOne[textEnterLogOne.length()-1]) << " | " << textEnterLogOne[textEnterLogOne.length()-1] << endl; 
						//cout << "Deleted" << endl;
					}

				}else{

					//if more characters can fit on second line, add character
					if(textEnterLogTwo.length()!=NULL){
						//delete previpus character
						textEnterLogTwoTotalWidth-=getFontCharWidth(textEnterLogTwo[textEnterLogTwo.length()-1]);
						//cout << "-" << getFontCharWidth(textEnterLogTwo[textEnterLogTwo.length()-1]) << " | " << textEnterLogTwo[textEnterLogTwo.length()-1] << endl; 
						textEnterLogTwo.erase(textEnterLogTwo.length()-1);
						//cout << "-2" << getFontCharWidth(textEnterLogTwo[textEnterLogTwo.length()-1]) << " | " << textEnterLogTwo[textEnterLogTwo.length()-1] << endl; 
						//cout << "Deleted" << endl;
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
					//cout << "+" << getFontCharWidth(key) << " | " << key << endl; 
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

					//cout << "Width: " << textEnterLogTwoTotalWidth << endl;
				}
			}
			//cout << "Total: " << textEnterLogTextLen+textEnterLogOneTotalWidth << endl;
		}

		//update line one+two into full text string
		textEnterLog=textEnterLogOne+textEnterLogTwo;

	}else if(gameState==2){
		
		if(roomIdTextClicked){
			
			if(key==8){
				//clear string
				room_input_id.clear();
				room_id_len=0;
			}else if(key==13){
				//enter key
				if(checkRoomId(room_input_id)){
					HostOrJoin="join";
					gameState=3;
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

	}else if(gameState==3){

		if(userIdTextClicked){

			if(key==8){
				//clear string
				user_input_id.clear();
				user_id_len=0;
			}else if(key==13){
				//enter key
				if(joinServerRoom()){
					gameState=1;
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

void drawMenu1(){

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

void drawMenu2(){

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

void drawMenu3(){

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

void gameMenu4(){

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
	drawUserBox(userBoxAlive, user_input_id);

	// back box
	drawBackBox(backBoxAlive);

	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

void drawScene() {

	if(gameState==0){
		drawMenu1();
	}else if(gameState==1){
		drawMenu2();
	}else if(gameState==2){
		drawMenu3();
	}else if(gameState==3){
		gameMenu4();
	}

	//swap
	glutSwapBuffers();
}

void update(int value) {

	//update server data
	serverUpdate();

	//update short list of display users
	for(int i=0;i<MAX_DISPLAY_USERS;i++){
		if( ((-(((userScrollPos-119)/3)-100)+i)<userList.size()) ){
			userShortList[i]=userList.at( -(((userScrollPos-119)/3)-100)+i );
		}else{
			userShortList[i]="";
		}
		
	}

	//update short list of messageList
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