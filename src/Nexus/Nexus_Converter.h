#ifndef _Nexus_Converter_h_
#define _Nexus_Converter_h_

class Nexus_Converter
{
public:
	// BMP to PNG
	static std::vector<NDI_BYTE> BMP2PNG(const char* BMPfile);
	
	// PNG to BMP
	static std::vector<NDI_BYTE> PNG2BMP(const char* PNGFile);


private:
	// BMP to PNG
	static unsigned decodeBMP(std::vector<NDI_BYTE>& image, unsigned& w, unsigned& h, const std::vector<NDI_BYTE>& bmp);

	// PNG to BMP
	static void encodeBMP(std::vector<NDI_BYTE>& bmp, const NDI_BYTE* image, int w, int h);

};
#endif


