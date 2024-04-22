**CCHAT**

A simple TCP server - client chatroom written in pure C. Utilizes NCurses to draw a UI for the chat room app.

Contains two programs - server and client. The server must be ran first. Then the clients can connect to the server and chat with other clients.

First run the server: 
`server -p [port number]`

then the clients (up to 10 - arbritrary and changeable)
`client -p [port number] -s [IPV4 Address of the server] -n [display name]`

Currently known bugs: 
When resizing the terminal, NCurses does not properly resize the UI
Probably does not work on IP addresses on different networks, I haven't tested it
