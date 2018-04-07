# CHAT SERVER

General Information:
A concurrent server which can support upto 100 clients simultaneously. Server is continuosly
listening to the client requests and asks for the USERNAME as soon as a client makes request to
join the Chat Server. A new entry is made into the Server if a new UserName is found, but if an old
Username is found then it is recognised and the old connection is setup again.
Note : Press ENTER to see if any new messages have been received on the client side.

Commands used by user:
1) exit => to go offline
2) get_users => to know which users are corrently online and offline ( o – online ; x - offline )
3) <username>: <message> => to send message to any of the specified user in the get_user list
either online or offline. (<username> and : are written without spaces)
4) broad: <message> => to send message to all the users ever registered in the chat system
5) mk_grp:<group_name>:<username>:<username>.....(max 32 number of users) => to make a
group of users. Later this group name can be used to send message to the specified users.
(all wiithout spaces).
6) <group_name>: <message> => to send message to all the users in the group either online or
offline. (<groupname> and : are written without spaces).

Solutions:
(a) To join and leave the chat system
run the ./client command on the terminal and type the username to joint he chat system.
Send exit to go offline.
(b) Tto get the list of online and offline users
use command (2) get_users
(c) To send message to any users
to send to all users use command (4) eg;- broad: hello
to send to a particular user use command (3) eg;- user1: hello
to send to a group of users first create a group by command (5)
eg;- mk_grp:grp1:user1:user2:user3 then type grp1: hello and the message is
delivered to all the users in the group.
(d) Server creating a new process for every client connection and use message queue for
communicating among the child processes.
Server has one global queue storing structure msgform. All messages sent accross the server
are stored on the message queue with the sender name and receiver_id. All the child process
are continuosly polling for their respective clients messages on the queue. If found it fetched
and does a send call to the client.
(e) Chat groups can be created
by using command (5) and communication can be done by any client on the chat group by
command (6).
