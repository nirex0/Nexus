#ifndef _Nexus_Custom_Math_Functions_
#define _Nexus_Custom_Math_Functions_
inline double Square(double number)
{
	return number*number;
}

inline int IntSquare(int number)
{
	return number*number;
}
#endif

int IntPow(int base, int exponent);

#ifndef _Nexus_Defined_WINGDI
#define _Nexus_Defined_WINGDI
typedef unsigned char  NDI_BYTE;
typedef unsigned short NDI_WORD;;
typedef unsigned int  NDI_DWORD;
#endif

#ifndef _Nexus_DataStructures_h_
#define _Nexus_DataStructures_h_

inline bool IsBigEndian()
{
	short word = 0x0001;
	if ((*(char *)& word) != 0x01)
	{
		return true;
	}
	return false;
}

inline NDI_WORD FlipWORD(NDI_WORD in)
{
	return ((in >> 8) | (in << 8));
}

inline NDI_DWORD FlipDWORD(NDI_DWORD in)
{
	return (((in & 0xFF000000) >> 24) | ((in & 0x000000FF) << 24) |
		((in & 0x00FF0000) >> 8) | ((in & 0x0000FF00) << 8));
}

typedef struct Pixel 
{
	NDI_BYTE Blue;
	NDI_BYTE Green;
	NDI_BYTE Red;
	NDI_BYTE Alpha;
} Pixel; 

class BMFH
{
public:
	NDI_WORD  bfType;
	NDI_DWORD bfSize;
	NDI_WORD  bfReserved1;
	NDI_WORD  bfReserved2;
	NDI_DWORD bfOffBits;

	BMFH();
	void display(void);
	void SwitchEndianess(void);
};

class BMIH
{
public:
	NDI_DWORD biSize;
	NDI_DWORD biWidth;
	NDI_DWORD biHeight;
	NDI_WORD  biPlanes;
	NDI_WORD  biBitCount;
	NDI_DWORD biCompression;
	NDI_DWORD biSizeImage;
	NDI_DWORD biXPelsPerMeter;
	NDI_DWORD biYPelsPerMeter;
	NDI_DWORD biClrUsed;
	NDI_DWORD biClrImportant;

	BMIH();
	void display(void);
	void SwitchEndianess(void);
};

#endif
