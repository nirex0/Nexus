#ifndef _Nexus_Entropy_h_
#define _Nexus_Entropy_h_
class Entropy
{
public:
	static std::string Nexus_Encrypt(std::string text, std::string key);
	static std::string Nexus_Decrypt(std::string text, std::string key);
};
#endif