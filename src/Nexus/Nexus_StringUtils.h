#ifndef _Nexus_StringUtils_h_
#define _Nexus_StringUtils_h_
class Nexus_String
{
public:
	template<typename Out>
	static void split(const std::string &s, char delim, Out result);
	static std::vector<std::string> split(const std::string &s, char delim);
	static std::string toStdStr(char* input);
	static bool cstr_equal(char* first, char* second);
};
#endif
