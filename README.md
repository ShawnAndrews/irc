Polished IRC 1.2.0
===

![alt tag](http://oi60.tinypic.com/5zfprn.jpg)


## What is Polished IRC?

This program can create chatrooms that hold communication between multiple users. Each chatroom is only accessible by having knowledge of the server's IP which is managing the chatroom and that chatroom's key. Messages in a chatroom are transmitted to other users via unencrypted and uncompressed TCP packets.

## How it works

1. Start the server, which is automatically assigned to port 3307. 
2. Run the client. 
3. From the client, enter the desired server's WAN IP address, or LAN IP if you're in the same network as the server to connect to it.
4. Select Host, Join, or Exit.
5. If you selected Join, enter the chatroom's key, your desired username and press enter. If you selected Host, simply enter your desired username and press enter.
6. You are now in a chatroom.

## Features

* Reliable Winsock API using TCP.
* Ability to quickly create a room and allow others to join your room.
* Able to support 10 rooms at 100 users per room.
* Client-Server architecture
* 640x480 resolution(windowed)
* Custom, hand-written GUI
* Simple to use with no account creation necessary.
* Chat log holds 170 of your room's past messages to browse.
* Lightweight. 

## Revisions

* IRC 1.0.0
  * Added complete program.
* IRC 1.1.0
  * Improved network efficiency.
  * Added ability to create and manage multi-threaded server(Port: 3307).
* IRC 1.2.0
  * Increased scability from switch to single-threaded server.
  * Improved performance and concurrency from client as it becomes multi-threaded.
  * Updated GUI.

## Screenshots

![alt tag](http://oi62.tinypic.com/2u5gjgy.jpg)
