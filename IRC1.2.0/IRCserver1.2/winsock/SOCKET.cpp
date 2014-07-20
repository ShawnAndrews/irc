#include "SOCKET.h"

SOCKET ClientParams::getSocket(){
	return sock;
}

void ClientParams::setSocket(SOCKET socket){
	sock=socket;
}

int ClientParams::setUniqueHostRoomId(Room* serverData){

	int id;

	bool passed=false;
	while(!passed){

		id=rand()%RANDOM_KEY_RANGE;

		passed=true;
		for(int i=0;i<MAX_ROOMS;i++){
			if(serverData[i].getRoomId()==id){
				passed=false;
			}
		}
	}

	hostroom_id=id;

	return id;
}

void ClientParams::setHostRoomSlot(int id){
	hostroom_slot=id;
}

int ClientParams::getHostRoomSlot(){
	return hostroom_slot;
}

int ClientParams::getHostRoomId(){
	return hostroom_id;
}

void ClientParams::setUniqueUserId(std::string x){
	uniqueuser_id=x;
}

std::string ClientParams::getUniqueUserId(){
	return uniqueuser_id;
}

void Room::setRoomId(int x){
	room_id=x;
}

int Room::getRoomId(){
	return room_id;
}

std::string Room::User::getUsername(){
	return username;
}

void Room::User::setUsername(std::string name){
	username=name;
}

int Room::User::sizeOfMessageQueue(){
	return message_queue.size();
}

int Room::User::sizeOfUserQueue(){
	return user_queue.size();
}

//precondition: there is available room in room for new user
void Room::addUser(std::string username){

	int index=0;

	//find position for new user
	for(int i=0; i<MAX_USERS; i++){
		if(users[i].getUsername()==""){
			index=i;
			break;
		}
	}

	//set new user in slot
	users[index].setUsername(username);

	//set status update for all users
	for(int i=0; i<getUserListSize(); i++){

		//dont add username update status to your own client
		if(users[i].getUsername()!=username){
			users[i].enqueueUserStatus("+"+username);
		}
		
	}
	
}

void Room::deleteUser(std::string username){

	int foundIndex=0;
	int moveAmount=0;

	//find index of user to be deleted
	for(int i=0; i<MAX_USERS; i++){
		if(users[i].getUsername()==username){
			foundIndex=i;
		}
	}

	//delete user
	users[foundIndex].setUsername("");

	//set amount of users to move down
	moveAmount=getUserListSize()-foundIndex;

	//move users above in list down 1 slot
	while(moveAmount>0){
		
		//move down
		users[foundIndex+1]=users[foundIndex];

		//decrease move amount
		foundIndex--;
		moveAmount--;

	}

	//set status update for all users
	for(int i=0; i<getUserListSize(); i++){
		users[i].enqueueUserStatus("-"+username);
	}

}

void Room::deleteRoom(){

	//delete all users
	initUserList();
	
	//reset room id
	room_id=NULL;

}

void Room::initUserList(){

	for(int i=0; i<MAX_USERS; i++){
		users[i].setUsername("");
	}

}

int Room::getUserListSize(){

	int size=0;

	for(int i=0; i<MAX_USERS; i++){
		if(users[i].getUsername()!=""){
			size++;
		}
	}
	cout << endl;
	return size;

}

int Room::getUserIndexFromUserList(std::string username){

	int counter=-1;

	for(int i=0; i<MAX_USERS; i++){
		if(users[i].getUsername()==username){
			return i;
		}
	}

}

bool Room::isAvailableUserSlot(){

	if( getUserListSize()!=MAX_USERS ){
		return true;
	}
	return false;
}

void Room::User::enqueue(std::string str){
	message_queue.push(str);
}

std::string Room::User::dequeue(){

	if(message_queue.size()!=NULL){
		std::string returnPop=message_queue.front();
		message_queue.pop();
		return returnPop;
	}
	return "";
}

void Room::User::enqueueUserStatus(std::string str){
	user_queue.push(str);
}

std::string Room::User::dequeueUser(){

	if(user_queue.size()!=NULL){
		std::string returnPop=user_queue.front();
		user_queue.pop();
		return returnPop;
	}
	return "";
}