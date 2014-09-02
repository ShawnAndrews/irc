/*
@author Shawn Andrews
@version 1.2.0

Terminology:
state = The current state of the game.
state::STATE_MENU = Diplay menu of Host, Join, or Exit options.
state::STATE_USERNAME = Display screen for user to enter username and press enter, or Back to previous screen.
state::STATE_KEY =  Display screen for user to enter room key and press enter, or Back to previous screen.
state::STATE_ROOM = Display chatroom to chat with other users.
state::STATE_EXIT = Exit client program.
DEFAULT_BUFLEN = Default buffer length for Winsock's Send() and Recv().
MAX_USERS = Maximum amount of users per chatroom.
MAX_DISPLAY_USERS = Maximum amount of users displayed at any one time in GUI user list. Use scroll bar to view more users.
MAX_MESSAGES = Maximum amount of messages in a chatroom log before the latest post is deleted to make room for new messages.
MAX_DISPLAY_MESSAGES = Maximum amount of messages displayed at any one time in GUI message list. Use scroll bar to view more messages.
RANDOM_KEY_RANGE = The number of possible keys available to allocate to various chatrooms.
MAX_ONE_LINE_CHARACTER_LENGTH = The maximum length of the characters in a string which can be fitted into one line of the message list in a chatroom.
FPS (Frames per Second) = The number of updates to server and screen per second.
sendCurrentMessage = If the client requested to send a message to chatroom in prior frame, send it the following frame.
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
v.) If you selected Join, enter the chatroom's key and press enter. Then enter your desired username and press enter.
vi.) You are now in a chatroom.

*/

//show console window
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
#include <thread>
#include <chrono>

#include "GUI.h"
#include "text3d.h"
#include "SOCKETclient.h"
#include <SOIL/SOIL.h>

#define DEFAULT_BUFLEN 2048
#define MAX_USERS 100
#define MAX_DISPLAY_USERS 10
#define MAX_MESSAGES 170
#define MAX_DISPLAY_MESSAGES 17
#define MAX_LOADING_FRAMES 28
#define MAX_MENU_ANIMATION_FRAMES 50
#define MAX_BORDER_ANIMATION_FRAMES 50
#define FPS 20


//variables
state currentState;
GLuint aboutTexture, roomTexture, scrollTexture, loadingTexture[MAX_LOADING_FRAMES], currentLoadingTexture, menuAnimationTexture[MAX_MENU_ANIMATION_FRAMES], currentMenuAnimationTexture, borderAnimationTexture[MAX_MENU_ANIMATION_FRAMES], currentBorderAnimationTexture;
bool hostBoxAlive, joinBoxAlive, exitBoxAlive, roomBoxAlive, backBoxAlive, userBoxAlive, textEnterLogActive, hostnameBoxAlive;
bool roomIdTextClicked, userIdTextClicked, hostnameIdTextClicked;
int room_id, user_id;
std::string room_input_id, user_input_id, hostname_input_id;
int room_id_len, user_id_len, hostname_id_len;
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
bool firstTimeUserListSent=true;
bool loadingState=false;
int loadingIconCounter, subLoadingIconCounter, menuAnimationCounter, subMenuAnimationCounter, borderAnimationCounter, subBorderAnimationCounter;
string loadingStateText;


/*
	Draw text, background color, or texture functions
*/

/* Draw full-window texture background */
void drawBackground(GLuint texture){

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//draw about screen
	glBegin(GL_QUADS); 
		glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f, 1.0f); //top-left
		glTexCoord2f( 1.0f, 0.0f );glVertex2f( 1.0f, 1.0f); //top-right
		glTexCoord2f( 1.0f, 1.0f );glVertex2f( 1.0f,  0.0f); //bottom-right
		glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f,  0.0f); //bottom-left
	glEnd();

	//enables/disables
	glDisable(GL_TEXTURE_2D);

}

/* Draw loading screen */
void drawLoadingScreen(){
	
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	/* Update counter */
	subLoadingIconCounter++;
	
	if(subLoadingIconCounter==2){
		loadingIconCounter++;
		
		if(loadingIconCounter>(MAX_LOADING_FRAMES-1)){
			loadingIconCounter=0;
		}else{
			currentLoadingTexture=loadingTexture[loadingIconCounter];
		}

		subLoadingIconCounter=0;
	}

	/* Draw loading icon animation */
	glPushMatrix();

		glEnable(GL_TEXTURE_2D);

		//bind texture
		glBindTexture(GL_TEXTURE_2D, currentLoadingTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glColor4f(1,1,1,0.9);
		glTranslatef(0.03f,0.885f,0.0f);

		//draw about screen
		glBegin(GL_QUADS); 
			glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f, 0.07f); //top-left
			glTexCoord2f( 1.0f, 1.0f );glVertex2f( 0.07f, 0.07f); //top-right
			glTexCoord2f( 1.0f, 0.0f );glVertex2f( 0.07f, 0.0f); //bottom-right
			glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f, 0.0f); //bottom-left
		glEnd();

		glDisable(GL_TEXTURE_2D);

	glPopMatrix();
	
	/* Draw loading background */
	glPushMatrix();
		
		glColor4f(0.8f,0.8f,0.8f,0.2f);
		glTranslatef(0.02f,0.57f,0.0f);

		//draw about screen
		glBegin(GL_QUADS);
			glVertex2f( 0.0f, 0.4f); //top-left
			glVertex2f( 0.3f, 0.4f); //top-right
			glVertex2f( 0.3f, 0.3f); //bottom-right
			glVertex2f( 0.0f, 0.3f); //bottom-left
		glEnd();
		
		glColor3f(1,1,1);
		
	glPopMatrix();
	

	/* Draw loading text */
	glPushMatrix();
		glColor4f(0.9f,0.9f,0.9f,0.85f);
		glScalef(0.025, 0.05, 0.0f);
		glTranslatef(5.5, 18, 0);
		t3dDraw2D(loadingStateText, -1, 1, 0.2f);
	glPopMatrix();

	glColor4f(1,1,1,1);
	
}

/* Draw menu animation overlay */
void drawMenuAnimationOverlay(){

	subMenuAnimationCounter++;

	if(subMenuAnimationCounter==2){

		menuAnimationCounter++;

		if(menuAnimationCounter>(MAX_MENU_ANIMATION_FRAMES-1)){
			menuAnimationCounter=0;
		}else{
			currentMenuAnimationTexture=menuAnimationTexture[menuAnimationCounter];
		}
		
		subMenuAnimationCounter=0;

	}

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
	glBindTexture(GL_TEXTURE_2D, currentMenuAnimationTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//draw
	glBegin(GL_QUADS); 
		glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f, 0.0f); //top-left
		glTexCoord2f( 1.0f, 1.0f );glVertex2f( 1.0f, 0.0f); //top-right
		glTexCoord2f( 1.0f, 0.0f );glVertex2f( 1.0f,  1.0f); //bottom-right
		glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f,  1.0f); //bottom-left
	glEnd();

	//enables/disables
	glDisable(GL_TEXTURE_2D);

}

/* Draw room border animation */
void drawRoomBorder(){

	subBorderAnimationCounter++;

	if(subBorderAnimationCounter==1){

		borderAnimationCounter++;

		if(borderAnimationCounter>(MAX_BORDER_ANIMATION_FRAMES-1)){
			borderAnimationCounter=0;
		}else{
			currentBorderAnimationTexture=borderAnimationTexture[borderAnimationCounter];
		}
		
		subBorderAnimationCounter=0;

	}

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
	glBindTexture(GL_TEXTURE_2D, currentBorderAnimationTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//draw
	glBegin(GL_QUADS); 
		glTexCoord2f( 0.0f, 1.0f );glVertex2f( 0.0f, 0.0f); //top-left
		glTexCoord2f( 1.0f, 1.0f );glVertex2f( 1.0f, 0.0f); //top-right
		glTexCoord2f( 1.0f, 0.0f );glVertex2f( 1.0f,  1.0f); //bottom-right
		glTexCoord2f( 0.0f, 0.0f );glVertex2f( 0.0f,  1.0f); //bottom-left
	glEnd();

	//enables/disables
	glDisable(GL_TEXTURE_2D);

}

/* Draw chatroom client's text to send to chatroom */
void drawRoomClientText(){

	glViewport(20, 19, 510, 80);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);


	glPushMatrix();
		glColor4f(0.0f, 0.0f, 0.8f, 0.6f);
		glScalef(0.02, 0.15, 0.0f);
		glTranslatef(1.0, 5.0, 0.0);
		float offsetRoomIdText=t3dDraw2D(user_input_id+":  ", -1, 1, 0.2f);
		textEnterLogTextLen=offsetRoomIdText;
	glPopMatrix();
	
	glPushMatrix();
		glColor4f(0.0f, 0.0f, 0.8f, 0.6f);
		glScalef(0.02, 0.15, 0.0f);
		glTranslatef(offsetRoomIdText+0.75, 5.0, 0.0);
		float toffsetRoomIdText=t3dDraw2D(textEnterLogOne, -1, 1, 0.2f);
	glPopMatrix();

	glPushMatrix();
		glColor4f(0.0f, 0.0f, 0.8f, 0.6f);
		glScalef(0.02, 0.15, 0.0f);
		glTranslatef(0.75, 3.25, 0.0);
		float ttoffsetRoomIdText=t3dDraw2D(textEnterLogTwo, -1, 1, 0.2f);
	glPopMatrix();

}

/* Draw chatroom message list scrollbar */
void drawRoomMessageListScrollbar(){

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

/* Draw chatroom message list */
void drawRoomMessageListText(){

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
	
		glPushMatrix();
			glColor4f(0.0f, 0.0f, 0.8f, 0.6f);
			glScalef(0.025, 0.75, 0.0f);
			glTranslatef(0.15, 0.0, 0);
			float temp=t3dDraw2D(messageListShortList[i], -1, 1, 0.2f);
		glPopMatrix();

		//space between messages in message list
		j-=20.0f;
	}


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

/* Draw chatroom user list scrollbar */
void drawRoomUserListScrollbar(){

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

/* Draw chatroom user list header text */
void drawRoomUserListHeaderText(){
	
	glViewport(514, 430, 80, 23);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	glColor4f(0.0f,0.0f,0.1f,0.05f);

	glPushMatrix();
		glBegin(GL_QUADS);
			glVertex2f(0.0,0.0);
			glVertex2f(1.0,0.0);
			glVertex2f(1.0,1.0);
			glVertex2f(0.0,1.0);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glColor4f(0.0f, 0.0f, 0.8f, 0.6f);
		glScalef(0.2, 1.0, 0.0f);
		glTranslatef(0.85, 0.0, 0);
		float temp=t3dDraw2D("Users", -1, 1, 0.2f);
	glPopMatrix();


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

/* Draw chatroom user list text */
void drawRoomUserListText(){

	float j=400.0f;
	for(int i=0;i<MAX_DISPLAY_USERS;i++){

		glViewport(514, j, 80, 20);
		glClearColor(1,1,1,0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluOrtho2D(0, 1, 0, 1);

		glColor4f(0.0f,0.0f,0.1f,0.05f);

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
			glColor4f(0.0f, 0.0f, 0.8f, 0.6f);
			glScalef(0.125, 0.85, 0.0f);
			glTranslatef(0.15, 0.0, 0);
			float temp=t3dDraw2D(userShortList[i], -1, 1, 0.2f);
		glPopMatrix();

		j-=30.0f;
	}


	glColor4f(1.0f,1.0f,1.0f,1.0f);

}

/* Draw chatroom information text */
void drawRoomInformationText(){

	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(1,1,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);


	glPushMatrix();
		glColor4f(0.5f, 0.8f, 0.9f, 0.5f);
		glScalef(0.025, 0.025, 0.0f);
		glTranslatef(1.5, 8.75, 0.0);
		float offsetRoomIdText=t3dDraw2D("User: "+user_input_id+" | ", -1, 1, 0.2f);
	glPopMatrix();

	glPushMatrix();
		glColor4f(0.5f, 0.8f, 0.9f, 0.5f);
		glScalef(0.025, 0.025, 0.0f);
		glTranslatef(offsetRoomIdText+1.5, 8.75, 0);
		float toffsetRoomIdText=t3dDraw2D("Room #: "+to_string(room_id), -1, 1, 0.2f);
	glPopMatrix();
}

/*
	Draw state functions
*/

/* Draw menu */
void drawStateMenu(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	
	// title image
	drawBackground(aboutTexture);

	// host box
	drawHostBox(hostBoxAlive);

	// join box
	drawJoinBox(joinBoxAlive);

	// exit box
	drawExitBox(exitBoxAlive);

	// clear color
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	
	// animation overlay
	drawMenuAnimationOverlay();
}

/* Draw state to retreive menu selection */
void drawStateRoom(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawBackground(roomTexture);

	drawRoomMessageListScrollbar();

	drawRoomUserListScrollbar();

	drawRoomUserListHeaderText();

	drawRoomUserListText();

	drawRoomClientText();

	drawRoomMessageListText();

	drawRoomInformationText();

	drawRoomBorder();

	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

/* Draw state to retreive room key of room to join */
void drawStateRoomKey(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	// title image
	drawBackground(aboutTexture);

	// room box
	drawRoomBox(roomBoxAlive, room_input_id);

	// back box
	drawBackBox(backBoxAlive);

	// clear color
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	
	// animation overlay
	drawMenuAnimationOverlay();
}

/* Draw state to retreive desired username of client */
void drawStateUsername(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);

	// title image
	drawBackground(aboutTexture);

	// user box
	drawUserBox(userBoxAlive, user_input_id);

	// back box
	drawBackBox(backBoxAlive);

	// clear color
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	
	// animation overlay
	drawMenuAnimationOverlay();

}

/* Draw state to retreive server IP address */
void drawStateHostname(){

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0,  windowWidth, windowHeight);
	glClearColor(1,0,1,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glClearColor(1,0,1,0);
	// title image
	drawBackground(aboutTexture);

	// user box
	drawHostnameBox(hostnameBoxAlive, hostname_input_id);

	// exit box
	drawExitBox(exitBoxAlive);

	// clear color
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	
	// animation overlay
	drawMenuAnimationOverlay();

}

/* Draw state to exit client */
void drawStateExit(){
	t3dCleanup();
	exit(0);
}

/* Draw current state */
void drawState() {
	
	// draw current state
	if(currentState==STATE_ROOM){
		drawStateRoom();
	}else if(currentState==STATE_MENU){
		drawStateMenu();
		if(loadingState){
			drawLoadingScreen();
		}
	}else if(currentState==STATE_KEY){
		drawStateRoomKey();
		if(loadingState){
			drawLoadingScreen();
		}
	}else if(currentState==STATE_USERNAME){
		drawStateUsername();
		if(loadingState){
			drawLoadingScreen();
		}
	}else if(currentState==STATE_HOSTNAME){
		drawStateHostname();
		if(loadingState){
			drawLoadingScreen();
		}
	}else if(currentState==STATE_EXIT){
		drawStateExit();
	}

	//swap
	glutSwapBuffers();
}

/*
	Chatroom and server functions
*/

/* Add a message list update to chatroom */
void addToChat(std::string line){
	cout << "Add to chat: " << line << endl;

	//if string if in proper format
	if(line.find(':')!=string::npos){

		//find if string requires split into 2 parts
		float lineFontLength=0.0f;
		for(int i=0;i<line.size();i++){
			//calculate length the sum of each individual character's length in the string
			lineFontLength+=getFontCharWidth(line[i]);
		}
		
		//if the message string requires more than 1 line then split it, else add
		if(lineFontLength>MAX_ONE_LINE_CHARACTER_LENGTH){

			//find where to split the line into 2 smaller lines
			float splitLength=0.0f;
			int startCounterPos=0;
			int splitTotalCounterPos=0;
			int splitCounterPos=0;
						
			//while the message requires more lines, split it and add to chat
			while( (splitTotalCounterPos!=line.size()) ){

				//find position to split message
				while( (splitLength<=MAX_ONE_LINE_CHARACTER_LENGTH) && (splitTotalCounterPos!=line.size()) ){
					splitLength+=getFontCharWidth(line[splitCounterPos]);
					splitTotalCounterPos++;
					splitCounterPos++;
				}

				//dequeue chat log if chat log is at max messages in order to make room for new message
				if(messageList.size()==MAX_MESSAGES){
					messageList.erase(messageList.begin());
				}

				//add splitted chat message into message list
				messageList.push_back(line.substr(startCounterPos, splitCounterPos));

				//reset split counter and set new split position of string
				splitLength=0.0f;
				startCounterPos=splitTotalCounterPos;
				splitCounterPos=0;

			}
			

		}else{

			//add message to chat log
			messageList.push_back(line);
		}

	}else{

		//push line onto chat stack
		messageList.push_back(line);

	}

	//move chat log down, if new message in chat log would make message not visible
	if( (messageList.size()>MAX_DISPLAY_MESSAGES) && ((messageList.size()<(MAX_MESSAGES-MAX_DISPLAY_MESSAGES))) && (chatScrollPos>113.0f) ){
		chatScrollPos-=CHAT_SCROLL_INCREMENT;
	}
	
}

/* Parse comma deliminated chatroom data */
void parseCommaDeliminatedRoomData(char recvbuf[DEFAULT_BUFLEN]){
	
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
					cout << "Userlist size: " << userList.size() << endl;
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

}

/* Parse comma deliminated response packet from request to join room, return true if room successfully joined */
bool parseCommaDeliminatedMenuData(char recvbuf[DEFAULT_BUFLEN], string& roomId){
	
	bool success=false;
	string recvbufString(recvbuf);

	if(recvbufString[1]=='0'){
		success=true;
		recvbufString.erase(0,3);

		roomId=recvbufString.substr(0,recvbufString.find(',', 0));
		cout << "Room id: '" << roomId << "'" << endl;
	}else{
		success=false;
	}

	return success;

}

/* Loads texture into OpenGL 2d texture using SOIL */
GLuint SOILLoadTexture(std::string filename){

	int width, height, channels;
	GLuint tempTexture;

	unsigned char* data = SOIL_load_image( filename.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO );
	if ( data )
	{
		tempTexture = SOIL_create_OGL_texture( data, width, height, channels, SOIL_CREATE_NEW_ID, 0 );
	}

	/* check for an error during the load process */
	if( 0 == tempTexture )
	{
		printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
	}

	/* Free Image */
	SOIL_free_image_data( data );

	return tempTexture;

}

/* Initialize states, textures, and enables/disables */
void init(){

	cout << "~CLIENT~\n" << endl;

	//seed
	srand(time(NULL));

	//init font
	t3dInit();

	//set loading text
	loadingStateText="Loading...";

	//init game state at menu
	currentState=STATE_HOSTNAME;

	//init message short list
	for(int i=0;i<MAX_DISPLAY_MESSAGES;i++){
		messageListShortList[i]="";
	}

	//init user short list
	for(int i=0;i<MAX_DISPLAY_USERS;i++){
		userShortList[i]="";
	}

	/* Load textures */
	scrollTexture=SOILLoadTexture("scrollbar.png");
	roomTexture=SOILLoadTexture("border.png");
	aboutTexture=SOILLoadTexture("title.png");
	
	// loading icon animation
	for(int i=0; i<MAX_LOADING_FRAMES; i++){

		/* Load image */
		std::string str = "loading/loading00";
		str+=to_string(i+1);
		str+=".png";
		loadingTexture[i]=SOILLoadTexture(str);

	}
	drawLoadingScreen(); // draw animation once to fix animation glitch

	// menu title animation
	for(int i=0; i<MAX_MENU_ANIMATION_FRAMES; i++){

		/* Load image */
		std::string str = "menu/title00";
		str+=to_string(i+1);
		str+=".png";
		menuAnimationTexture[i]=SOILLoadTexture(str);

	}
	
	// border animation
	for(int i=0; i<MAX_BORDER_ANIMATION_FRAMES; i++){

		/* Load image */
		std::string str = "border/border00";
		str+=to_string(i+1);
		str+=".png";
		borderAnimationTexture[i]=SOILLoadTexture(str);

	}

	//enables/disables
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

/* Attempt to connect to server room */
void attemptServerRoomJoin() {
	
	loadingState=true;
	bool successfullyJoinedRoom=false;
	string packetRoomJoin="";
	string receivedRoomId="";

	// make packet
	if(HostOrJoin=="host"){

		//send 0(host)
		packetRoomJoin+=",0,";

		//send desired username to server
		string tempUserId = user_input_id;
		tempUserId.append(",");
		packetRoomJoin+=tempUserId;

	}else{

		//send 1(join)
		packetRoomJoin+=",1,";

		//send room number
		string tempRoomId = room_input_id;
		tempRoomId.append(",");
		packetRoomJoin+=tempRoomId;

		//send desired username to server
		string tempUserId = user_input_id;
		tempUserId.append(",");
		packetRoomJoin+=tempUserId;

	}

	// send request to join room packet
	send(ConnectSocket, packetRoomJoin.c_str(), strlen(packetRoomJoin.c_str())+2, 0);
	cout << "Send: '" << packetRoomJoin.c_str() << "'" << endl;

	// receive result from request to join room
	recv(ConnectSocket,recvbuf,recvbuflen,0);

	// if successful response to join room
	if(parseCommaDeliminatedMenuData(recvbuf, packetRoomJoin)){

		// set state to currently in room
		currentState=STATE_ROOM;

		// set room and user id
		room_id=atoi(packetRoomJoin.c_str());
		user_id=atoi(user_input_id.c_str());

		//change to non-blocking socket
		u_long iMode=1;
		ioctlsocket(ConnectSocket,FIONBIO,&iMode);

	}else{
		MessageBox(NULL,"Error - Room key is invalid or room contains a user with the same name as yours.","Server error",MB_OK);
	}

	loadingState=false;
	
}

/* Attempt to connect to server */
bool attemptServerConnect(){

	loadingState=true;

	// Initialize Winsock
    WSAStartup(MAKEWORD(2,2), &wsaData);

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(hostname_input_id.c_str(), DEFAULT_PORT, &hints, &result);

    // AoffsetRoomIdTextt to connect to address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
        }

        // Connect to server
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		loadingState=false;
        if (iResult == SOCKET_ERROR) {
			lastSocketErrorCode=WSAGetLastError();
			cout << "CAN'T CONNECT!" << ", Socket Error:" << lastSocketErrorCode << endl;

            closesocket(ConnectSocket);

			/* Display socket error message box*/
			char buffer[7];
			itoa(lastSocketErrorCode,buffer,10);
			std::string temp(buffer);
			std::string strPlusErrorCode = "Invalid server, socket error: "+temp;
			MessageBox(NULL,strPlusErrorCode.c_str(),"Server error",MB_OK);

			return false;
            continue;
        }else{
			currentState=STATE_MENU;
		}
        break;
    }

    freeaddrinfo(result);

	return true;
}

/* Update server */
void updateServer(){

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
	
	//if packet in not empty, parse it 
	if(iResult>0){
		cout << "Received: " << recvbuf << endl << endl;
			
		//update user list with new server data
		parseCommaDeliminatedRoomData(recvbuf);
	}

	
}

/* Update server, user and message list */
void update(int value) {

	/* Update server if in room */
	if(currentState==STATE_ROOM){
		updateServer();
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

/*
	OpenGL event functions
*/

/* Executed on mouse movement */
void mouseMovement(int x, int y) {
	
	if(currentState==STATE_MENU){

		//check if mouse is over host box
		hostBoxAlive=checkHostBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over join box
		joinBoxAlive=checkJoinBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over exit box
		exitBoxAlive=checkExitBoxAlive(x, y, windowWidth, windowHeight);

	}else if(currentState==STATE_KEY){

		//check if mouse is over room box
		roomBoxAlive=checkRoomBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

	}else if(currentState==STATE_USERNAME){

		//check if mouse is over user box
		userBoxAlive=checkUserBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

	}else if(currentState==STATE_HOSTNAME){

		//check if mouse is over hostname box
		hostnameBoxAlive=checkHostnameBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over back box
		backBoxAlive=checkBackBoxAlive(x, y, windowWidth, windowHeight);

		//check if mouse is over exit box
		exitBoxAlive=checkExitBoxAlive(x, y, windowWidth, windowHeight);

	}

}

/* Executed on mouse click */
void mouseClicked(int button, int state, int x, int y){

	// if not loading
	if(!loadingState){

		// handle mouse events for current state
		if(currentState==STATE_MENU){

			// if host box clicked
			if( checkHostBoxClicked(x, y, windowWidth, windowHeight, button, state) ){
				HostOrJoin="host";
				currentState=STATE_USERNAME;
			}
		
			// if join box clicked
			checkJoinBoxClicked(x, y, windowWidth, windowHeight, button, state, currentState);

			// if exit box clicked
			checkExitBoxClicked(x, y, windowWidth, windowHeight, button, state, currentState);

		}else if(currentState==STATE_ROOM){

			if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){

				// if clicked inside user scroll, return alive flag
				userScrollActive=isUserScrollActive(x,y,userScrollPos);

				// if clicked inside chat scroll, return alive flag
				chatScrollActive=isChatScrollActive(x,y,chatScrollPos);

				// if clicked inside text enter log, return alive flag
				textEnterLogActive=isTextEnterLogActive(x,y);

				// check "leave" button
				if(checkBackButtonClicked(x,y)){
					//send user back to menu
					currentState=STATE_MENU;
					firstTimeUserListSent=false;

					//reset to blocking socket
					u_long iMode=0;
					ioctlsocket(ConnectSocket,FIONBIO,&iMode);

					//erase message and user list
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

					//reset
					shutdown(ConnectSocket, SD_BOTH);
					WSACleanup();
					closesocket(ConnectSocket);

					//reconnect to server
					thread(attemptServerConnect).detach();
				
				}

			}

		}else if(currentState==STATE_KEY){
	
			// if room box clicked
			checkRoomBoxClicked(x, y, windowWidth, windowHeight, button, state, roomIdTextClicked);
	
			// if back box clicked
			checkBackBoxClicked(x, y, windowWidth, windowHeight, button, state, currentState);

		}else if(currentState==STATE_USERNAME){

			// if user box clicked
			checkUserBoxClicked(x, y, windowWidth, windowHeight, button, state, userIdTextClicked);
	
			// if back box clicked
			checkBackBoxClicked(x, y, windowWidth, windowHeight, button, state, currentState);

		}else if(currentState==STATE_HOSTNAME){

			// if hostname box clicked
			checkHostnameBoxClicked(x, y, windowWidth, windowHeight, button, state, hostnameIdTextClicked);

			// if exit box clicked
			checkExitBoxClicked(x, y, windowWidth, windowHeight, button, state, currentState);

		}

	}
	
}

/* Executed on mouse click held down */
void mouseClickedActive(int x, int y){

	if(currentState==STATE_ROOM){

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

/* Executed on any key press down */
void handleKeypress(unsigned char key, int x, int y){

	// if not loading
	if(!loadingState){

		// handle keyboard events for current state
		if(currentState==STATE_ROOM){

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

		}else if(currentState==STATE_KEY){
		
			if(roomIdTextClicked){
			
				if(key==8){
					//clear string
					room_input_id.clear();
					room_id_len=0;
				}else if(key==13){
					//enter key
					if(checkRoomId(room_input_id)){
						HostOrJoin="join";
						currentState=STATE_USERNAME;
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

		}else if(currentState==STATE_USERNAME){

			if(userIdTextClicked){

				if(key==8){
					//clear string
					user_input_id.clear();
					user_id_len=0;
				}else if(key==13){
					//enter key
					thread attempt(attemptServerRoomJoin);
					attempt.detach();
				}else{
					//if room id's char limit not exceeded, add key
					if(user_id_len<MAX_USER_ID_CHARS){
						//add key
						user_input_id+=key;
						user_id_len++;
					}
				}

			}

		}else if(currentState==STATE_HOSTNAME){
		
			if(key==8){
				//clear string
				hostname_input_id.clear();
				hostname_id_len=0;
			}else if(key==13){
				//enter key
				thread(attemptServerConnect).detach();
			}else{
				//if room id's char limit not exceeded, add key
				if(hostname_id_len<MAX_HOSTNAME_CHARS){
					//add key
					hostname_input_id+=key;
					hostname_id_len++;
				}
			}

		}
	}

	//escape key
	if (key==27){
		exit(0);
	}

}

/* Executed on any key press up */
void handleKeypressUp(unsigned char key, int x, int y){


}

/* Handle resize - Don't resize if client aoffsetRoomIdTextts to resize */
void handleResize(int w, int h) {
	//for resizeable
	//glViewport(0, 0, w/1.3, h);
	//windowWidth=w;
	//windowHeight=h;

	glutReshapeWindow(windowWidth,windowHeight);
}


/* Main OpenGL loop */
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Polished IRC 1.2.0");
	glewInit();
	GLenum err = glewInit();
    if(GLEW_OK != err) {
		cout << "glewInit failed." << endl;
    }

	init();
	glutDisplayFunc(drawState);
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