#ifndef SOCKET_H
#define SOCKET_H

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "Ws2_32.lib")
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <time.h>
#include <vector>
#include <sstream>
#include <queue>

using namespace std;

#define RANDOM_KEY_RANGE 1000000
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "3307"
#define MAX_ROOMS 10
#define MAX_USERS 100
#define DEFAULT_PACKET_SCHEME "U:M:"
enum state{STATE_MENU, STATE_ROOM};

struct Room{

private:
	struct User{

	public:
		std::string username;
		queue<std::string> message_queue;
		queue<std::string> user_queue;

	};
	
	int room_size;
	int room_id;
	User users[MAX_USERS];

public:
	Room::Room():room_id(NULL), room_size(NULL){initUserList();}

	/* OPERATIONS ON ROOM */ 
	
	//room id operations
	void setRoomId(int);
	int getRoomId();

	//room size operations
	void setRoomSize(int);
	int getRoomSize();

	//add user operations
	void addUser(std::string);
	void deleteUser(std::string username);

	//get counter from username
	int getUserIndexFromUserList(std::string);

	//initialize user list's usernames to ""
	void initUserList();

	//is there room to another user?
	bool isAvailableUserSlot();

	//delete room
	void deleteRoom();

	/* OPERATIONS ON USERS */ 

	//get and set username
	std::string getUsername(int index);
	void setUsername(int index, std::string name);

	//enqueue and dequeue messages
	void enqueueMessage(int index, std::string);
	std::string dequeueMessage(int index);
		
	//enqueue and dequeue user statuses
	void enqueueUserStatus(int index, std::string);
	std::string dequeueUserStatus(int index);

	//get size of message and user queue
	int sizeOfMessageQueue(int index);
	int sizeOfUserQueue(int index);

};

struct ClientParams{
	
private:
	SOCKET sock;
	int hostroom_slot;
    int hostroom_id;
	std::string uniqueuser_id;
	state STATE;
	
public:
	ClientParams::ClientParams():sock(INVALID_SOCKET), hostroom_id(NULL), STATE(STATE_MENU){};

	SOCKET getSocket();
	void setSocket(SOCKET);
	
	void setHostRoomSlot(int);
	int getHostRoomSlot();

	int setUniqueHostRoomId(Room*);
	void setHostRoomId(int);
	int getHostRoomId();

	state getState();
	void setState(state);

	void setUniqueUserId(std::string);
	std::string getUniqueUserId();
};

#endif