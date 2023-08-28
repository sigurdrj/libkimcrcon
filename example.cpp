#include "libkimcrcon.hpp"

int main(){
	libkimcrcon::MCRcon client;

	//            Remote IP   Port    Password
	client.login("127.0.0.1", 25575, "PASSWORD GOES HERE");

	client.send_cmds({"say This is RCON through C++!", "say And here is a second command"});

	std::cout << "Here is a list of players:\n\n";
	std::cout << client.send_cmd("list") << '\n';

	client.disconnect();
}
