#ifndef LIBKIMCRCON_HPP
#define LIBKIMCRCON_HPP

#include <iostream>
#include <string>
#include <vector>
#include <SFML/Network.hpp>

#define GETBITATINDEX(n,idx) (((n)>>(idx))&1)
#define WITHBITSETATINDEX(n,idx,bit) (n | (bit << (31 - idx)))

using std::string;
using std::vector;

namespace libkimcrcon{
	struct Packet{
		private:

		int length; // Bytes, ignoring its own value (minus 4 bytes for the final size)
		int requestID;
		int type;
		vector <string> payload;

		int string_lengths_summed(const vector <string> &vec){
			int ret = 0;

			for (string str : vec)
				ret += str.size();

			return ret;
		}

		public:

		void set_length(const int &newLength){length = newLength;} // Discouraged for users, the length is calculated in set_payload() for normal use
		int get_length(){return length;}

		void set_request_id(const int &newID){requestID = newID;}
		int get_request_id(){return requestID;}

		void set_type(const int &newType){type = newType;}
		int get_type(){return type;}

		void set_payload(const vector <string> &newPayload){
			payload = newPayload;

			// Update the length
			//       length   requestID   Size of payload                  Null terminators in the payload   Last null byte
			length = 4 +      4 +         string_lengths_summed(payload) + payload.size() +                  1;
		}

		vector <string> get_payload(){return payload;}
	};

	string packet_type_to_str(const int packetType){
		if (packetType == 3)
			return "Login";

		if (packetType == 2)
			return "Command";

		if (packetType == 0)
			return "Command response";

		return "UNKNOWN TYPE";
	}

	void print_human_readable_packet(Packet &p){
		std::cout << "Length     | " << p.get_length() << '\n';
		std::cout << "Request ID | " << p.get_request_id() << '\n';
		std::cout << "Type       | " << p.get_type() << " (" << packet_type_to_str(p.get_type()) << ")\n";
		std::cout << "Payload below:\n";

		for (string &cmd : p.get_payload())
			std::cout << cmd << '\n';
	}

	string encode_little_endian_int(const int &n){
		string ret = "";

		unsigned char currByte = 0;

		for (unsigned char i = 0; i < 32; i++){
			currByte |= (GETBITATINDEX(n, i) << (i % 8));

			if (i % 8 == 7){
				ret += currByte;
				currByte = 0;
			}
		}

		return ret;
	}


	int decode_little_endian_int(const string &fourBytes){
		int ret = 0;

		for (unsigned char i = 0; i < 4; i++){
			for (unsigned char j = 0; j < 8; j++)
				ret |= GETBITATINDEX(fourBytes[i], j) << (i * 8 + j);
		}

		return ret;
	}

   	string encode_payload(const vector <string> &vec){
   		string ret = "";

   		for (string str : vec)
   			ret += str + '\0';

   		return ret;
   	}

	class MCRcon{
		private:

		int currID = 0;

		public:

		sf::TcpSocket socket;

		std::pair <bool, Packet> parse_response(string data, const bool useReceivedLength = false){
			Packet ret;

			if (data.size() < 4 + 4 + 4 + 1) // Too small packet, return an error
				return std::make_pair(false, ret);

			if (data[data.size() - 1] != '\0') // Verify there is a null byte at the end of the data
				return std::make_pair(false, ret);

			ret.set_request_id (decode_little_endian_int(data.substr(4, 4)));
			ret.set_type       (decode_little_endian_int(data.substr(8, 4)));

			vector <string> payload = {};

			string buf = "";

			for (unsigned long long i = 4 + 4 + 4; i < data.size() - 1; i++){
				if (data[i] == '\0'){
					payload.push_back(buf);
					buf = "";
				} else{
					buf += data[i];
				}
			}

			ret.set_payload(payload);

			if (useReceivedLength)
				ret.set_length(decode_little_endian_int(data.substr(0, 4)));

			return std::make_pair(true, ret);
		}

		std::pair <bool, Packet> send_cmds(const vector <string> cmds){
			Packet p;
			p.set_request_id(currID);
			p.set_type(2);
			p.set_payload(cmds);

			return send_packet(p);
		}

		string send_cmd(const string cmd){
			std::pair <bool, Packet> response = send_cmds({cmd});

			if (!response.first)
				return "";

			return response.second.get_payload()[0];
		}

		std::pair <bool, Packet> send_packet(Packet &packet){
			string toSend =
				encode_little_endian_int(packet.get_length()) +
				encode_little_endian_int(packet.get_request_id()) +
				encode_little_endian_int(packet.get_type()) +
				encode_payload(packet.get_payload()) +
				'\0'; // Last null byte

			Packet ret;

			if (socket.send(toSend.c_str(), toSend.size()) != sf::Socket::Done)
				return std::make_pair(false, ret);

			char response[4096];
			std::size_t received;

			if (socket.receive(response, 4096, received) != sf::Socket::Done)
				return std::make_pair(false, ret);

			std::pair <bool, Packet> tmp = parse_response(string(response, received));
			if (!tmp.first)
				return std::make_pair(false, ret);

			ret = tmp.second;

			return std::make_pair(true, ret);
		}

		bool login(const sf::IpAddress ipAddress, const unsigned short port = 25575, const string password = ""){
			if (socket.connect(ipAddress, port) != sf::Socket::Done)
				return false;

			Packet p;
			p.set_request_id(currID);
			p.set_type(3); // Type number for login
			p.set_payload({password});

			if (!send_packet(p).first)
				return false;

			return true;
		}

		void disconnect(){
			socket.disconnect();
		}
	};
}

#endif // LIBKIMCRCON_HPP
