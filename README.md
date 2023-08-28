# libkimcrcon
Library for interacting with RCON-enabled Minecraft servers

# Example usage (also found in example.cpp)
```cpp
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
```

# Documentation
## std::string libkimcrcon::MCRcon::send\_cmd(const std::string)
Sends a single command to the server, returns the response as a std::string \
If there is any error, it will return an empty string ""

# Notes
This library does not utilise TLS or any encryption whatsoever \
Only use this for controlling a local Minecraft server over a trusted network \

The response parsing does not yet properly validate if the response is \
spec-compliant.

This library was written only based on https://wiki.vg/RCON ([Archive](https://archive.ph/7JHYC))

# TODO
Add verification for command size, response text amount etc
