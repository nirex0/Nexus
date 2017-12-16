#ifndef _Nexus_Bitmap_h_
#define _Nexus_Bitmap_h_
bool SafeFread( char* buffer, int size, int number, FILE* fp );
bool NexusCheckDataSize( void );

class BMP
{
private:

	int BitDepth;
	int Width;
	int Height;
	Pixel** Pixels;
	Pixel* Colors;
	int XPelsPerMeter;
	int YPelsPerMeter;

	NDI_BYTE* MetaData1;
	int SizeOfMetaData1;
	NDI_BYTE* MetaData2;
	int SizeOfMetaData2;

	bool Read32bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Read24bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Read8bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Read4bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Read1bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);

	bool Write32bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Write24bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Write8bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Write4bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);
	bool Write1bitRow(NDI_BYTE* Buffer, int BufferSize, int Row);

	NDI_BYTE FindClosestColor(Pixel& input);

public:

	int GetBitDepth(void);
	int GetWidth(void);
	int GetHeight(void);
	int GetNumberOfColors(void);
	void SetDPI(int HorizontalDPI, int VerticalDPI);
	int GetVerticalDPI(void);
	int GetHorizontalDPI(void);

	BMP();
	BMP(BMP& Input);
	~BMP();
	Pixel* operator()(int i, int j);

	Pixel GetPixel(int i, int j) const;
	bool SetPixel(int i, int j, Pixel NewPixel);

	bool CreateStandardColorTable(void);

	bool SetSize(int NewWidth, int NewHeight);
	bool SetBitDepth(int NewDepth);
	bool WriteToFile(const char* FileName);
	bool ReadFromFile(const char* FileName);

	Pixel GetColor(int ColorNumber);
	bool SetColor(int ColorNumber, Pixel NewColor);
};

#endif
