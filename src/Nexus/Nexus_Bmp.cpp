#include "Nexus.h"

/* These functions are defined in Nexus_Converter.h */

// BMP to PNG
unsigned Nexus_Converter::decodeBMP(std::vector<NDI_BYTE>& image, unsigned& w, unsigned& h, const std::vector<NDI_BYTE>& bmp)
{
	static const unsigned MINHEADER = 54; //minimum BMP header size

	if (bmp.size() < MINHEADER) return -1;
	if (bmp[0] != 'B' || bmp[1] != 'M') return 1; //It's not a BMP file if it doesn't start with marker 'BM'
	unsigned pixeloffset = bmp[10] + 256 * bmp[11]; //where the pixel data starts
													//read width and height from BMP header
	w = bmp[18] + bmp[19] * 256;
	h = bmp[22] + bmp[23] * 256;
	//read number of channels from BMP header
	if (bmp[28] != 24 && bmp[28] != 32) return 2; //only 24-bit and 32-bit BMPs are supported.
	unsigned numChannels = bmp[28] / 8;

	//The amount of scanline bytes is width of image times channels, with extra bytes added if needed
	//to make it a multiple of 4 bytes.
	unsigned scanlineBytes = w * numChannels;
	if (scanlineBytes % 4 != 0) scanlineBytes = (scanlineBytes / 4) * 4 + 4;

	unsigned dataSize = scanlineBytes * h;
	if (bmp.size() < dataSize + pixeloffset) return 3; //BMP file too small to contain all pixels

	image.resize(w * h * 4);

	/*
	There are 3 differences between BMP and the raw image buffer for LodePNG:
	-it's upside down
	-it's in BGR instead of RGB format (or BRGA instead of RGBA)
	-each scanline has padding bytes to make it a multiple of 4 if needed
	The 2D for loop below does all these 3 conversions at once.
	*/
	for (unsigned y = 0; y < h; y++)
		for (unsigned x = 0; x < w; x++)
		{
			//pixel start byte position in the BMP
			unsigned bmpos = pixeloffset + (h - y - 1) * scanlineBytes + numChannels * x;
			//pixel start byte position in the new raw image
			unsigned newpos = 4 * y * w + 4 * x;
			if (numChannels == 3)
			{
				image[newpos + 0] = bmp[bmpos + 2]; //R
				image[newpos + 1] = bmp[bmpos + 1]; //G
				image[newpos + 2] = bmp[bmpos + 0]; //B
				image[newpos + 3] = 255;            //A
			}
			else
			{
				image[newpos + 0] = bmp[bmpos + 3]; //R
				image[newpos + 1] = bmp[bmpos + 2]; //G
				image[newpos + 2] = bmp[bmpos + 1]; //B
				image[newpos + 3] = bmp[bmpos + 0]; //A
			}
		}
	return 0;
}
std::vector<NDI_BYTE> Nexus_Converter::BMP2PNG(const char* BMPfile)
{
	std::vector<unsigned char> bmp;
	lodepng::load_file(bmp, BMPfile);
	std::vector<unsigned char> image;
	unsigned w, h;
	unsigned error = decodeBMP(image, w, h, bmp);
	std::vector<unsigned char> png;
	error = lodepng::encode(png, image, w, h);
	return png;
}

// PNG to BMP
void Nexus_Converter::encodeBMP(std::vector<NDI_BYTE>& bmp, const NDI_BYTE* image, int w, int h)
{
	//3 bytes per pixel used for both input and output.
	int inputChannels = 3;
	int outputChannels = 3;

	//bytes 0-13
	bmp.push_back('B'); bmp.push_back('M'); //0: bfType
	bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); //2: bfSize; size not yet known for now, filled in later.
	bmp.push_back(0); bmp.push_back(0); //6: bfReserved1
	bmp.push_back(0); bmp.push_back(0); //8: bfReserved2
	bmp.push_back(54 % 256); bmp.push_back(54 / 256); bmp.push_back(0); bmp.push_back(0); //10: bfOffBits (54 header bytes)

																						  //bytes 14-53
	bmp.push_back(40); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //14: biSize
	bmp.push_back(w % 256); bmp.push_back(w / 256); bmp.push_back(0); bmp.push_back(0); //18: biWidth
	bmp.push_back(h % 256); bmp.push_back(h / 256); bmp.push_back(0); bmp.push_back(0); //22: biHeight
	bmp.push_back(1); bmp.push_back(0); //26: biPlanes
	bmp.push_back(outputChannels * 8); bmp.push_back(0); //28: biBitCount
	bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //30: biCompression
	bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //34: biSizeImage
	bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //38: biXPelsPerMeter
	bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //42: biYPelsPerMeter
	bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //46: biClrUsed
	bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //50: biClrImportant

																			 /*
																			 Convert the input RGBRGBRGB pixel buffer to the BMP pixel buffer format. There are 3 differences with the input buffer:
																			 -BMP stores the rows inversed, from bottom to top
																			 -BMP stores the color channels in BGR instead of RGB order
																			 -BMP requires each row to have a multiple of 4 bytes, so sometimes padding bytes are added between rows
																			 */

	int imagerowbytes = outputChannels * w;
	imagerowbytes = imagerowbytes % 4 == 0 ? imagerowbytes : imagerowbytes + (4 - imagerowbytes % 4); //must be multiple of 4

	for (int y = h - 1; y >= 0; y--) //the rows are stored inversed in bmp
	{
		int c = 0;
		for (int x = 0; x < imagerowbytes; x++)
		{
			if (x < w * outputChannels)
			{
				int inc = c;
				//Convert RGB(A) into BGR(A)
				if (c == 0) inc = 2;
				else if (c == 2) inc = 0;
				bmp.push_back(image[inputChannels * (w * y + x / outputChannels) + inc]);
			}
			else bmp.push_back(0);
			c++;
			if (c >= outputChannels) c = 0;
		}
	}

	// Fill in the size
	bmp[2] = bmp.size() % 256;
	bmp[3] = (bmp.size() / 256) % 256;
	bmp[4] = (bmp.size() / 65536) % 256;
	bmp[5] = bmp.size() / 16777216;
}
std::vector<NDI_BYTE> Nexus_Converter::PNG2BMP(const char* PNGFile)
{
	const char* infile = PNGFile;

	std::vector<unsigned char> image; //the raw pixels
	unsigned width, height;

	unsigned error = lodepng::decode(image, width, height, infile, LCT_RGB, 8);

	std::vector<NDI_BYTE> bmp;
	encodeBMP(bmp, &image[0], width, height);
	return bmp;
}

/* These functions are defined in Nexus_StringUtils.h */

template<typename Out>
void Nexus_String::split(const std::string &s, char delim, Out result)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		*(result++) = item;
	}
}

std::vector<std::string> Nexus_String::split(const std::string &s, char delim) 
{
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

std::string Nexus_String::toStdStr(char* input)
{
	std::string outStr = "";
	size_t count = 0;
	while (input[count])
	{
		outStr += input[count];
		count++;
	}
	return outStr;
}

bool Nexus_String::cstr_equal(char* first, char* second)
{
	size_t flen = strlen(first);
	size_t slen = strlen(second);
	if (flen != slen)
	{
		return false;
	}
	if (flen == slen)
	{
		size_t x = flen;
		for (size_t i = 0; i < x; i++)
		{
			if (first[i] != second[i])
			{
				return false;
			}
		}
		return true;
	}
}

/* These functions are defined in Nexus_Entropy.h */

std::string Entropy::Nexus_Encrypt(std::string text, std::string key)
{
	int textLen = text.length();
	int keyLen = key.length();
	std::vector<char> vec;
	for (int i = 0; i < textLen; i++)
	{
		char l;
		for (int j = 0; j < keyLen; j++)
		{
			l = static_cast<char>(text[i] + (2 * key[j] * pow(key[j], 2) + (8 * key[j])));
		}
		vec.push_back(l);
	}
	std::string returnStr = "";
	int size = static_cast<int>(vec.size());
	for (int i = 0; i < size; i++)
	{
		returnStr += vec[i];
	}
	return returnStr;
}

std::string Entropy::Nexus_Decrypt(std::string text, std::string key)
{
	int textLen = text.length();
	int keyLen = key.length();
	std::vector<char> vec;
	for (int i = 0; i < textLen; i++)
	{
		char l;
		for (int j = 0; j < keyLen; j++)
		{
			l = static_cast<char>(text[i] - (2 * key[j] * pow(key[j], 2) + (8 * key[j])));
		}
		vec.push_back(l);
	}
	std::string returnStr = "";
	int size = static_cast<int>(vec.size());
	for (int i = 0; i < size; i++)
	{
		returnStr += vec[i];
	}
	return returnStr;
}

/* These functions are defined in Nexus_Injector.h */

BMP Nexus::BMPEmbedText(std::string text, BMP bmp)
{

	int textLength = text.length();

	// initially, we'll be hiding characters in the image
	State state = Hiding;

	// holds the index of the character that is being hidden
	int charIndex = 0;

	// holds the value of the character converted to integer
	int charValue = 0;

	// holds the index of the color element (R or G or B) that is currently being processed
	long pixelElementIndex = 0;

	// holds the number of trailing zeros that have been added when finishing the process
	int zeros = 0;

	// hold pixel elements
	int R = 0, G = 0, B = 0;

	// pass through the rows
	for (int i = 0; i < bmp.GetHeight(); i++)
	{
		// pass through each row
		for (int j = 0; j < bmp.GetWidth(); j++)
		{
			// holds the pixel that is currently being processed
			Pixel pixel = bmp.GetPixel(j, i);

			// now, clear the least significant bit (LSB) from each pixel element
			R = pixel.Red - pixel.Red % 2;
			G = pixel.Green - pixel.Green % 2;
			B = pixel.Blue - pixel.Blue % 2;

			// for each pixel, pass through its elements (RGB)
			for (int n = 0; n < 3; n++)
			{
				// check if new 8 bits has been processed
				if (pixelElementIndex % 8 == 0)
				{
					// check if the whole process has finished
					// we can say that it's finished when 8 zeros are added
					if (state == Filling_With_Zeros && zeros == 8)
					{
						// apply the last pixel on the image
						// even if only a part of its elements have been affected
						if ((pixelElementIndex - 1) % 3 < 2)
						{
							Pixel pix;
							pix.Red = R;
							pix.Green = G;
							pix.Blue = B;
							bmp.SetPixel(j, i, pix);
						}

						// return the bitmap with the text hidden in
						return bmp;
					}

					// check if all characters has been hidden
					if (charIndex >= textLength)
					{
						// start adding zeros to mark the end of the text
						state = Filling_With_Zeros;
					}
					else
					{
						// move to the next character and process again
						charValue = text[charIndex++];
					}
				}

				// check which pixel element has the turn to hide a bit in its LSB
				switch (pixelElementIndex % 3)
				{
				case 0:
				{
					if (state == Hiding)
					{
						// the rightmost bit in the character will be (charValue % 2)
						// to put this value instead of the LSB of the pixel element
						// just add it to it
						// recall that the LSB of the pixel element had been cleared
						// before this operation
						R += charValue % 2;

						// removes the added rightmost bit of the character
						// such that next time we can reach the next one
						charValue /= 2;
					}
				} break;
				case 1:
				{
					if (state == Hiding)
					{
						G += charValue % 2;

						charValue /= 2;
					}
				} break;
				case 2:
				{
					if (state == Hiding)
					{
						B += charValue % 2;

						charValue /= 2;
					}
					Pixel pix;
					pix.Red = R;
					pix.Green = G;
					pix.Blue = B;
					bmp.SetPixel(j, i, pix);
				} break;
				}

				pixelElementIndex++;

				if (state == Filling_With_Zeros)
				{
					// increment the value of zeros until it is 8
					zeros++;
				}
			}
		}
	}

	return bmp;
}

std::string Nexus::BMPExtractText(BMP bmp)
{
	int colorUnitIndex = 0;
	int charValue = 0;

	// holds the text that will be extracted from the image
	std::string extractedText = "";

	// pass through the rows
	for (int i = 0; i < bmp.GetHeight(); i++)
	{
		// pass through each row
		for (int j = 0; j < bmp.GetWidth(); j++)
		{
			Pixel pixel = bmp.GetPixel(j, i);

			// for each pixel, pass through its elements (RGB)
			for (int n = 0; n < 3; n++)
			{
				switch (colorUnitIndex % 3)
				{
				case 0:
				{
					// get the LSB from the pixel element (will be pixel.R % 2)
					// then add one bit to the right of the current character
					// this can be done by (charValue = charValue * 2)
					// replace the added bit (which value is by default 0) with
					// the LSB of the pixel element, simply by addition
					charValue = charValue * 2 + pixel.Red % 2;
				} break;
				case 1:
				{
					charValue = charValue * 2 + pixel.Green % 2;
				} break;
				case 2:
				{
					charValue = charValue * 2 + pixel.Blue % 2;
				} break;
				}

				colorUnitIndex++;

				// if 8 bits has been added,
				// then add the current character to the result text
				if (colorUnitIndex % 8 == 0)
				{
					// reverse? of course, since each time the process occurs
					// on the right (for simplicity)
					charValue = reverseBits(charValue);

					// can only be 0 if it is the stop character (the 8 zeros)
					if (charValue == 0)
					{
						return extractedText;
					}

					// convert the character value from int to char
					char c = (char)charValue;

					// add the current character to the result text
					extractedText += c;
				}
			}
		}
	}

	return extractedText;
}

int Nexus::reverseBits(int n)
{
	int result = 0;

	for (int i = 0; i < 8; i++)
	{
		result = result * 2 + n % 2;
		n /= 2;
	}

	return result;
}

/* These functions are defined in Nexus.h */

bool NexusWarnings = true;

void SetNexusWarningsOff(void)
{
	NexusWarnings = false;
}
void SetNexusWarningsOn(void)
{
	NexusWarnings = true;
}
bool GetNexusWarningstate(void)
{
	return NexusWarnings;
}

/* These functions are defined in Nexus_DataStructures.h */

int IntPow(int base, int exponent)
{
	int i;
	int output = 1;
	for (i = 0; i < exponent; i++)
	{
		output *= base;
	}
	return output;
}

BMFH::BMFH()
{
	bfType = 19778;
	bfReserved1 = 0;
	bfReserved2 = 0;
}

void BMFH::SwitchEndianess(void)
{
	bfType = FlipWORD(bfType);
	bfSize = FlipDWORD(bfSize);
	bfReserved1 = FlipWORD(bfReserved1);
	bfReserved2 = FlipWORD(bfReserved2);
	bfOffBits = FlipDWORD(bfOffBits);
	return;
}

BMIH::BMIH()
{
	biPlanes = 1;
	biCompression = 0;
	biXPelsPerMeter = DefaultXPelsPerMeter;
	biYPelsPerMeter = DefaultYPelsPerMeter;
	biClrUsed = 0;
	biClrImportant = 0;
}

void BMIH::SwitchEndianess(void)
{
	biSize = FlipDWORD(biSize);
	biWidth = FlipDWORD(biWidth);
	biHeight = FlipDWORD(biHeight);
	biPlanes = FlipWORD(biPlanes);
	biBitCount = FlipWORD(biBitCount);
	biCompression = FlipDWORD(biCompression);
	biSizeImage = FlipDWORD(biSizeImage);
	biXPelsPerMeter = FlipDWORD(biXPelsPerMeter);
	biYPelsPerMeter = FlipDWORD(biYPelsPerMeter);
	biClrUsed = FlipDWORD(biClrUsed);
	biClrImportant = FlipDWORD(biClrImportant);
	return;
}

void BMIH::display(void)
{
	using namespace std;
	cout << "biSize: " << (int)biSize << endl
		<< "biWidth: " << (int)biWidth << endl
		<< "biHeight: " << (int)biHeight << endl
		<< "biPlanes: " << (int)biPlanes << endl
		<< "biBitCount: " << (int)biBitCount << endl
		<< "biCompression: " << (int)biCompression << endl
		<< "biSizeImage: " << (int)biSizeImage << endl
		<< "biXPelsPerMeter: " << (int)biXPelsPerMeter << endl
		<< "biYPelsPerMeter: " << (int)biYPelsPerMeter << endl
		<< "biClrUsed: " << (int)biClrUsed << endl
		<< "biClrImportant: " << (int)biClrImportant << endl << endl;
}

void BMFH::display(void)
{
	using namespace std;
	cout << "bfType: " << (int)bfType << endl
		<< "bfSize: " << (int)bfSize << endl
		<< "bfReserved1: " << (int)bfReserved1 << endl
		<< "bfReserved2: " << (int)bfReserved2 << endl
		<< "bfOffBits: " << (int)bfOffBits << endl << endl;
}

/* These functions are defined in Nexus_Bitmap.h */

Pixel BMP::GetPixel(int i, int j) const
{
	using namespace std;
	bool Warn = false;
	if (i >= Width)
	{
		i = Width - 1; Warn = true;
	}
	if (i < 0)
	{
		i = 0; Warn = true;
	}
	if (j >= Height)
	{
		j = Height - 1; Warn = true;
	}
	if (j < 0)
	{
		j = 0; Warn = true;
	}
	if (Warn && NexusWarnings)
	{
		cout << "Nexus Warning: Attempted to access non-existent pixel;" << endl
			<< "               Truncating request to fit in the range [0,"
			<< Width - 1 << "] x [0," << Height - 1 << "]." << endl;
	}
	return Pixels[i][j];
}

bool BMP::SetPixel(int i, int j, Pixel NewPixel)
{
	Pixels[i][j] = NewPixel;
	return true;
}


bool BMP::SetColor(int ColorNumber, Pixel NewColor)
{
	using namespace std;
	if (BitDepth != 1 && BitDepth != 4 && BitDepth != 8)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Attempted to change color table for a BMP object" << endl
				<< "               that lacks a color table. Ignoring request." << endl;
		}
		return false;
	}
	if (!Colors)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Attempted to set a color, but the color table" << endl
				<< "               is not defined. Ignoring request." << endl;
		}
		return false;
	}
	if (ColorNumber >= GetNumberOfColors())
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Requested color number "
				<< ColorNumber << " is outside the allowed" << endl
				<< "               range [0," << GetNumberOfColors() - 1
				<< "]. Ignoring request to set this color." << endl;
		}
		return false;
	}
	Colors[ColorNumber] = NewColor;
	return true;
}

// Pixel BMP::GetColor( int ColorNumber ) const
Pixel BMP::GetColor(int ColorNumber)
{
	Pixel Output;
	Output.Red = 255;
	Output.Green = 255;
	Output.Blue = 255;
	Output.Alpha = 0;

	using namespace std;
	if (BitDepth != 1 && BitDepth != 4 && BitDepth != 8)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Attempted to access color table for a BMP object" << endl
				<< "               that lacks a color table. Ignoring request." << endl;
		}
		return Output;
	}
	if (!Colors)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Requested a color, but the color table" << endl
				<< "               is not defined. Ignoring request." << endl;
		}
		return Output;
	}
	if (ColorNumber >= GetNumberOfColors())
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Requested color number "
				<< ColorNumber << " is outside the allowed" << endl
				<< "               range [0," << GetNumberOfColors() - 1
				<< "]. Ignoring request to get this color." << endl;
		}
		return Output;
	}
	Output = Colors[ColorNumber];
	return Output;
}

BMP::BMP()
{
	Width = 1;
	Height = 1;
	BitDepth = 24;
	Pixels = new Pixel*[Width];
	Pixels[0] = new Pixel[Height];
	Colors = NULL;

	XPelsPerMeter = 0;
	YPelsPerMeter = 0;

	MetaData1 = NULL;
	SizeOfMetaData1 = 0;
	MetaData2 = NULL;
	SizeOfMetaData2 = 0;
}

// BMP::BMP( const BMP& Input )
BMP::BMP(BMP& Input)
{
	// first, make the image empty.

	Width = 1;
	Height = 1;
	BitDepth = 24;
	Pixels = new Pixel*[Width];
	Pixels[0] = new Pixel[Height];
	Colors = NULL;
	XPelsPerMeter = 0;
	YPelsPerMeter = 0;

	MetaData1 = NULL;
	SizeOfMetaData1 = 0;
	MetaData2 = NULL;
	SizeOfMetaData2 = 0;

	// now, set the correct bit depth

	SetBitDepth(Input.GetBitDepth());

	// set the correct pixel size 

	SetSize(Input.GetWidth(), Input.GetHeight());

	// set the DPI information from Input

	SetDPI(Input.GetHorizontalDPI(), Input.GetVerticalDPI());

	// if there is a color table, get all the colors

	if (BitDepth == 1 || BitDepth == 4 ||
		BitDepth == 8)
	{
		for (int k = 0; k < GetNumberOfColors(); k++)
		{
			SetColor(k, Input.GetColor(k));
		}
	}

	// get all the pixels 

	for (int j = 0; j < Height; j++)
	{
		for (int i = 0; i < Width; i++)
		{
			Pixels[i][j] = *Input(i, j);
			//   Pixels[i][j] = Input.GetPixel(i,j); // *Input(i,j);
		}
	}
}

BMP::~BMP()
{
	int i;
	for (i = 0;i < Width;i++)
	{
		delete[] Pixels[i];
	}
	delete[] Pixels;
	if (Colors)
	{
		delete[] Colors;
	}

	if (MetaData1)
	{
		delete[] MetaData1;
	}
	if (MetaData2)
	{
		delete[] MetaData2;
	}
}

Pixel* BMP::operator()(int i, int j)
{
	using namespace std;
	bool Warn = false;
	if (i >= Width)
	{
		i = Width - 1; Warn = true;
	}
	if (i < 0)
	{
		i = 0; Warn = true;
	}
	if (j >= Height)
	{
		j = Height - 1; Warn = true;
	}
	if (j < 0)
	{
		j = 0; Warn = true;
	}
	if (Warn && NexusWarnings)
	{
		cout << "Nexus Warning: Attempted to access non-existent pixel;" << endl
			<< "               Truncating request to fit in the range [0,"
			<< Width - 1 << "] x [0," << Height - 1 << "]." << endl;
	}
	return &(Pixels[i][j]);
}

// int BMP::GetBitDepth( void ) const
int BMP::GetBitDepth(void)
{
	return BitDepth;
}

// int BMP::GetHeight( void ) const
int BMP::GetHeight(void)
{
	return Height;
}

// int BMP::GetWidth( void ) const
int BMP::GetWidth(void)
{
	return Width;
}

// int BMP::GetNumberOfColors( void ) const
int BMP::GetNumberOfColors(void)
{
	int output = IntPow(2, BitDepth);
	if (BitDepth == 32)
	{
		output = IntPow(2, 24);
	}
	return output;
}

bool BMP::SetBitDepth(int NewDepth)
{
	using namespace std;
	if (NewDepth != 1 && NewDepth != 4 &&
		NewDepth != 8 && NewDepth != 16 &&
		NewDepth != 24 && NewDepth != 32)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: User attempted to set unsupported bit depth "
				<< NewDepth << "." << endl
				<< "               Bit depth remains unchanged at "
				<< BitDepth << "." << endl;
		}
		return false;
	}

	BitDepth = NewDepth;
	if (Colors)
	{
		delete[] Colors;
	}
	int NumberOfColors = IntPow(2, BitDepth);
	if (BitDepth == 1 || BitDepth == 4 || BitDepth == 8)
	{
		Colors = new Pixel[NumberOfColors];
	}
	else
	{
		Colors = NULL;
	}
	if (BitDepth == 1 || BitDepth == 4 || BitDepth == 8)
	{
		CreateStandardColorTable();
	}

	return true;
}

bool BMP::SetSize(int NewWidth, int NewHeight)
{
	using namespace std;
	if (NewWidth <= 0 || NewHeight <= 0)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: User attempted to set a non-positive width or height." << endl
				<< "               Size remains unchanged at "
				<< Width << " x " << Height << "." << endl;
		}
		return false;
	}

	int i, j;

	for (i = 0;i < Width;i++)
	{
		delete[] Pixels[i];
	}
	delete[] Pixels;

	Width = NewWidth;
	Height = NewHeight;
	Pixels = new Pixel*[Width];

	for (i = 0; i < Width; i++)
	{
		Pixels[i] = new Pixel[Height];
	}

	for (i = 0; i < Width; i++)
	{
		for (j = 0; j < Height; j++)
		{
			Pixels[i][j].Red = 255;
			Pixels[i][j].Green = 255;
			Pixels[i][j].Blue = 255;
			Pixels[i][j].Alpha = 0;
		}
	}

	return true;
}

bool BMP::WriteToFile(const char* FileName)
{
	using namespace std;
	if (!NexusCheckDataSize())
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: Data types are wrong size!" << endl
				<< "              You may need to mess with Nexus_DataTypes.h" << endl
				<< "              to fix these errors, and then recompile." << endl
				<< "              All 32-bit and 64-bit machines should be" << endl
				<< "              supported, however." << endl << endl;
		}
		return false;
	}

	FILE* fp = fopen(FileName, "wb");
	if (fp == NULL)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: Cannot open file "
				<< FileName << " for output." << endl;
		}
		fclose(fp);
		return false;
	}

	// some preliminaries

	double dBytesPerPixel = ((double)BitDepth) / 8.0;
	double dBytesPerRow = dBytesPerPixel * (Width + 0.0);
	dBytesPerRow = ceil(dBytesPerRow);

	int BytePaddingPerRow = 4 - ((int)(dBytesPerRow)) % 4;
	if (BytePaddingPerRow == 4)
	{
		BytePaddingPerRow = 0;
	}

	double dActualBytesPerRow = dBytesPerRow + BytePaddingPerRow;

	double dTotalPixelBytes = Height * dActualBytesPerRow;

	double dPaletteSize = 0;
	if (BitDepth == 1 || BitDepth == 4 || BitDepth == 8)
	{
		dPaletteSize = IntPow(2, BitDepth)*4.0;
	}

	// leave some room for 16-bit masks 
	if (BitDepth == 16)
	{
		dPaletteSize = 3 * 4;
	}

	double dTotalFileSize = 14 + 40 + dPaletteSize + dTotalPixelBytes;

	// write the file header 

	BMFH bmfh;
	bmfh.bfSize = (NDI_DWORD)dTotalFileSize;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = (NDI_DWORD)(14 + 40 + dPaletteSize);

	if (IsBigEndian())
	{
		bmfh.SwitchEndianess();
	}

	fwrite((char*) &(bmfh.bfType), sizeof(NDI_WORD), 1, fp);
	fwrite((char*) &(bmfh.bfSize), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmfh.bfReserved1), sizeof(NDI_WORD), 1, fp);
	fwrite((char*) &(bmfh.bfReserved2), sizeof(NDI_WORD), 1, fp);
	fwrite((char*) &(bmfh.bfOffBits), sizeof(NDI_DWORD), 1, fp);

	// write the info header 

	BMIH bmih;
	bmih.biSize = 40;
	bmih.biWidth = Width;
	bmih.biHeight = Height;
	bmih.biPlanes = 1;
	bmih.biBitCount = BitDepth;
	bmih.biCompression = 0;
	bmih.biSizeImage = (NDI_DWORD)dTotalPixelBytes;
	if (XPelsPerMeter)
	{
		bmih.biXPelsPerMeter = XPelsPerMeter;
	}
	else
	{
		bmih.biXPelsPerMeter = DefaultXPelsPerMeter;
	}
	if (YPelsPerMeter)
	{
		bmih.biYPelsPerMeter = YPelsPerMeter;
	}
	else
	{
		bmih.biYPelsPerMeter = DefaultYPelsPerMeter;
	}

	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	// indicates that we'll be using bit fields for 16-bit files
	if (BitDepth == 16)
	{
		bmih.biCompression = 3;
	}

	if (IsBigEndian())
	{
		bmih.SwitchEndianess();
	}

	fwrite((char*) &(bmih.biSize), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biWidth), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biHeight), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biPlanes), sizeof(NDI_WORD), 1, fp);
	fwrite((char*) &(bmih.biBitCount), sizeof(NDI_WORD), 1, fp);
	fwrite((char*) &(bmih.biCompression), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biSizeImage), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biXPelsPerMeter), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biYPelsPerMeter), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biClrUsed), sizeof(NDI_DWORD), 1, fp);
	fwrite((char*) &(bmih.biClrImportant), sizeof(NDI_DWORD), 1, fp);

	// write the palette 
	if (BitDepth == 1 || BitDepth == 4 || BitDepth == 8)
	{
		int NumberOfColors = IntPow(2, BitDepth);

		// if there is no palette, create one 
		if (!Colors)
		{
			if (!Colors)
			{
				Colors = new Pixel[NumberOfColors];
			}
			CreateStandardColorTable();
		}

		int n;
		for (n = 0; n < NumberOfColors; n++)
		{
			fwrite((char*) &(Colors[n]), 4, 1, fp);
		}
	}

	// write the pixels 
	int i, j;
	if (BitDepth != 16)
	{
		NDI_BYTE* Buffer;
		int BufferSize = (int)((Width*BitDepth) / 8.0);
		while (8 * BufferSize < Width*BitDepth)
		{
			BufferSize++;
		}
		while (BufferSize % 4)
		{
			BufferSize++;
		}

		Buffer = new NDI_BYTE[BufferSize];
		for (j = 0; j < BufferSize; j++)
		{
			Buffer[j] = 0;
		}

		j = Height - 1;

		while (j > -1)
		{
			bool Success = false;
			if (BitDepth == 32)
			{
				Success = Write32bitRow(Buffer, BufferSize, j);
			}
			if (BitDepth == 24)
			{
				Success = Write24bitRow(Buffer, BufferSize, j);
			}
			if (BitDepth == 8)
			{
				Success = Write8bitRow(Buffer, BufferSize, j);
			}
			if (BitDepth == 4)
			{
				Success = Write4bitRow(Buffer, BufferSize, j);
			}
			if (BitDepth == 1)
			{
				Success = Write1bitRow(Buffer, BufferSize, j);
			}
			if (Success)
			{
				int BytesWritten = (int)fwrite((char*)Buffer, 1, BufferSize, fp);
				if (BytesWritten != BufferSize)
				{
					Success = false;
				}
			}
			if (!Success)
			{
				if (NexusWarnings)
				{
					cout << "Nexus Error: Could not write proper amount of data." << endl;
				}
				j = -1;
			}
			j--;
		}

		delete[] Buffer;
	}

	if (BitDepth == 16)
	{
		// write the bit masks

		NDI_WORD BlueMask = 31;    // bits 12-16
		NDI_WORD GreenMask = 2016; // bits 6-11
		NDI_WORD RedMask = 63488;  // bits 1-5
		NDI_WORD ZeroWORD;

		if (IsBigEndian())
		{
			RedMask = FlipWORD(RedMask);
		}
		fwrite((char*)&RedMask, 2, 1, fp);
		fwrite((char*)&ZeroWORD, 2, 1, fp);

		if (IsBigEndian())
		{
			GreenMask = FlipWORD(GreenMask);
		}
		fwrite((char*)&GreenMask, 2, 1, fp);
		fwrite((char*)&ZeroWORD, 2, 1, fp);

		if (IsBigEndian())
		{
			BlueMask = FlipWORD(BlueMask);
		}
		fwrite((char*)&BlueMask, 2, 1, fp);
		fwrite((char*)&ZeroWORD, 2, 1, fp);

		int DataBytes = Width * 2;
		int PaddingBytes = (4 - DataBytes % 4) % 4;

		// write the actual pixels

		for (j = Height - 1; j >= 0; j--)
		{
			// write all row pixel data
			i = 0;
			int WriteNumber = 0;
			while (WriteNumber < DataBytes)
			{
				NDI_WORD TempWORD;

				NDI_WORD RedWORD = (NDI_WORD)((Pixels[i][j]).Red / 8);
				NDI_WORD GreenWORD = (NDI_WORD)((Pixels[i][j]).Green / 4);
				NDI_WORD BlueWORD = (NDI_WORD)((Pixels[i][j]).Blue / 8);

				TempWORD = (RedWORD << 11) + (GreenWORD << 5) + BlueWORD;
				if (IsBigEndian())
				{
					TempWORD = FlipWORD(TempWORD);
				}

				fwrite((char*)&TempWORD, 2, 1, fp);
				WriteNumber += 2;
				i++;
			}
			// write any necessary row padding
			WriteNumber = 0;
			while (WriteNumber < PaddingBytes)
			{
				NDI_BYTE TempBYTE;
				fwrite((char*)&TempBYTE, 1, 1, fp);
				WriteNumber++;
			}
		}

	}

	fclose(fp);
	return true;
}

bool BMP::ReadFromFile(const char* FileName)
{
	using namespace std;
	if (!NexusCheckDataSize())
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: Data types are wrong size!" << endl
				<< "              You may need to mess with DataTypes.h" << endl
				<< "              to fix these errors, and then recompile." << endl
				<< "              All 32-bit and 64-bit machines should be" << endl
				<< "              supported, however." << endl << endl;
		}
		return false;
	}

	FILE* fp = fopen(FileName, "rb");
	if (fp == NULL)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: Cannot open file "
				<< FileName << " for input." << endl;
		}
		SetBitDepth(1);
		SetSize(1, 1);
		return false;
	}

	// read the file header 

	BMFH bmfh;
	bool NotCorrupted = true;

	NotCorrupted &= SafeFread((char*) &(bmfh.bfType), sizeof(NDI_WORD), 1, fp);

	bool IsBitmap = false;

	if (IsBigEndian() && bmfh.bfType == 16973)
	{
		IsBitmap = true;
	}
	if (!IsBigEndian() && bmfh.bfType == 19778)
	{
		IsBitmap = true;
	}

	if (!IsBitmap)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: " << FileName
				<< " is not a Windows BMP file!" << endl;
		}
		fclose(fp);
		return false;
	}

	NotCorrupted &= SafeFread((char*) &(bmfh.bfSize), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmfh.bfReserved1), sizeof(NDI_WORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmfh.bfReserved2), sizeof(NDI_WORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmfh.bfOffBits), sizeof(NDI_DWORD), 1, fp);

	if (IsBigEndian())
	{
		bmfh.SwitchEndianess();
	}

	// read the info header

	BMIH bmih;

	NotCorrupted &= SafeFread((char*) &(bmih.biSize), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biWidth), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biHeight), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biPlanes), sizeof(NDI_WORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biBitCount), sizeof(NDI_WORD), 1, fp);

	NotCorrupted &= SafeFread((char*) &(bmih.biCompression), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biSizeImage), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biXPelsPerMeter), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biYPelsPerMeter), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biClrUsed), sizeof(NDI_DWORD), 1, fp);
	NotCorrupted &= SafeFread((char*) &(bmih.biClrImportant), sizeof(NDI_DWORD), 1, fp);

	if (IsBigEndian())
	{
		bmih.SwitchEndianess();
	}

	// a safety catch: if any of the header information didn't read properly, abort
	// future idea: check to see if at least most is self-consistent

	if (!NotCorrupted)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: " << FileName
				<< " is obviously corrupted." << endl;
		}
		SetSize(1, 1);
		SetBitDepth(1);
		fclose(fp);
		return false;
	}

	XPelsPerMeter = bmih.biXPelsPerMeter;
	YPelsPerMeter = bmih.biYPelsPerMeter;

	// if bmih.biCompression 1 or 2, then the file is RLE compressed

	if (bmih.biCompression == 1 || bmih.biCompression == 2)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: " << FileName << " is (RLE) compressed." << endl
				<< "              Nexus does not support compression." << endl;
		}
		SetSize(1, 1);
		SetBitDepth(1);
		fclose(fp);
		return false;
	}

	// if bmih.biCompression > 3, then something strange is going on 
	// it's probably an OS2 bitmap file.

	if (bmih.biCompression > 3)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: " << FileName << " is in an unsupported format."
				<< endl
				<< "              (bmih.biCompression = "
				<< bmih.biCompression << ")" << endl
				<< "              The file is probably an old OS2 bitmap or corrupted."
				<< endl;
		}
		SetSize(1, 1);
		SetBitDepth(1);
		fclose(fp);
		return false;
	}

	if (bmih.biCompression == 3 && bmih.biBitCount != 16)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: " << FileName
				<< " uses bit fields and is not a" << endl
				<< "              16-bit file. This is not supported." << endl;
		}
		SetSize(1, 1);
		SetBitDepth(1);
		fclose(fp);
		return false;
	}

	// set the bit depth

	int TempBitDepth = (int)bmih.biBitCount;
	if (TempBitDepth != 1 && TempBitDepth != 4
		&& TempBitDepth != 8 && TempBitDepth != 16
		&& TempBitDepth != 24 && TempBitDepth != 32)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: " << FileName << " has unrecognized bit depth." << endl;
		}
		SetSize(1, 1);
		SetBitDepth(1);
		fclose(fp);
		return false;
	}
	SetBitDepth((int)bmih.biBitCount);

	// set the size

	if ((int)bmih.biWidth <= 0 || (int)bmih.biHeight <= 0)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: " << FileName
				<< " has a non-positive width or height." << endl;
		}
		SetSize(1, 1);
		SetBitDepth(1);
		fclose(fp);
		return false;
	}
	SetSize((int)bmih.biWidth, (int)bmih.biHeight);

	// some preliminaries

	double dBytesPerPixel = ((double)BitDepth) / 8.0;
	double dBytesPerRow = dBytesPerPixel * (Width + 0.0);
	dBytesPerRow = ceil(dBytesPerRow);

	int BytePaddingPerRow = 4 - ((int)(dBytesPerRow)) % 4;
	if (BytePaddingPerRow == 4)
	{
		BytePaddingPerRow = 0;
	}

	// if < 16 bits, read the palette

	if (BitDepth < 16)
	{
		// determine the number of colors specified in the 
		// color table

		int NumberOfColorsToRead = ((int)bmfh.bfOffBits - 54) / 4;
		if (NumberOfColorsToRead > IntPow(2, BitDepth))
		{
			NumberOfColorsToRead = IntPow(2, BitDepth);
		}

		if (NumberOfColorsToRead < GetNumberOfColors())
		{
			if (NexusWarnings)
			{
				cout << "Nexus Warning: file " << FileName << " has an underspecified" << endl
					<< "               color table. The table will be padded with extra" << endl
					<< "               white (255,255,255,0) entries." << endl;
			}
		}

		int n;
		for (n = 0; n < NumberOfColorsToRead; n++)
		{
			SafeFread((char*) &(Colors[n]), 4, 1, fp);
		}
		for (n = NumberOfColorsToRead; n < GetNumberOfColors(); n++)
		{
			Pixel WHITE;
			WHITE.Red = 255;
			WHITE.Green = 255;
			WHITE.Blue = 255;
			WHITE.Alpha = 0;
			SetColor(n, WHITE);
		}


	}

	// skip blank data if bfOffBits so indicates

	int BytesToSkip = bmfh.bfOffBits - 54;;
	if (BitDepth < 16)
	{
		BytesToSkip -= 4 * IntPow(2, BitDepth);
	}
	if (BitDepth == 16 && bmih.biCompression == 3)
	{
		BytesToSkip -= 3 * 4;
	}
	if (BytesToSkip < 0)
	{
		BytesToSkip = 0;
	}
	if (BytesToSkip > 0 && BitDepth != 16)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Extra meta data detected in file " << FileName << endl
				<< "               Data will be skipped." << endl;
		}
		NDI_BYTE* TempSkipBYTE;
		TempSkipBYTE = new NDI_BYTE[BytesToSkip];
		SafeFread((char*)TempSkipBYTE, BytesToSkip, 1, fp);
		delete[] TempSkipBYTE;
	}

	// This code reads 1, 4, 8, 24, and 32-bpp files 
	// with a more-efficient buffered technique.

	int i, j;
	if (BitDepth != 16)
	{
		int BufferSize = (int)((Width*BitDepth) / 8.0);
		while (8 * BufferSize < Width*BitDepth)
		{
			BufferSize++;
		}
		while (BufferSize % 4)
		{
			BufferSize++;
		}
		NDI_BYTE* Buffer;
		Buffer = new NDI_BYTE[BufferSize];
		j = Height - 1;
		while (j > -1)
		{
			int BytesRead = (int)fread((char*)Buffer, 1, BufferSize, fp);
			if (BytesRead < BufferSize)
			{
				j = -1;
				if (NexusWarnings)
				{
					cout << "Nexus Error: Could not read proper amount of data." << endl;
				}
			}
			else
			{
				bool Success = false;
				if (BitDepth == 1)
				{
					Success = Read1bitRow(Buffer, BufferSize, j);
				}
				if (BitDepth == 4)
				{
					Success = Read4bitRow(Buffer, BufferSize, j);
				}
				if (BitDepth == 8)
				{
					Success = Read8bitRow(Buffer, BufferSize, j);
				}
				if (BitDepth == 24)
				{
					Success = Read24bitRow(Buffer, BufferSize, j);
				}
				if (BitDepth == 32)
				{
					Success = Read32bitRow(Buffer, BufferSize, j);
				}
				if (!Success)
				{
					if (NexusWarnings)
					{
						cout << "Nexus Error: Could not read enough pixel data!" << endl;
					}
					j = -1;
				}
			}
			j--;
		}
		delete[] Buffer;
	}

	if (BitDepth == 16)
	{
		int DataBytes = Width * 2;
		int PaddingBytes = (4 - DataBytes % 4) % 4;

		// set the default mask

		NDI_WORD BlueMask = 31; // bits 12-16
		NDI_WORD GreenMask = 992; // bits 7-11
		NDI_WORD RedMask = 31744; // bits 2-6

		// read the bit fields, if necessary, to 
		// override the default 5-5-5 mask

		if (bmih.biCompression != 0)
		{
			// read the three bit masks

			NDI_WORD TempMaskWORD;

			SafeFread((char*)&RedMask, 2, 1, fp);
			if (IsBigEndian())
			{
				RedMask = FlipWORD(RedMask);
			}
			SafeFread((char*)&TempMaskWORD, 2, 1, fp);

			SafeFread((char*)&GreenMask, 2, 1, fp);
			if (IsBigEndian())
			{
				GreenMask = FlipWORD(GreenMask);
			}
			SafeFread((char*)&TempMaskWORD, 2, 1, fp);

			SafeFread((char*)&BlueMask, 2, 1, fp);
			if (IsBigEndian())
			{
				BlueMask = FlipWORD(BlueMask);
			}
			SafeFread((char*)&TempMaskWORD, 2, 1, fp);
		}

		// read and skip any meta data

		if (BytesToSkip > 0)
		{
			if (NexusWarnings)
			{
				cout << "Nexus Warning: Extra meta data detected in file "
					<< FileName << endl
					<< "               Data will be skipped." << endl;
			}
			NDI_BYTE* TempSkipBYTE;
			TempSkipBYTE = new NDI_BYTE[BytesToSkip];
			SafeFread((char*)TempSkipBYTE, BytesToSkip, 1, fp);
			delete[] TempSkipBYTE;
		}

		// determine the red, green and blue shifts

		int GreenShift = 0;
		NDI_WORD TempShiftWORD = GreenMask;
		while (TempShiftWORD > 31)
		{
			TempShiftWORD = TempShiftWORD >> 1; GreenShift++;
		}
		int BlueShift = 0;
		TempShiftWORD = BlueMask;
		while (TempShiftWORD > 31)
		{
			TempShiftWORD = TempShiftWORD >> 1; BlueShift++;
		}
		int RedShift = 0;
		TempShiftWORD = RedMask;
		while (TempShiftWORD > 31)
		{
			TempShiftWORD = TempShiftWORD >> 1; RedShift++;
		}

		// read the actual pixels

		for (j = Height - 1; j >= 0; j--)
		{
			i = 0;
			int ReadNumber = 0;
			while (ReadNumber < DataBytes)
			{
				NDI_WORD TempWORD;
				SafeFread((char*)&TempWORD, 2, 1, fp);
				if (IsBigEndian())
				{
					TempWORD = FlipWORD(TempWORD);
				}
				ReadNumber += 2;

				NDI_WORD Red = RedMask & TempWORD;
				NDI_WORD Green = GreenMask & TempWORD;
				NDI_WORD Blue = BlueMask & TempWORD;

				NDI_BYTE BlueBYTE = (NDI_BYTE)8 * (Blue >> BlueShift);
				NDI_BYTE GreenBYTE = (NDI_BYTE)8 * (Green >> GreenShift);
				NDI_BYTE RedBYTE = (NDI_BYTE)8 * (Red >> RedShift);

				(Pixels[i][j]).Red = RedBYTE;
				(Pixels[i][j]).Green = GreenBYTE;
				(Pixels[i][j]).Blue = BlueBYTE;

				i++;
			}
			ReadNumber = 0;
			while (ReadNumber < PaddingBytes)
			{
				NDI_BYTE TempBYTE;
				SafeFread((char*)&TempBYTE, 1, 1, fp);
				ReadNumber++;
			}
		}

	}

	fclose(fp);
	return true;
}

bool BMP::CreateStandardColorTable(void)
{
	using namespace std;
	if (BitDepth != 1 && BitDepth != 4 && BitDepth != 8)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Attempted to create color table at a bit" << endl
				<< "               depth that does not require a color table." << endl
				<< "               Ignoring request." << endl;
		}
		return false;
	}

	if (BitDepth == 1)
	{
		int i;
		for (i = 0; i < 2; i++)
		{
			Colors[i].Red = i * 255;
			Colors[i].Green = i * 255;
			Colors[i].Blue = i * 255;
			Colors[i].Alpha = 0;
		}
		return true;
	}

	if (BitDepth == 4)
	{
		int i = 0;
		int j, k, ell;

		// simplify the code for the first 8 colors
		for (ell = 0; ell < 2; ell++)
		{
			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					Colors[i].Red = j * 128;
					Colors[i].Green = k * 128;
					Colors[i].Blue = ell * 128;
					i++;
				}
			}
		}

		// simplify the code for the last 8 colors
		for (ell = 0; ell < 2; ell++)
		{
			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					Colors[i].Red = j * 255;
					Colors[i].Green = k * 255;
					Colors[i].Blue = ell * 255;
					i++;
				}
			}
		}

		// overwrite the duplicate color
		i = 8;
		Colors[i].Red = 192;
		Colors[i].Green = 192;
		Colors[i].Blue = 192;

		for (i = 0; i < 16; i++)
		{
			Colors[i].Alpha = 0;
		}
		return true;
	}

	if (BitDepth == 8)
	{
		int i = 0;
		int j, k, ell;

		// do an easy loop, which works for all but colors 
		// 0 to 9 and 246 to 255
		for (ell = 0; ell < 4; ell++)
		{
			for (k = 0; k < 8; k++)
			{
				for (j = 0; j < 8; j++)
				{
					Colors[i].Red = j * 32;
					Colors[i].Green = k * 32;
					Colors[i].Blue = ell * 64;
					Colors[i].Alpha = 0;
					i++;
				}
			}
		}

		// now redo the first 8 colors  
		i = 0;
		for (ell = 0; ell < 2; ell++)
		{
			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					Colors[i].Red = j * 128;
					Colors[i].Green = k * 128;
					Colors[i].Blue = ell * 128;
					i++;
				}
			}
		}

		// overwrite colors 7, 8, 9
		i = 7;
		Colors[i].Red = 192;
		Colors[i].Green = 192;
		Colors[i].Blue = 192;
		i++; // 8
		Colors[i].Red = 192;
		Colors[i].Green = 220;
		Colors[i].Blue = 192;
		i++; // 9
		Colors[i].Red = 166;
		Colors[i].Green = 202;
		Colors[i].Blue = 240;

		// overwrite colors 246 to 255 
		i = 246;
		Colors[i].Red = 255;
		Colors[i].Green = 251;
		Colors[i].Blue = 240;
		i++; // 247
		Colors[i].Red = 160;
		Colors[i].Green = 160;
		Colors[i].Blue = 164;
		i++; // 248
		Colors[i].Red = 128;
		Colors[i].Green = 128;
		Colors[i].Blue = 128;
		i++; // 249
		Colors[i].Red = 255;
		Colors[i].Green = 0;
		Colors[i].Blue = 0;
		i++; // 250
		Colors[i].Red = 0;
		Colors[i].Green = 255;
		Colors[i].Blue = 0;
		i++; // 251
		Colors[i].Red = 255;
		Colors[i].Green = 255;
		Colors[i].Blue = 0;
		i++; // 252
		Colors[i].Red = 0;
		Colors[i].Green = 0;
		Colors[i].Blue = 255;
		i++; // 253
		Colors[i].Red = 255;
		Colors[i].Green = 0;
		Colors[i].Blue = 255;
		i++; // 254
		Colors[i].Red = 0;
		Colors[i].Green = 255;
		Colors[i].Blue = 255;
		i++; // 255
		Colors[i].Red = 255;
		Colors[i].Green = 255;
		Colors[i].Blue = 255;

		return true;
	}
	return true;
}

bool SafeFread(char* buffer, int size, int number, FILE* fp)
{
	using namespace std;
	int ItemsRead;
	if (feof(fp))
	{
		return false;
	}
	ItemsRead = (int)fread(buffer, size, number, fp);
	if (ItemsRead < number)
	{
		return false;
	}
	return true;
}

void BMP::SetDPI(int HorizontalDPI, int VerticalDPI)
{
	XPelsPerMeter = (int)(HorizontalDPI * 39.37007874015748);
	YPelsPerMeter = (int)(VerticalDPI * 39.37007874015748);
}

// int BMP::GetVerticalDPI( void ) const
int BMP::GetVerticalDPI(void)
{
	if (!YPelsPerMeter)
	{
		YPelsPerMeter = DefaultYPelsPerMeter;
	}
	return (int)(YPelsPerMeter / (double) 39.37007874015748);
}

// int BMP::GetHorizontalDPI( void ) const
int BMP::GetHorizontalDPI(void)
{
	if (!XPelsPerMeter)
	{
		XPelsPerMeter = DefaultXPelsPerMeter;
	}
	return (int)(XPelsPerMeter / (double) 39.37007874015748);
}

/* These functions are defined in Nexus_VariousBMPutilities.h */

BMFH GetBMFH(const char* szFileNameIn)
{
	using namespace std;
	BMFH bmfh;

	FILE* fp;
	fp = fopen(szFileNameIn, "rb");

	if (!fp)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: Cannot initialize from file "
				<< szFileNameIn << "." << endl
				<< "              File cannot be opened or does not exist."
				<< endl;
		}
		bmfh.bfType = 0;
		return bmfh;
	}

	SafeFread((char*) &(bmfh.bfType), sizeof(NDI_WORD), 1, fp);
	SafeFread((char*) &(bmfh.bfSize), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmfh.bfReserved1), sizeof(NDI_WORD), 1, fp);
	SafeFread((char*) &(bmfh.bfReserved2), sizeof(NDI_WORD), 1, fp);
	SafeFread((char*) &(bmfh.bfOffBits), sizeof(NDI_DWORD), 1, fp);

	fclose(fp);

	if (IsBigEndian())
	{
		bmfh.SwitchEndianess();
	}

	return bmfh;
}

BMIH GetBMIH(const char* szFileNameIn)
{
	using namespace std;
	BMFH bmfh;
	BMIH bmih;

	FILE* fp;
	fp = fopen(szFileNameIn, "rb");

	if (!fp)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: Cannot initialize from file "
				<< szFileNameIn << "." << endl
				<< "              File cannot be opened or does not exist."
				<< endl;
		}
		return bmih;
	}

	// read the bmfh, i.e., first 14 bytes (just to get it out of the way);

	NDI_BYTE TempBYTE;
	int i;
	for (i = 14; i > 0; i--)
	{
		SafeFread((char*)&TempBYTE, sizeof(NDI_BYTE), 1, fp);
	}

	// read the bmih 

	SafeFread((char*) &(bmih.biSize), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmih.biWidth), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmih.biHeight), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmih.biPlanes), sizeof(NDI_WORD), 1, fp);

	SafeFread((char*) &(bmih.biBitCount), sizeof(NDI_WORD), 1, fp);
	SafeFread((char*) &(bmih.biCompression), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmih.biSizeImage), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmih.biXPelsPerMeter), sizeof(NDI_DWORD), 1, fp);

	SafeFread((char*) &(bmih.biYPelsPerMeter), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmih.biClrUsed), sizeof(NDI_DWORD), 1, fp);
	SafeFread((char*) &(bmih.biClrImportant), sizeof(NDI_DWORD), 1, fp);

	fclose(fp);

	if (IsBigEndian())
	{
		bmih.SwitchEndianess();
	}

	return bmih;
}

void DisplayBitmapInfo(const char* szFileNameIn)
{
	using namespace std;
	FILE* fp;
	fp = fopen(szFileNameIn, "rb");

	if (!fp)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: Cannot initialize from file "
				<< szFileNameIn << "." << endl
				<< "              File cannot be opened or does not exist."
				<< endl;
		}
		return;
	}
	fclose(fp);

	// don't duplicate work! Just use the functions from above!

	BMFH bmfh = GetBMFH(szFileNameIn);
	BMIH bmih = GetBMIH(szFileNameIn);

	cout << "File information for file " << szFileNameIn
		<< ":" << endl << endl;

	cout << "BITMAPFILEHEADER:" << endl
		<< "bfType: " << bmfh.bfType << endl
		<< "bfSize: " << bmfh.bfSize << endl
		<< "bfReserved1: " << bmfh.bfReserved1 << endl
		<< "bfReserved2: " << bmfh.bfReserved2 << endl
		<< "bfOffBits: " << bmfh.bfOffBits << endl << endl;

	cout << "BITMAPINFOHEADER:" << endl
		<< "biSize: " << bmih.biSize << endl
		<< "biWidth: " << bmih.biWidth << endl
		<< "biHeight: " << bmih.biHeight << endl
		<< "biPlanes: " << bmih.biPlanes << endl
		<< "biBitCount: " << bmih.biBitCount << endl
		<< "biCompression: " << bmih.biCompression << endl
		<< "biSizeImage: " << bmih.biSizeImage << endl
		<< "biXPelsPerMeter: " << bmih.biXPelsPerMeter << endl
		<< "biYPelsPerMeter: " << bmih.biYPelsPerMeter << endl
		<< "biClrUsed: " << bmih.biClrUsed << endl
		<< "biClrImportant: " << bmih.biClrImportant << endl << endl;
	return;
}

int GetBitmapColorDepth(const char* szFileNameIn)
{
	BMIH bmih = GetBMIH(szFileNameIn);
	return (int)bmih.biBitCount;
}

void PixelToPixelCopy(BMP& From, int FromX, int FromY,
	BMP& To, int ToX, int ToY)
{
	*To(ToX, ToY) = *From(FromX, FromY);
	return;
}

void PixelToPixelCopyTransparent(BMP& From, int FromX, int FromY,
	BMP& To, int ToX, int ToY,
	Pixel& Transparent)
{
	if (From(FromX, FromY)->Red != Transparent.Red ||
		From(FromX, FromY)->Green != Transparent.Green ||
		From(FromX, FromY)->Blue != Transparent.Blue)
	{
		*To(ToX, ToY) = *From(FromX, FromY);
	}
	return;
}

void RangedPixelToPixelCopy(BMP& From, int FromL, int FromR, int FromB, int FromT,
	BMP& To, int ToX, int ToY)
{
	// make sure the conventions are followed
	if (FromB < FromT)
	{
		int Temp = FromT; FromT = FromB; FromB = Temp;
	}

	// make sure that the copied regions exist in both bitmaps
	if (FromR >= From.GetWidth())
	{
		FromR = From.GetWidth() - 1;
	}
	if (FromL < 0) { FromL = 0; }

	if (FromB >= From.GetHeight())
	{
		FromB = From.GetHeight() - 1;
	}
	if (FromT < 0) { FromT = 0; }

	if (ToX + (FromR - FromL) >= To.GetWidth())
	{
		FromR = To.GetWidth() - 1 + FromL - ToX;
	}
	if (ToY + (FromB - FromT) >= To.GetHeight())
	{
		FromB = To.GetHeight() - 1 + FromT - ToY;
	}

	int i, j;
	for (j = FromT; j <= FromB; j++)
	{
		for (i = FromL; i <= FromR; i++)
		{
			PixelToPixelCopy(From, i, j,
				To, ToX + (i - FromL), ToY + (j - FromT));
		}
	}

	return;
}

void RangedPixelToPixelCopyTransparent(
	BMP& From, int FromL, int FromR, int FromB, int FromT,
	BMP& To, int ToX, int ToY,
	Pixel& Transparent)
{
	// make sure the conventions are followed
	if (FromB < FromT)
	{
		int Temp = FromT; FromT = FromB; FromB = Temp;
	}

	// make sure that the copied regions exist in both bitmaps
	if (FromR >= From.GetWidth())
	{
		FromR = From.GetWidth() - 1;
	}
	if (FromL < 0) { FromL = 0; }

	if (FromB >= From.GetHeight())
	{
		FromB = From.GetHeight() - 1;
	}
	if (FromT < 0) { FromT = 0; }

	if (ToX + (FromR - FromL) >= To.GetWidth())
	{
		FromR = To.GetWidth() - 1 + FromL - ToX;
	}
	if (ToY + (FromB - FromT) >= To.GetHeight())
	{
		FromB = To.GetHeight() - 1 + FromT - ToY;
	}

	int i, j;
	for (j = FromT; j <= FromB; j++)
	{
		for (i = FromL; i <= FromR; i++)
		{
			PixelToPixelCopyTransparent(From, i, j,
				To, ToX + (i - FromL), ToY + (j - FromT),
				Transparent);
		}
	}

	return;
}

bool CreateGrayscaleColorTable(BMP& InputImage)
{
	using namespace std;
	int BitDepth = InputImage.GetBitDepth();
	if (BitDepth != 1 && BitDepth != 4 && BitDepth != 8)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Warning: Attempted to create color table at a bit" << endl
				<< "               depth that does not require a color table." << endl
				<< "               Ignoring request." << endl;
		}
		return false;
	}
	int i;
	int NumberOfColors = InputImage.GetNumberOfColors();

	NDI_BYTE StepSize;
	if (BitDepth != 1)
	{
		StepSize = 255 / (NumberOfColors - 1);
	}
	else
	{
		StepSize = 255;
	}

	for (i = 0; i < NumberOfColors; i++)
	{
		NDI_BYTE TempBYTE = i*StepSize;
		Pixel TempColor;
		TempColor.Red = TempBYTE;
		TempColor.Green = TempBYTE;
		TempColor.Blue = TempBYTE;
		TempColor.Alpha = 0;
		InputImage.SetColor(i, TempColor);
	}
	return true;
}

bool BMP::Read32bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int i;
	if (Width * 4 > BufferSize)
	{
		return false;
	}
	for (i = 0; i < Width; i++)
	{
		memcpy((char*) &(Pixels[i][Row]), (char*)Buffer + 4 * i, 4);
	}
	return true;
}

bool BMP::Read24bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int i;
	if (Width * 3 > BufferSize)
	{
		return false;
	}
	for (i = 0; i < Width; i++)
	{
		memcpy((char*) &(Pixels[i][Row]), Buffer + 3 * i, 3);
	}
	return true;
}

bool BMP::Read8bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int i;
	if (Width > BufferSize)
	{
		return false;
	}
	for (i = 0; i < Width; i++)
	{
		int Index = Buffer[i];
		*(this->operator()(i, Row)) = GetColor(Index);
	}
	return true;
}

bool BMP::Read4bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int Shifts[2] = { 4  ,0 };
	int Masks[2] = { 240,15 };

	int i = 0;
	int j;
	int k = 0;
	if (Width > 2 * BufferSize)
	{
		return false;
	}
	while (i < Width)
	{
		j = 0;
		while (j < 2 && i < Width)
		{
			int Index = (int)((Buffer[k] & Masks[j]) >> Shifts[j]);
			*(this->operator()(i, Row)) = GetColor(Index);
			i++; j++;
		}
		k++;
	}
	return true;
}

bool BMP::Read1bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int Shifts[8] = { 7  ,6 ,5 ,4 ,3,2,1,0 };
	int Masks[8] = { 128,64,32,16,8,4,2,1 };

	int i = 0;
	int j;
	int k = 0;

	if (Width > 8 * BufferSize)
	{
		return false;
	}
	while (i < Width)
	{
		j = 0;
		while (j < 8 && i < Width)
		{
			int Index = (int)((Buffer[k] & Masks[j]) >> Shifts[j]);
			*(this->operator()(i, Row)) = GetColor(Index);
			i++; j++;
		}
		k++;
	}
	return true;
}

bool BMP::Write32bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int i;
	if (Width * 4 > BufferSize)
	{
		return false;
	}
	for (i = 0; i < Width; i++)
	{
		memcpy((char*)Buffer + 4 * i, (char*) &(Pixels[i][Row]), 4);
	}
	return true;
}

bool BMP::Write24bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int i;
	if (Width * 3 > BufferSize)
	{
		return false;
	}
	for (i = 0; i < Width; i++)
	{
		memcpy((char*)Buffer + 3 * i, (char*) &(Pixels[i][Row]), 3);
	}
	return true;
}

bool BMP::Write8bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int i;
	if (Width > BufferSize)
	{
		return false;
	}
	for (i = 0; i < Width; i++)
	{
		Buffer[i] = FindClosestColor(Pixels[i][Row]);
	}
	return true;
}

bool BMP::Write4bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int PositionWeights[2] = { 16,1 };

	int i = 0;
	int j;
	int k = 0;
	if (Width > 2 * BufferSize)
	{
		return false;
	}
	while (i < Width)
	{
		j = 0;
		int Index = 0;
		while (j < 2 && i < Width)
		{
			Index += (PositionWeights[j] * (int)FindClosestColor(Pixels[i][Row]));
			i++; j++;
		}
		Buffer[k] = (NDI_BYTE)Index;
		k++;
	}
	return true;
}

bool BMP::Write1bitRow(NDI_BYTE* Buffer, int BufferSize, int Row)
{
	int PositionWeights[8] = { 128,64,32,16,8,4,2,1 };

	int i = 0;
	int j;
	int k = 0;
	if (Width > 8 * BufferSize)
	{
		return false;
	}
	while (i < Width)
	{
		j = 0;
		int Index = 0;
		while (j < 8 && i < Width)
		{
			Index += (PositionWeights[j] * (int)FindClosestColor(Pixels[i][Row]));
			i++; j++;
		}
		Buffer[k] = (NDI_BYTE)Index;
		k++;
	}
	return true;
}

NDI_BYTE BMP::FindClosestColor(Pixel& input)
{
	using namespace std;

	int i = 0;
	int NumberOfColors = GetNumberOfColors();
	NDI_BYTE BestI = 0;
	int BestMatch = 999999;

	while (i < NumberOfColors)
	{
		Pixel Attempt = GetColor(i);
		int TempMatch = IntSquare((int)Attempt.Red - (int)input.Red)
			+ IntSquare((int)Attempt.Green - (int)input.Green)
			+ IntSquare((int)Attempt.Blue - (int)input.Blue);
		if (TempMatch < BestMatch)
		{
			BestI = (NDI_BYTE)i; BestMatch = TempMatch;
		}
		if (BestMatch < 1)
		{
			i = NumberOfColors;
		}
		i++;
	}
	return BestI;
}

bool NexusCheckDataSize(void)
{
	using namespace std;
	bool ReturnValue = true;
	if (sizeof(NDI_BYTE) != 1)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: NDI_BYTE has the wrong size ("
				<< sizeof(NDI_BYTE) << " bytes)," << endl
				<< "              Compared to the expected 1 byte value" << endl;
		}
		ReturnValue = false;
	}
	if (sizeof(NDI_WORD) != 2)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: NDI_WORD has the wrong size ("
				<< sizeof(NDI_WORD) << " bytes)," << endl
				<< "              Compared to the expected 2 byte value" << endl;
		}
		ReturnValue = false;
	}
	if (sizeof(NDI_DWORD) != 4)
	{
		if (NexusWarnings)
		{
			cout << "Nexus Error: NDI_DWORD has the wrong size ("
				<< sizeof(NDI_DWORD) << " bytes)," << endl
				<< "              Compared to the expected 4 byte value" << endl;
		}
		ReturnValue = false;
	}
	return ReturnValue;
}

bool Rescale(BMP& InputImage, char mode, int NewDimension)
{
	using namespace std;
	int CapMode = toupper(mode);

	BMP OldImage(InputImage);

	if (CapMode != 'P' &&
		CapMode != 'W' &&
		CapMode != 'H' &&
		CapMode != 'F')
	{
		if (NexusWarnings)
		{
			char ErrorMessage[1024];
			sprintf(ErrorMessage, "Nexus Error: Unknown rescale mode %c requested\n", mode);
			cout << ErrorMessage;
		}
		return false;
	}

	int NewWidth = 0;
	int NewHeight = 0;

	int OldWidth = OldImage.GetWidth();
	int OldHeight = OldImage.GetHeight();

	if (CapMode == 'P')
	{
		NewWidth = (int)floor(OldWidth * NewDimension / 100.0);
		NewHeight = (int)floor(OldHeight * NewDimension / 100.0);
	}
	if (CapMode == 'F')
	{
		if (OldWidth > OldHeight)
		{
			CapMode = 'W';
		}
		else
		{
			CapMode = 'H';
		}
	}

	if (CapMode == 'W')
	{
		double percent = (double)NewDimension / (double)OldWidth;
		NewWidth = NewDimension;
		NewHeight = (int)floor(OldHeight * percent);
	}
	if (CapMode == 'H')
	{
		double percent = (double)NewDimension / (double)OldHeight;
		NewHeight = NewDimension;
		NewWidth = (int)floor(OldWidth * percent);
	}

	if (NewWidth < 1)
	{
		NewWidth = 1;
	}
	if (NewHeight < 1)
	{
		NewHeight = 1;
	}

	InputImage.SetSize(NewWidth, NewHeight);
	InputImage.SetBitDepth(24);

	int I, J;
	double ThetaI, ThetaJ;

	for (int j = 0; j < NewHeight - 1; j++)
	{
		ThetaJ = (double)(j*(OldHeight - 1.0))
			/ (double)(NewHeight - 1.0);
		J = (int)floor(ThetaJ);
		ThetaJ -= J;

		for (int i = 0; i < NewWidth - 1; i++)
		{
			ThetaI = (double)(i*(OldWidth - 1.0))
				/ (double)(NewWidth - 1.0);
			I = (int)floor(ThetaI);
			ThetaI -= I;

			InputImage(i, j)->Red = (NDI_BYTE)
				((1.0 - ThetaI - ThetaJ + ThetaI*ThetaJ)*(OldImage(I, J)->Red)
					+ (ThetaI - ThetaI*ThetaJ)*(OldImage(I + 1, J)->Red)
					+ (ThetaJ - ThetaI*ThetaJ)*(OldImage(I, J + 1)->Red)
					+ (ThetaI*ThetaJ)*(OldImage(I + 1, J + 1)->Red));
			InputImage(i, j)->Green = (NDI_BYTE)
				((1.0 - ThetaI - ThetaJ + ThetaI*ThetaJ)*OldImage(I, J)->Green
					+ (ThetaI - ThetaI*ThetaJ)*OldImage(I + 1, J)->Green
					+ (ThetaJ - ThetaI*ThetaJ)*OldImage(I, J + 1)->Green
					+ (ThetaI*ThetaJ)*OldImage(I + 1, J + 1)->Green);
			InputImage(i, j)->Blue = (NDI_BYTE)
				((1.0 - ThetaI - ThetaJ + ThetaI*ThetaJ)*OldImage(I, J)->Blue
					+ (ThetaI - ThetaI*ThetaJ)*OldImage(I + 1, J)->Blue
					+ (ThetaJ - ThetaI*ThetaJ)*OldImage(I, J + 1)->Blue
					+ (ThetaI*ThetaJ)*OldImage(I + 1, J + 1)->Blue);
		}
		InputImage(NewWidth - 1, j)->Red = (NDI_BYTE)
			((1.0 - ThetaJ)*(OldImage(OldWidth - 1, J)->Red)
				+ ThetaJ*(OldImage(OldWidth - 1, J + 1)->Red));
		InputImage(NewWidth - 1, j)->Green = (NDI_BYTE)
			((1.0 - ThetaJ)*(OldImage(OldWidth - 1, J)->Green)
				+ ThetaJ*(OldImage(OldWidth - 1, J + 1)->Green));
		InputImage(NewWidth - 1, j)->Blue = (NDI_BYTE)
			((1.0 - ThetaJ)*(OldImage(OldWidth - 1, J)->Blue)
				+ ThetaJ*(OldImage(OldWidth - 1, J + 1)->Blue));
	}

	for (int i = 0; i < NewWidth - 1; i++)
	{
		ThetaI = (double)(i*(OldWidth - 1.0))
			/ (double)(NewWidth - 1.0);
		I = (int)floor(ThetaI);
		ThetaI -= I;
		InputImage(i, NewHeight - 1)->Red = (NDI_BYTE)
			((1.0 - ThetaI)*(OldImage(I, OldHeight - 1)->Red)
				+ ThetaI*(OldImage(I, OldHeight - 1)->Red));
		InputImage(i, NewHeight - 1)->Green = (NDI_BYTE)
			((1.0 - ThetaI)*(OldImage(I, OldHeight - 1)->Green)
				+ ThetaI*(OldImage(I, OldHeight - 1)->Green));
		InputImage(i, NewHeight - 1)->Blue = (NDI_BYTE)
			((1.0 - ThetaI)*(OldImage(I, OldHeight - 1)->Blue)
				+ ThetaI*(OldImage(I, OldHeight - 1)->Blue));
	}

	*InputImage(NewWidth - 1, NewHeight - 1) = *OldImage(OldWidth - 1, OldHeight - 1);
	return true;
}

