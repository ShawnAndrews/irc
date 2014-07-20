#include <iostream>
#include <GL/glut.h>
#include "text3d.h"
#include "imageloader.h"

#define MAX_ROOM_ID_CHARS 6
#define MAX_USER_ID_CHARS 8
#define MAX_HOSTNAME_CHARS 15
#define TEXT_ENTER_WIDTH_MAX 48.0f
#define MAX_ONE_LINE_CHARACTER_LENGTH 39.0f
#define CHAT_SCROLL_INCREMENT 3;
enum state{STATE_MENU, STATE_USERNAME, STATE_HOSTNAME, STATE_KEY, STATE_ROOM};

const int windowHeight=480;
const int windowWidth=640;

GLuint loadTexture(Image* image) {
	GLuint textureId;
	glGenTextures(1, &textureId); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
				 0,                            //0 for now
				 GL_RGB,                       //Format OpenGL uses for image
				 image->width, image->height,  //Width and height
				 0,                            //The border of the image
				 GL_RGB, //GL_RGB, because pixels are stored in RGB format
				 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
				                   //as unsigned numbers
				 image->pixels);               //The actual pixel data
	return textureId; //Returns the id of the texture
}

bool pointInRectangle(float px,float py, float rx1, float rx2, float rx3, float rx4, float ry1, float ry2, float ry3, float ry4){

	if( (((px > rx1) && (px < rx2)) && ((px < rx3) && (px > rx4))) 
		&& (((py < ry1) && (py > ry2)) && ((py > ry3) && (py < ry4))) ){
		return true;
	}else{
		return false;
	}
}

bool checkRoomId(std::string room_id){
	const char* cStr = room_id.c_str();
	int tempLength = -1;
	if( (atoi(cStr)<=999999) && (atoi(cStr)>=0) ){
		tempLength++;
	}
	for(int i=0; i<room_id.length(); i++){
		if( (room_id[i]=='1') || (room_id[i]=='2') || (room_id[i]=='3') || (room_id[i]=='4') || (room_id[i]=='5') || (room_id[i]=='6') || (room_id[i]=='7') || (room_id[i]=='8') || (room_id[i]=='9') || (room_id[i]=='0') ){
			tempLength++;
		}
	}

	if(tempLength==room_id.length()){
		return true;
	}else{
		return false;
	}

}

void checkRoomBoxClicked(int x, int y, int windowWidth, int windowHeight, int button, int state, bool& roomIdTextClicked){

	if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
		if(pointInRectangle(x,y,
			0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
			0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
			roomIdTextClicked=true;
		}else{
			roomIdTextClicked=false;
		}
	}

}

void checkUserBoxClicked(int x, int y, int windowWidth, int windowHeight, int button, int state, bool& userIdTextClicked){

	if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
		if(pointInRectangle(x,y,
			0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
			0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
			userIdTextClicked=true;
		}else{
			userIdTextClicked=false;
		}
	}

}

void checkHostnameBoxClicked(int x, int y, int windowWidth, int windowHeight, int button, int state, bool& hostnameIdTextClicked){

	if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
		if(pointInRectangle(x,y,
			0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
			0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
			hostnameIdTextClicked=true;
		}else{
			hostnameIdTextClicked=false;
		}
	}

}

bool checkHostBoxClicked(int x, int y, int windowWidth, int windowHeight, int button, int& click_state){

	if(button==GLUT_LEFT_BUTTON && click_state==GLUT_DOWN){
		if(pointInRectangle(x,y,
			0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
			0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
			return true;
		}
	}
	return false;
}

void checkJoinBoxClicked(int x, int y, int windowWidth, int windowHeight, int button, int click_state, state& gameState){

	if(button==GLUT_LEFT_BUTTON && click_state==GLUT_DOWN){
		if(pointInRectangle(x,y,
			0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
			0.66*windowHeight,0.58*windowHeight,0.58*windowHeight,0.66*windowHeight)){
			gameState=STATE_KEY;
		}
	}

}

void checkBackBoxClicked(int x, int y, int windowWidth, int windowHeight, int button, int click_state, state& gameState){

	if(button==GLUT_LEFT_BUTTON && click_state==GLUT_DOWN){
		if(pointInRectangle(x,y,
			0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
			0.66*windowHeight,0.58*windowHeight,0.58*windowHeight,0.66*windowHeight)){
			gameState=STATE_MENU;
		}
	}

}

bool checkBackButtonClicked(int x, int y){
	//glViewport(556, 19, 64, 80);
	if(pointInRectangle(x,y,
		0.86*windowWidth,0.96*windowWidth,0.96*windowWidth,0.86*windowWidth,
		0.97*windowHeight,0.80*windowHeight,0.80*windowHeight,0.97*windowHeight)){
		return true;
	}
	return false;
}

void checkExitBoxClicked(int x, int y, int windowWidth, int windowHeight, int button, int click_state, state& gameState){

	// if exit box clicked
	if(button==GLUT_LEFT_BUTTON && click_state==GLUT_DOWN){
		if(pointInRectangle(x,y,
			0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
			0.79*windowHeight,0.71*windowHeight,0.71*windowHeight,0.79*windowHeight)){
			exit(0);
		}
	}

}

bool isUserScrollActive(int x, int y, int userScrollPos){
	if(pointInRectangle(x,y,
	600.0f,620.0f,620.0f,600.0f,
	windowHeight-userScrollPos,windowHeight-userScrollPos-40.0f,windowHeight-userScrollPos-40.0f,windowHeight-userScrollPos)){
		return true;
	}
	return false;
}

bool isChatScrollActive(int x, int y, int chatScrollPos){
	if(pointInRectangle(x,y,
	470.0f,490.0f,490.0f,470.0f,
	windowHeight-chatScrollPos,windowHeight-chatScrollPos-40.0f,windowHeight-chatScrollPos-40.0f,windowHeight-chatScrollPos)){
		return true;
	}
	return false;
}

bool isTextEnterLogActive(int x, int y){
	if(pointInRectangle(x,y,
	20.0f,530.0f,530.0f,20.0f,
	460.0f,380.0f,380.0f,460.0f)){
		return true;
	}
	return false;
}

bool checkRoomBoxAlive(int x, int y, int windowWidth, int windowHeight){

	if(pointInRectangle(x,y,
		0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
		0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
		return true;
	}else{
		return false;
	}

}

bool checkUserBoxAlive(int x, int y, int windowWidth, int windowHeight){

	if(pointInRectangle(x,y,
		0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
		0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
		return true;
	}else{
		return false;
	}

}

bool checkHostnameBoxAlive(int x, int y, int windowWidth, int windowHeight){

	if(pointInRectangle(x,y,
		0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
		0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
		return true;
	}else{
		return false;
	}

}

bool checkHostBoxAlive(int x, int y, int windowWidth, int windowHeight){

	if(pointInRectangle(x,y,
		0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
		0.535*windowHeight,0.45*windowHeight,0.45*windowHeight,0.535*windowHeight)){
		return true;
	}else{
		return false;
	}

}

bool checkJoinBoxAlive(int x, int y, int windowWidth, int windowHeight){

	if(pointInRectangle(x,y,
		0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
		0.66*windowHeight,0.58*windowHeight,0.58*windowHeight,0.66*windowHeight)){
		return true;
	}else{
		return false;
	}

}

bool checkBackBoxAlive(int x, int y, int windowWidth, int windowHeight){

	if(pointInRectangle(x,y,
		0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
		0.66*windowHeight,0.58*windowHeight,0.58*windowHeight,0.66*windowHeight)){
		return true;
	}else{
		return false;
	}

}

bool checkExitBoxAlive(int x, int y, int windowWidth, int windowHeight){

	if(pointInRectangle(x,y,
		0.41*windowWidth,0.56*windowWidth,0.56*windowWidth,0.41*windowWidth,
		0.79*windowHeight,0.71*windowHeight,0.71*windowHeight,0.79*windowHeight)){
		return true;
	}else{
		return false;
	}

}

void drawHostBox(bool hostBoxAlive){

	if(hostBoxAlive){
		glColor4f(0.9f, 0.9f, 0.9f, 0.4f);
	}else{
		glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
	}
	glBegin(GL_QUADS);
		glVertex2f(0.41, 0.465); //bottom-left
		glVertex2f(0.41, 0.55); //top-left

		glVertex2f(0.41, 0.55); //top-left
		glVertex2f(0.56, 0.55); //top-right

		glVertex2f(0.56, 0.55); //top-right
		glVertex2f(0.56, 0.465); //bottom-right

		glVertex2f(0.56, 0.465); //bottom-right
		glVertex2f(0.41, 0.465); //bottom-left
	glEnd();
	
	glPushMatrix();
		glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
		glScalef(0.04, 0.04, 0.0f);
		glTranslatef(10.65, 12.2, 0);
		t3dDraw2D("Host", -1, 1, 0.2f);
	glPopMatrix();

}

void drawRoomBox(bool roomBoxAlive, std::string& room_id){

	if(roomBoxAlive){
		glColor4f(0.9f, 0.9f, 0.9f, 0.4f);
	}else{
		glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
	}
	glBegin(GL_QUADS);
		glVertex2f(0.41, 0.465); //bottom-left
		glVertex2f(0.41, 0.55); //top-left

		glVertex2f(0.41, 0.55); //top-left
		glVertex2f(0.56, 0.55); //top-right

		glVertex2f(0.56, 0.55); //top-right
		glVertex2f(0.56, 0.465); //bottom-right

		glVertex2f(0.56, 0.465); //bottom-right
		glVertex2f(0.41, 0.465); //bottom-left
	glEnd();

	if(room_id.empty()){
		glPushMatrix();
			glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
			glScalef(0.02, 0.04, 0.0f);
			glTranslatef(21.65, 12.2, 0);
			t3dDraw2D("Room #", -1, 1, 0.2f);
		glPopMatrix();
	}else{
		glPushMatrix();
			glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
			glScalef(0.02, 0.04, 0.0f);
			glTranslatef(20.65, 12.2, 0);
			t3dDraw2D(room_id, -1, 1, 0.2f);
		glPopMatrix();
	}

}

void drawUserBox(bool userBoxAlive, std::string& user_id){

	if(userBoxAlive){
		glColor4f(0.9f, 0.9f, 0.9f, 0.4f);
	}else{
		glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
	}
	glBegin(GL_QUADS);
		glVertex2f(0.41, 0.465); //bottom-left
		glVertex2f(0.41, 0.55); //top-left

		glVertex2f(0.41, 0.55); //top-left
		glVertex2f(0.56, 0.55); //top-right

		glVertex2f(0.56, 0.55); //top-right
		glVertex2f(0.56, 0.465); //bottom-right

		glVertex2f(0.56, 0.465); //bottom-right
		glVertex2f(0.41, 0.465); //bottom-left
	glEnd();

	if(user_id.empty()){
		glPushMatrix();
			glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
			glScalef(0.02, 0.04, 0.0f);
			glTranslatef(21.15, 12.2, 0);
			float temp=t3dDraw2D("Username", -1, 1, 0.2f);
		glPopMatrix();
	}else{
		glPushMatrix();
			glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
			glScalef(0.02, 0.04, 0.0f);
			glTranslatef(20.65, 12.2, 0);
			t3dDraw2D(user_id, -1, 1, 0.2f);
		glPopMatrix();
	}

}

void drawHostnameBox(bool hostnameBoxAlive, std::string& hostname_id){

	if(hostnameBoxAlive){
		glColor4f(0.9f, 0.9f, 0.9f, 0.4f);
	}else{
		glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
	}
	glBegin(GL_QUADS);
		glVertex2f(0.41, 0.465); //bottom-left
		glVertex2f(0.41, 0.55); //top-left

		glVertex2f(0.41, 0.55); //top-left
		glVertex2f(0.56, 0.55); //top-right

		glVertex2f(0.56, 0.55); //top-right
		glVertex2f(0.56, 0.465); //bottom-right

		glVertex2f(0.56, 0.465); //bottom-right
		glVertex2f(0.41, 0.465); //bottom-left
	glEnd();

	if(hostname_id.empty()){
		glPushMatrix();
			glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
			glScalef(0.02, 0.04, 0.0f);
			glTranslatef(21.15, 12.2, 0);
			float temp=t3dDraw2D("Server IP", -1, 1, 0.2f);
		glPopMatrix();
	}else{
		glPushMatrix();
			glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
			glScalef(0.02, 0.04, 0.0f);
			glTranslatef(20.65, 12.2, 0);
			t3dDraw2D(hostname_id, -1, 1, 0.2f);
		glPopMatrix();
	}

}

void drawJoinBox(bool joinBoxAlive){
	
	if(joinBoxAlive){
		glColor4f(0.9f, 0.9f, 0.9f, 0.4f);
	}else{
		glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
	}
	glBegin(GL_QUADS); 
		glVertex2f(0.41, 0.34); //bottom-left
		glVertex2f(0.41, 0.42); //top-left

		glVertex2f(0.41, 0.42); //top-left
		glVertex2f(0.56, 0.42); //top-right

		glVertex2f(0.56, 0.42); //top-right
		glVertex2f(0.56, 0.34); //bottom-right

		glVertex2f(0.56, 0.34); //bottom-right
		glVertex2f(0.41, 0.34); //bottom-left
	glEnd();

	glPushMatrix();
		glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
		glScalef(0.04, 0.04, 0.0f);
		glTranslatef(10.65, 9.0, 0);
		t3dDraw2D("Join", -1, 1, 0.2f);
	glPopMatrix();

}

void drawBackBox(bool backBoxAlive){
	
	if(backBoxAlive){
		glColor4f(0.9f, 0.9f, 0.9f, 0.4f);
	}else{
		glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
	}
	glBegin(GL_QUADS); 
		glVertex2f(0.41, 0.34); //bottom-left
		glVertex2f(0.41, 0.42); //top-left

		glVertex2f(0.41, 0.42); //top-left
		glVertex2f(0.56, 0.42); //top-right

		glVertex2f(0.56, 0.42); //top-right
		glVertex2f(0.56, 0.34); //bottom-right

		glVertex2f(0.56, 0.34); //bottom-right
		glVertex2f(0.41, 0.34); //bottom-left
	glEnd();

	glPushMatrix();
		glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
		glScalef(0.04, 0.04, 0.0f);
		glTranslatef(10.65, 9.0, 0);
		float temp=t3dDraw2D("Back", -1, 1, 0.2f);
	glPopMatrix();

}

void drawExitBox(bool exitBoxAlive){
	
	if(exitBoxAlive){
		glColor4f(0.9f, 0.9f, 0.9f, 0.4f);
	}else{
		glColor4f(0.7f, 0.7f, 0.7f, 0.4f);
	}
	glBegin(GL_QUADS); 
		glVertex2f(0.41, 0.208); //bottom-left
		glVertex2f(0.41, 0.29); //top-left

		glVertex2f(0.41, 0.29); //top-left
		glVertex2f(0.56, 0.29); //top-right

		glVertex2f(0.56, 0.29); //top-right
		glVertex2f(0.56, 0.208); //bottom-right

		glVertex2f(0.56, 0.208); //bottom-right
		glVertex2f(0.41, 0.208); //bottom-left
	glEnd();

	glPushMatrix();
		glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
		glScalef(0.04, 0.04, 0.0f);
		glTranslatef(10.65, 5.75, 0);
		t3dDraw2D("Exit", -1, 1, 0.2f);
	glPopMatrix();

}