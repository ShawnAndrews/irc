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

//precondition: there is available room in room for new user
void Room::addUser(std::string username){

	int index=0;

	//find position for new user
	for(int i=0; i<MAX_USERS; i++){
		if(getUsername(i)==""){
			index=i;
			break;
		}
	}

	//set new user in slot
	setUsername(index, username);

	//set status update for all users
	for(int i=0; i<getUserListSize(); i++){

		//dont add username update status to your own client
		if(getUsername(i)!=username){
			enqueueUserStatus(i, "+"+username);
		}
		
	}
	
}

void Room::deleteUser(std::string username){

	int foundIndex=0;
	int moveAmount=0;

	//find index of user to be deleted
	for(int i=0; i<MAX_USERS; i++){
		if(getUsername(i)==username){
			foundIndex=i;
		}
	}

	//delete user
	setUsername(foundIndex, "");

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
		enqueueUserStatus(i, "-"+username);
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
		setUsername(i, "");
	}

}

int Room::getUserListSize(){

	int size=0;

	for(int i=0; i<MAX_USERS; i++){
		if(getUsername(i)!=""){
			size++;
		}
	}
	cout << endl;
	return size;

}

int Room::getUserIndexFromUserList(std::string username){

	int counter=-1;

	for(int i=0; i<MAX_USERS; i++){
		if(getUsername(i)==username){
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

/* OPERATIONS ON USER */

std::string Room::getUsername(int index){
	return users[index].username;
}

void Room::setUsername(int index, std::string name){
	users[index].username=name;
}

int Room::sizeOfMessageQueue(int index){
	return users[index].message_queue.size();
}

int Room::sizeOfUserQueue(int index){
	return users[index].user_queue.size();
}

void Room::enqueueMessage(int index, std::string str){
	users[index].message_queue.push(str);
}

std::string Room::dequeueMessage(int index){

	if(users[index].message_queue.size()!=NULL){
		std::string returnPop=users[index].message_queue.front();
		users[index].message_queue.pop();
		return returnPop;
	}
	return "";
}

void Room::enqueueUserStatus(int index, std::string str){
	users[index].user_queue.push(str);
}

std::string Room::dequeueUserStatus(int index){

	if(users[index].user_queue.size()!=NULL){
		std::string returnPop=users[index].user_queue.front();
		users[index].user_queue.pop();
		return returnPop;
	}
	return "";
}