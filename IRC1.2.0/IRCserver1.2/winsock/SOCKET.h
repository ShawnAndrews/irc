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
#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "3307"
#define MAX_ROOMS 10
#define MAX_USERS 100
#define DEFAULT_PACKET_SCHEME "U:M:"

struct Room{

private:
	struct User{

	private:
		std::string username;
		queue<std::string> message_queue;
		queue<std::string> user_queue;

	public:
		User::User(){};
		User::User(std::string str):username(str){};

		//get and set username
		std::string getUsername();
		void setUsername(std::string name);

		//enqueue and dequeue messages
		void enqueue(std::string);
		std::string dequeue();
		
		//enqueue and dequeue user statuses
		void enqueueUserStatus(std::string);
		std::string dequeueUser();

		//get size of message and user queue
		int sizeOfMessageQueue();
		int sizeOfUserQueue();

	};
	
	
	int room_id;

public:
	Room::Room():room_id(NULL){initUserList();}
	
	User users[MAX_USERS];
	
	//room id operations
	void setRoomId(int);
	int getRoomId();

	//add user operations
	void addUser(std::string);
	void deleteUser(std::string username);

	//get counter from username
	int getUserIndexFromUserList(std::string);

	//get user list size
	int getUserListSize();

	//initialize user list's usernames to ""
	void initUserList();

	//is there room to another user?
	bool isAvailableUserSlot();

	//delete room
	void deleteRoom();

};

struct ClientParams{
	
private:
	SOCKET sock;
	int hostroom_slot;
    int hostroom_id;
	std::string uniqueuser_id;
	
public:
	ClientParams::ClientParams():sock(INVALID_SOCKET), hostroom_id(NULL){};

	SOCKET getSocket();
	void setSocket(SOCKET);
	
	void setHostRoomSlot(int);
	int getHostRoomSlot();

	int setUniqueHostRoomId(Room*);
	int getHostRoomId();

	void setUniqueUserId(std::string);
	std::string getUniqueUserId();
};

#endif