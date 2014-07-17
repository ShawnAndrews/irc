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

		id=rand()%999999+1;

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

int Room::User::sizeOfMessageQueue(){
	return message_queue.size();
}

int Room::User::sizeOfUserQueue(){
	return user_queue.size();
}

bool Room::addUser(std::string username){

	if(users.size()!=MAX_USERS){
		users.push_back(User(username));
		
		//update user status
		for(int i=0;i<users.size();i++){
			
			//dont add to your own username to queue, because initial user list was already sent includeing your username 
			if(users.at(i).getUsername()!=username){
				users.at(i).enqueueUserStatus("+"+username);
			}
		}

		return users.size()-1;
	}
	return -1;
}


bool Room::deleteUser(std::string username){

	int counter=0;

	for(std::vector<User>::iterator currentUser = users.begin(); currentUser != users.end(); ++currentUser){
		
		if( (currentUser->getUsername()==username) ){
			users.erase(users.begin()+counter);

			//update user status
			for(int i=0;i<users.size();i++){
				users.at(i).enqueueUserStatus("-"+username);
			}

			return true;
		}
		counter++;
	}
	return false;
}

void Room::deleteRoom(){

	//delete all users
	for(std::vector<User>::iterator currentUser = users.begin(); currentUser != users.end(); ++currentUser){
		users.erase(currentUser);
	}
	
	//reset room id
	room_id=0;

}

int Room::getCounterFromUsername(std::string username){

	int counter=0;

	for(std::vector<User>::iterator currentUser = users.begin(); currentUser != users.end(); ++currentUser){
		
		if( (currentUser->getUsername()==username) ){
			return counter;
		}
		counter++;
	}
	return -1;
}

bool Room::isAvailableUserSlot(){

	if( users.size()!=MAX_USERS ){
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