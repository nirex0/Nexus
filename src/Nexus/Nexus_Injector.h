#ifndef _Nexus_Injector_h_
#define _Nexus_Injector_h_
class Nexus
{
public:
	static BMP BMPEmbedText(std::string text, BMP bmp);
	static std::string BMPExtractText(BMP bmp);
	static int reverseBits(int n);
};
#endif