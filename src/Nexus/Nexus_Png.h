#ifndef NEXUS_PNG_H
#define NEXUS_PNG_H

#include <string.h> /*for size_t*/

extern const char* NEXUS_PNG_VERSION_STRING;

/*
The following #defines are used to create code sections. They can be disabled
to disable code sections, which can give faster compile time and smaller binary.
The "NO_COMPILE" defines are designed to be used to pass as defines to the
compiler command to disable them without modifying this header, e.g.
-DNEXUS_PNG_NO_COMPILE_ZLIB for gcc.
In addition to those below, you can also define NEXUS_PNG_NO_COMPILE_CRC to
allow implementing a custom nexuspng_crc32.
*/
/*deflate & zlib. If disabled, you must specify alternative zlib functions in
the custom_zlib field of the compress and decompress settings*/
#ifndef NEXUS_PNG_NO_COMPILE_ZLIB
#define NEXUS_PNG_COMPILE_ZLIB
#endif
/*png encoder and png decoder*/
#ifndef NEXUS_PNG_NO_COMPILE_PNG
#define NEXUS_PNG_COMPILE_PNG
#endif
/*deflate&zlib decoder and png decoder*/
#ifndef NEXUS_PNG_NO_COMPILE_DECODER
#define NEXUS_PNG_COMPILE_DECODER
#endif
/*deflate&zlib encoder and png encoder*/
#ifndef NEXUS_PNG_NO_COMPILE_ENCODER
#define NEXUS_PNG_COMPILE_ENCODER
#endif
/*the optional built in harddisk file loading and saving functions*/
#ifndef NEXUS_PNG_NO_COMPILE_DISK
#define NEXUS_PNG_COMPILE_DISK
#endif
/*support for chunks other than IHDR, IDAT, PLTE, tRNS, IEND: ancillary and unknown chunks*/
#ifndef NEXUS_PNG_NO_COMPILE_ANCILLARY_CHUNKS
#define NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS
#endif
/*ability to convert error numerical codes to English text string*/
#ifndef NEXUS_PNG_NO_COMPILE_ERROR_TEXT
#define NEXUS_PNG_COMPILE_ERROR_TEXT
#endif
/*Compile the default allocators (C's free, malloc and realloc). If you disable this,
you can define the functions nexuspng_free, nexuspng_malloc and nexuspng_realloc in your
source files with custom allocators.*/
#ifndef NEXUS_PNG_NO_COMPILE_ALLOCATORS
#define NEXUS_PNG_COMPILE_ALLOCATORS
#endif
/*compile the C++ version (you can disable the C++ wrapper here even when compiling for C++)*/
#ifdef __cplusplus
#ifndef NEXUS_PNG_NO_COMPILE_CPP
#define NEXUS_PNG_COMPILE_CPP
#endif
#endif

#ifdef NEXUS_PNG_COMPILE_CPP
#include <vector>
#include <string>
#endif /*NEXUS_PNG_COMPILE_CPP*/

#ifdef NEXUS_PNG_COMPILE_PNG
/*The PNG color types (also used for raw).*/
typedef enum NexusPNGColorType
{
  LCT_GREY = 0, /*greyscale: 1,2,4,8,16 bit*/
  LCT_RGB = 2, /*RGB: 8,16 bit*/
  LCT_PALETTE = 3, /*palette: 1,2,4,8 bit*/
  LCT_GREY_ALPHA = 4, /*greyscale with alpha: 8,16 bit*/
  LCT_RGBA = 6 /*RGB with alpha: 8,16 bit*/
} NexusPNGColorType;

#ifdef NEXUS_PNG_COMPILE_DECODER
/*
Converts PNG data in memory to raw pixel data.
out: Output parameter. Pointer to buffer that will contain the raw pixel data.
     After decoding, its size is w * h * (bytes per pixel) bytes larger than
     initially. Bytes per pixel depends on colortype and bitdepth.
     Must be freed after usage with free(*out).
     Note: for 16-bit per channel colors, uses big endian format like PNG does.
w: Output parameter. Pointer to width of pixel data.
h: Output parameter. Pointer to height of pixel data.
in: Memory buffer with the PNG file.
insize: size of the in buffer.
colortype: the desired color type for the raw output image. See explanation on PNG color types.
bitdepth: the desired bit depth for the raw output image. See explanation on PNG color types.
Return value: NexusPNG error code (0 means no error).
*/
unsigned nexuspng_decode_memory(unsigned char** out, unsigned* w, unsigned* h,
                               const unsigned char* in, size_t insize,
                               NexusPNGColorType colortype, unsigned bitdepth);

/*Same as nexuspng_decode_memory, but always decodes to 32-bit RGBA raw image*/
unsigned nexuspng_decode32(unsigned char** out, unsigned* w, unsigned* h,
                          const unsigned char* in, size_t insize);

/*Same as nexuspng_decode_memory, but always decodes to 24-bit RGB raw image*/
unsigned nexuspng_decode24(unsigned char** out, unsigned* w, unsigned* h,
                          const unsigned char* in, size_t insize);

#ifdef NEXUS_PNG_COMPILE_DISK
/*
Load PNG from disk, from file with given name.
Same as the other decode functions, but instead takes a filename as input.
*/
unsigned nexuspng_decode_file(unsigned char** out, unsigned* w, unsigned* h,
                             const char* filename,
                             NexusPNGColorType colortype, unsigned bitdepth);

/*Same as nexuspng_decode_file, but always decodes to 32-bit RGBA raw image.*/
unsigned nexuspng_decode32_file(unsigned char** out, unsigned* w, unsigned* h,
                               const char* filename);

/*Same as nexuspng_decode_file, but always decodes to 24-bit RGB raw image.*/
unsigned nexuspng_decode24_file(unsigned char** out, unsigned* w, unsigned* h,
                               const char* filename);
#endif /*NEXUS_PNG_COMPILE_DISK*/
#endif /*NEXUS_PNG_COMPILE_DECODER*/


#ifdef NEXUS_PNG_COMPILE_ENCODER
/*
Converts raw pixel data into a PNG image in memory. The colortype and bitdepth
  of the output PNG image cannot be chosen, they are automatically determined
  by the colortype, bitdepth and content of the input pixel data.
  Note: for 16-bit per channel colors, needs big endian format like PNG does.
out: Output parameter. Pointer to buffer that will contain the PNG image data.
     Must be freed after usage with free(*out).
outsize: Output parameter. Pointer to the size in bytes of the out buffer.
image: The raw pixel data to encode. The size of this buffer should be
       w * h * (bytes per pixel), bytes per pixel depends on colortype and bitdepth.
w: width of the raw pixel data in pixels.
h: height of the raw pixel data in pixels.
colortype: the color type of the raw input image. See explanation on PNG color types.
bitdepth: the bit depth of the raw input image. See explanation on PNG color types.
Return value: NexusPNG error code (0 means no error).
*/
unsigned nexuspng_encode_memory(unsigned char** out, size_t* outsize,
                               const unsigned char* image, unsigned w, unsigned h,
                               NexusPNGColorType colortype, unsigned bitdepth);

/*Same as nexuspng_encode_memory, but always encodes from 32-bit RGBA raw image.*/
unsigned nexuspng_encode32(unsigned char** out, size_t* outsize,
                          const unsigned char* image, unsigned w, unsigned h);

/*Same as nexuspng_encode_memory, but always encodes from 24-bit RGB raw image.*/
unsigned nexuspng_encode24(unsigned char** out, size_t* outsize,
                          const unsigned char* image, unsigned w, unsigned h);

#ifdef NEXUS_PNG_COMPILE_DISK
/*
Converts raw pixel data into a PNG file on disk.
Same as the other encode functions, but instead takes a filename as output.
NOTE: This overwrites existing files without warning!
*/
unsigned nexuspng_encode_file(const char* filename,
                             const unsigned char* image, unsigned w, unsigned h,
                             NexusPNGColorType colortype, unsigned bitdepth);

/*Same as nexuspng_encode_file, but always encodes from 32-bit RGBA raw image.*/
unsigned nexuspng_encode32_file(const char* filename,
                               const unsigned char* image, unsigned w, unsigned h);

/*Same as nexuspng_encode_file, but always encodes from 24-bit RGB raw image.*/
unsigned nexuspng_encode24_file(const char* filename,
                               const unsigned char* image, unsigned w, unsigned h);
#endif /*NEXUS_PNG_COMPILE_DISK*/
#endif /*NEXUS_PNG_COMPILE_ENCODER*/


#ifdef NEXUS_PNG_COMPILE_CPP
namespace nexuspng
{
#ifdef NEXUS_PNG_COMPILE_DECODER
/*Same as nexuspng_decode_memory, but decodes to an std::vector. The colortype
is the format to output the pixels to. Default is RGBA 8-bit per channel.*/
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                const unsigned char* in, size_t insize,
                NexusPNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                const std::vector<unsigned char>& in,
                NexusPNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#ifdef NEXUS_PNG_COMPILE_DISK
/*
Converts PNG file from disk to raw pixel data in memory.
Same as the other decode functions, but instead takes a filename as input.
*/
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                const std::string& filename,
                NexusPNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#endif /* NEXUS_PNG_COMPILE_DISK */
#endif /* NEXUS_PNG_COMPILE_DECODER */

#ifdef NEXUS_PNG_COMPILE_ENCODER
/*Same as nexuspng_encode_memory, but encodes to an std::vector. colortype
is that of the raw input data. The output PNG color type will be auto chosen.*/
unsigned encode(std::vector<unsigned char>& out,
                const unsigned char* in, unsigned w, unsigned h,
                NexusPNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
unsigned encode(std::vector<unsigned char>& out,
                const std::vector<unsigned char>& in, unsigned w, unsigned h,
                NexusPNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#ifdef NEXUS_PNG_COMPILE_DISK
/*
Converts 32-bit RGBA raw pixel data into a PNG file on disk.
Same as the other encode functions, but instead takes a filename as output.
NOTE: This overwrites existing files without warning!
*/
unsigned encode(const std::string& filename,
                const unsigned char* in, unsigned w, unsigned h,
                NexusPNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
unsigned encode(const std::string& filename,
                const std::vector<unsigned char>& in, unsigned w, unsigned h,
                NexusPNGColorType colortype = LCT_RGBA, unsigned bitdepth = 8);
#endif /* NEXUS_PNG_COMPILE_DISK */
#endif /* NEXUS_PNG_COMPILE_ENCODER */
} /* namespace nexuspng */
#endif /*NEXUS_PNG_COMPILE_CPP*/
#endif /*NEXUS_PNG_COMPILE_PNG*/

#ifdef NEXUS_PNG_COMPILE_ERROR_TEXT
/*Returns an English description of the numerical error code.*/
const char* nexuspng_error_text(unsigned code);
#endif /*NEXUS_PNG_COMPILE_ERROR_TEXT*/

#ifdef NEXUS_PNG_COMPILE_DECODER
/*Settings for zlib decompression*/
typedef struct NexusPNGDecompressSettings NexusPNGDecompressSettings;
struct NexusPNGDecompressSettings
{
  unsigned ignore_adler32; /*if 1, continue and don't give an error message if the Adler32 checksum is corrupted*/

  /*use custom zlib decoder instead of built in one (default: null)*/
  unsigned (*custom_zlib)(unsigned char**, size_t*,
                          const unsigned char*, size_t,
                          const NexusPNGDecompressSettings*);
  /*use custom deflate decoder instead of built in one (default: null)
  if custom_zlib is used, custom_deflate is ignored since only the built in
  zlib function will call custom_deflate*/
  unsigned (*custom_inflate)(unsigned char**, size_t*,
                             const unsigned char*, size_t,
                             const NexusPNGDecompressSettings*);

  const void* custom_context; /*optional custom settings for custom functions*/
};

extern const NexusPNGDecompressSettings nexuspng_default_decompress_settings;
void nexuspng_decompress_settings_init(NexusPNGDecompressSettings* settings);
#endif /*NEXUS_PNG_COMPILE_DECODER*/

#ifdef NEXUS_PNG_COMPILE_ENCODER
/*
Settings for zlib compression. Tweaking these settings tweaks the balance
between speed and compression ratio.
*/
typedef struct NexusPNGCompressSettings NexusPNGCompressSettings;
struct NexusPNGCompressSettings /*deflate = compress*/
{
  /*LZ77 related settings*/
  unsigned btype; /*the block type for LZ (0, 1, 2 or 3, see zlib standard). Should be 2 for proper compression.*/
  unsigned use_lz77; /*whether or not to use LZ77. Should be 1 for proper compression.*/
  unsigned windowsize; /*must be a power of two <= 32768. higher compresses more but is slower. Default value: 2048.*/
  unsigned minmatch; /*mininum lz77 length. 3 is normally best, 6 can be better for some PNGs. Default: 0*/
  unsigned nicematch; /*stop searching if >= this length found. Set to 258 for best compression. Default: 128*/
  unsigned lazymatching; /*use lazy matching: better compression but a bit slower. Default: true*/

  /*use custom zlib encoder instead of built in one (default: null)*/
  unsigned (*custom_zlib)(unsigned char**, size_t*,
                          const unsigned char*, size_t,
                          const NexusPNGCompressSettings*);
  /*use custom deflate encoder instead of built in one (default: null)
  if custom_zlib is used, custom_deflate is ignored since only the built in
  zlib function will call custom_deflate*/
  unsigned (*custom_deflate)(unsigned char**, size_t*,
                             const unsigned char*, size_t,
                             const NexusPNGCompressSettings*);

  const void* custom_context; /*optional custom settings for custom functions*/
};

extern const NexusPNGCompressSettings nexuspng_default_compress_settings;
void nexuspng_compress_settings_init(NexusPNGCompressSettings* settings);
#endif /*NEXUS_PNG_COMPILE_ENCODER*/

#ifdef NEXUS_PNG_COMPILE_PNG
/*
Color mode of an image. Contains all information required to decode the pixel
bits to RGBA colors. This information is the same as used in the PNG file
format, and is used both for PNG and raw image data in NexusPNG.
*/
typedef struct NexusPNGColorMode
{
  /*header (IHDR)*/
  NexusPNGColorType colortype; /*color type, see PNG standard or documentation further in this header file*/
  unsigned bitdepth;  /*bits per sample, see PNG standard or documentation further in this header file*/

  /*
  palette (PLTE and tRNS)

  Dynamically allocated with the colors of the palette, including alpha.
  When encoding a PNG, to store your colors in the palette of the NexusPNGColorMode, first use
  nexuspng_palette_clear, then for each color use nexuspng_palette_add.
  If you encode an image without alpha with palette, don't forget to put value 255 in each A byte of the palette.

  When decoding, by default you can ignore this palette, since NexusPNG already
  fills the palette colors in the pixels of the raw RGBA output.

  The palette is only supported for color type 3.
  */
  unsigned char* palette; /*palette in RGBARGBA... order. When allocated, must be either 0, or have size 1024*/
  size_t palettesize; /*palette size in number of colors (amount of bytes is 4 * palettesize)*/

  /*
  transparent color key (tRNS)

  This color uses the same bit depth as the bitdepth value in this struct, which can be 1-bit to 16-bit.
  For greyscale PNGs, r, g and b will all 3 be set to the same.

  When decoding, by default you can ignore this information, since NexusPNG sets
  pixels with this key to transparent already in the raw RGBA output.

  The color key is only supported for color types 0 and 2.
  */
  unsigned key_defined; /*is a transparent color key given? 0 = false, 1 = true*/
  unsigned key_r;       /*red/greyscale component of color key*/
  unsigned key_g;       /*green component of color key*/
  unsigned key_b;       /*blue component of color key*/
} NexusPNGColorMode;

/*init, cleanup and copy functions to use with this struct*/
void nexuspng_color_mode_init(NexusPNGColorMode* info);
void nexuspng_color_mode_cleanup(NexusPNGColorMode* info);
/*return value is error code (0 means no error)*/
unsigned nexuspng_color_mode_copy(NexusPNGColorMode* dest, const NexusPNGColorMode* source);

void nexuspng_palette_clear(NexusPNGColorMode* info);
/*add 1 color to the palette*/
unsigned nexuspng_palette_add(NexusPNGColorMode* info,
                             unsigned char r, unsigned char g, unsigned char b, unsigned char a);

/*get the total amount of bits per pixel, based on colortype and bitdepth in the struct*/
unsigned nexuspng_get_bpp(const NexusPNGColorMode* info);
/*get the amount of color channels used, based on colortype in the struct.
If a palette is used, it counts as 1 channel.*/
unsigned nexuspng_get_channels(const NexusPNGColorMode* info);
/*is it a greyscale type? (only colortype 0 or 4)*/
unsigned nexuspng_is_greyscale_type(const NexusPNGColorMode* info);
/*has it got an alpha channel? (only colortype 2 or 6)*/
unsigned nexuspng_is_alpha_type(const NexusPNGColorMode* info);
/*has it got a palette? (only colortype 3)*/
unsigned nexuspng_is_palette_type(const NexusPNGColorMode* info);
/*only returns true if there is a palette and there is a value in the palette with alpha < 255.
Loops through the palette to check this.*/
unsigned nexuspng_has_palette_alpha(const NexusPNGColorMode* info);
/*
Check if the given color info indicates the possibility of having non-opaque pixels in the PNG image.
Returns true if the image can have translucent or invisible pixels (it still be opaque if it doesn't use such pixels).
Returns false if the image can only have opaque pixels.
In detail, it returns true only if it's a color type with alpha, or has a palette with non-opaque values,
or if "key_defined" is true.
*/
unsigned nexuspng_can_have_alpha(const NexusPNGColorMode* info);
/*Returns the byte size of a raw image buffer with given width, height and color mode*/
size_t nexuspng_get_raw_size(unsigned w, unsigned h, const NexusPNGColorMode* color);

#ifdef NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS
/*The information of a Time chunk in PNG.*/
typedef struct NexusPNGTime
{
  unsigned year;    /*2 bytes used (0-65535)*/
  unsigned month;   /*1-12*/
  unsigned day;     /*1-31*/
  unsigned hour;    /*0-23*/
  unsigned minute;  /*0-59*/
  unsigned second;  /*0-60 (to allow for leap seconds)*/
} NexusPNGTime;
#endif /*NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS*/

/*Information about the PNG image, except pixels, width and height.*/
typedef struct NexusPNGInfo
{
  /*header (IHDR), palette (PLTE) and transparency (tRNS) chunks*/
  unsigned compression_method;/*compression method of the original file. Always 0.*/
  unsigned filter_method;     /*filter method of the original file*/
  unsigned interlace_method;  /*interlace method of the original file*/
  NexusPNGColorMode color;     /*color type and bits, palette and transparency of the PNG file*/

#ifdef NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS
  /*
  suggested background color chunk (bKGD)
  This color uses the same color mode as the PNG (except alpha channel), which can be 1-bit to 16-bit.

  For greyscale PNGs, r, g and b will all 3 be set to the same. When encoding
  the encoder writes the red one. For palette PNGs: When decoding, the RGB value
  will be stored, not a palette index. But when encoding, specify the index of
  the palette in background_r, the other two are then ignored.

  The decoder does not use this background color to edit the color of pixels.
  */
  unsigned background_defined; /*is a suggested background color given?*/
  unsigned background_r;       /*red component of suggested background color*/
  unsigned background_g;       /*green component of suggested background color*/
  unsigned background_b;       /*blue component of suggested background color*/

  /*
  non-international text chunks (tEXt and zTXt)

  The char** arrays each contain num strings. The actual messages are in
  text_strings, while text_keys are keywords that give a short description what
  the actual text represents, e.g. Title, Author, Description, or anything else.

  A keyword is minimum 1 character and maximum 79 characters long. It's
  discouraged to use a single line length longer than 79 characters for texts.

  Don't allocate these text buffers yourself. Use the init/cleanup functions
  correctly and use nexuspng_add_text and nexuspng_clear_text.
  */
  size_t text_num; /*the amount of texts in these char** buffers (there may be more texts in itext)*/
  char** text_keys; /*the keyword of a text chunk (e.g. "Comment")*/
  char** text_strings; /*the actual text*/

  /*
  international text chunks (iTXt)
  Similar to the non-international text chunks, but with additional strings
  "langtags" and "transkeys".
  */
  size_t itext_num; /*the amount of international texts in this PNG*/
  char** itext_keys; /*the English keyword of the text chunk (e.g. "Comment")*/
  char** itext_langtags; /*language tag for this text's language, ISO/IEC 646 string, e.g. ISO 639 language tag*/
  char** itext_transkeys; /*keyword translated to the international language - UTF-8 string*/
  char** itext_strings; /*the actual international text - UTF-8 string*/

  /*time chunk (tIME)*/
  unsigned time_defined; /*set to 1 to make the encoder generate a tIME chunk*/
  NexusPNGTime time;

  /*phys chunk (pHYs)*/
  unsigned phys_defined; /*if 0, there is no pHYs chunk and the values below are undefined, if 1 else there is one*/
  unsigned phys_x; /*pixels per unit in x direction*/
  unsigned phys_y; /*pixels per unit in y direction*/
  unsigned phys_unit; /*may be 0 (unknown unit) or 1 (metre)*/

  /*
  unknown chunks
  There are 3 buffers, one for each position in the PNG where unknown chunks can appear
  each buffer contains all unknown chunks for that position consecutively
  The 3 buffers are the unknown chunks between certain critical chunks:
  0: IHDR-PLTE, 1: PLTE-IDAT, 2: IDAT-IEND
  Do not allocate or traverse this data yourself. Use the chunk traversing functions declared
  later, such as nexuspng_chunk_next and nexuspng_chunk_append, to read/write this struct.
  */
  unsigned char* unknown_chunks_data[3];
  size_t unknown_chunks_size[3]; /*size in bytes of the unknown chunks, given for protection*/
#endif /*NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS*/
} NexusPNGInfo;

/*init, cleanup and copy functions to use with this struct*/
void nexuspng_info_init(NexusPNGInfo* info);
void nexuspng_info_cleanup(NexusPNGInfo* info);
/*return value is error code (0 means no error)*/
unsigned nexuspng_info_copy(NexusPNGInfo* dest, const NexusPNGInfo* source);

#ifdef NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS
void nexuspng_clear_text(NexusPNGInfo* info); /*use this to clear the texts again after you filled them in*/
unsigned nexuspng_add_text(NexusPNGInfo* info, const char* key, const char* str); /*push back both texts at once*/

void nexuspng_clear_itext(NexusPNGInfo* info); /*use this to clear the itexts again after you filled them in*/
unsigned nexuspng_add_itext(NexusPNGInfo* info, const char* key, const char* langtag,
                           const char* transkey, const char* str); /*push back the 4 texts of 1 chunk at once*/
#endif /*NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS*/

/*
Converts raw buffer from one color type to another color type, based on
NexusPNGColorMode structs to describe the input and output color type.
See the reference manual at the end of this header file to see which color conversions are supported.
return value = NexusPNG error code (0 if all went ok, an error if the conversion isn't supported)
The out buffer must have size (w * h * bpp + 7) / 8, where bpp is the bits per pixel
of the output color type (nexuspng_get_bpp).
For < 8 bpp images, there should not be padding bits at the end of scanlines.
For 16-bit per channel colors, uses big endian format like PNG does.
Return value is NexusPNG error code
*/
unsigned nexuspng_convert(unsigned char* out, const unsigned char* in,
                         const NexusPNGColorMode* mode_out, const NexusPNGColorMode* mode_in,
                         unsigned w, unsigned h);

#ifdef NEXUS_PNG_COMPILE_DECODER
/*
Settings for the decoder. This contains settings for the PNG and the Zlib
decoder, but not the Info settings from the Info structs.
*/
typedef struct NexusPNGDecoderSettings
{
  NexusPNGDecompressSettings zlibsettings; /*in here is the setting to ignore Adler32 checksums*/

  unsigned ignore_crc; /*ignore CRC checksums*/

  unsigned color_convert; /*whether to convert the PNG to the color type you want. Default: yes*/

#ifdef NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS
  unsigned read_text_chunks; /*if false but remember_unknown_chunks is true, they're stored in the unknown chunks*/
  /*store all bytes from unknown chunks in the NexusPNGInfo (off by default, useful for a png editor)*/
  unsigned remember_unknown_chunks;
#endif /*NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS*/
} NexusPNGDecoderSettings;

void nexuspng_decoder_settings_init(NexusPNGDecoderSettings* settings);
#endif /*NEXUS_PNG_COMPILE_DECODER*/

#ifdef NEXUS_PNG_COMPILE_ENCODER
/*automatically use color type with less bits per pixel if losslessly possible. Default: AUTO*/
typedef enum NexusPNGFilterStrategy
{
  /*every filter at zero*/
  LFS_ZERO,
  /*Use filter that gives minimum sum, as described in the official PNG filter heuristic.*/
  LFS_MINSUM,
  /*Use the filter type that gives smallest Shannon entropy for this scanline. Depending
  on the image, this is better or worse than minsum.*/
  LFS_ENTROPY,
  /*
  Brute-force-search PNG filters by compressing each filter for each scanline.
  Experimental, very slow, and only rarely gives better compression than MINSUM.
  */
  LFS_BRUTE_FORCE,
  /*use predefined_filters buffer: you specify the filter type for each scanline*/
  LFS_PREDEFINED
} NexusPNGFilterStrategy;

/*Gives characteristics about the colors of the image, which helps decide which color model to use for encoding.
Used internally by default if "auto_convert" is enabled. Public because it's useful for custom algorithms.*/
typedef struct NexusPNGColorProfile
{
  unsigned colored; /*not greyscale*/
  unsigned key; /*image is not opaque and color key is possible instead of full alpha*/
  unsigned short key_r; /*key values, always as 16-bit, in 8-bit case the byte is duplicated, e.g. 65535 means 255*/
  unsigned short key_g;
  unsigned short key_b;
  unsigned alpha; /*image is not opaque and alpha channel or alpha palette required*/
  unsigned numcolors; /*amount of colors, up to 257. Not valid if bits == 16.*/
  unsigned char palette[1024]; /*Remembers up to the first 256 RGBA colors, in no particular order*/
  unsigned bits; /*bits per channel (not for palette). 1,2 or 4 for greyscale only. 16 if 16-bit per channel required.*/
} NexusPNGColorProfile;

void nexuspng_color_profile_init(NexusPNGColorProfile* profile);

/*Get a NexusPNGColorProfile of the image.*/
unsigned nexuspng_get_color_profile(NexusPNGColorProfile* profile,
                                   const unsigned char* image, unsigned w, unsigned h,
                                   const NexusPNGColorMode* mode_in);
/*The function NexusPNG uses internally to decide the PNG color with auto_convert.
Chooses an optimal color model, e.g. grey if only grey pixels, palette if < 256 colors, ...*/
unsigned nexuspng_auto_choose_color(NexusPNGColorMode* mode_out,
                                   const unsigned char* image, unsigned w, unsigned h,
                                   const NexusPNGColorMode* mode_in);

/*Settings for the encoder.*/
typedef struct NexusPNGEncoderSettings
{
  NexusPNGCompressSettings zlibsettings; /*settings for the zlib encoder, such as window size, ...*/

  unsigned auto_convert; /*automatically choose output PNG color type. Default: true*/

  /*If true, follows the official PNG heuristic: if the PNG uses a palette or lower than
  8 bit depth, set all filters to zero. Otherwise use the filter_strategy. Note that to
  completely follow the official PNG heuristic, filter_palette_zero must be true and
  filter_strategy must be LFS_MINSUM*/
  unsigned filter_palette_zero;
  /*Which filter strategy to use when not using zeroes due to filter_palette_zero.
  Set filter_palette_zero to 0 to ensure always using your chosen strategy. Default: LFS_MINSUM*/
  NexusPNGFilterStrategy filter_strategy;
  /*used if filter_strategy is LFS_PREDEFINED. In that case, this must point to a buffer with
  the same length as the amount of scanlines in the image, and each value must <= 5. You
  have to cleanup this buffer, NexusPNG will never free it. Don't forget that filter_palette_zero
  must be set to 0 to ensure this is also used on palette or low bitdepth images.*/
  const unsigned char* predefined_filters;

  /*force creating a PLTE chunk if colortype is 2 or 6 (= a suggested palette).
  If colortype is 3, PLTE is _always_ created.*/
  unsigned force_palette;
#ifdef NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS
  /*add NexusPNG identifier and version as a text chunk, for debugging*/
  unsigned add_id;
  /*encode text chunks as zTXt chunks instead of tEXt chunks, and use compression in iTXt chunks*/
  unsigned text_compression;
#endif /*NEXUS_PNG_COMPILE_ANCILLARY_CHUNKS*/
} NexusPNGEncoderSettings;

void nexuspng_encoder_settings_init(NexusPNGEncoderSettings* settings);
#endif /*NEXUS_PNG_COMPILE_ENCODER*/


#if defined(NEXUS_PNG_COMPILE_DECODER) || defined(NEXUS_PNG_COMPILE_ENCODER)
/*The settings, state and information for extended encoding and decoding.*/
typedef struct NexusPNGState
{
#ifdef NEXUS_PNG_COMPILE_DECODER
  NexusPNGDecoderSettings decoder; /*the decoding settings*/
#endif /*NEXUS_PNG_COMPILE_DECODER*/
#ifdef NEXUS_PNG_COMPILE_ENCODER
  NexusPNGEncoderSettings encoder; /*the encoding settings*/
#endif /*NEXUS_PNG_COMPILE_ENCODER*/
  NexusPNGColorMode info_raw; /*specifies the format in which you would like to get the raw pixel buffer*/
  NexusPNGInfo info_png; /*info of the PNG image obtained after decoding*/
  unsigned error;
#ifdef NEXUS_PNG_COMPILE_CPP
  /* For the nexuspng::State subclass. */
  virtual ~NexusPNGState(){}
#endif
} NexusPNGState;

/*init, cleanup and copy functions to use with this struct*/
void nexuspng_state_init(NexusPNGState* state);
void nexuspng_state_cleanup(NexusPNGState* state);
void nexuspng_state_copy(NexusPNGState* dest, const NexusPNGState* source);
#endif /* defined(NEXUS_PNG_COMPILE_DECODER) || defined(NEXUS_PNG_COMPILE_ENCODER) */

#ifdef NEXUS_PNG_COMPILE_DECODER
/*
Same as nexuspng_decode_memory, but uses a NexusPNGState to allow custom settings and
getting much more information about the PNG image and color mode.
*/
unsigned nexuspng_decode(unsigned char** out, unsigned* w, unsigned* h,
                        NexusPNGState* state,
                        const unsigned char* in, size_t insize);

/*
Read the PNG header, but not the actual data. This returns only the information
that is in the header chunk of the PNG, such as width, height and color type. The
information is placed in the info_png field of the NexusPNGState.
*/
unsigned nexuspng_inspect(unsigned* w, unsigned* h,
                         NexusPNGState* state,
                         const unsigned char* in, size_t insize);
#endif /*NEXUS_PNG_COMPILE_DECODER*/


#ifdef NEXUS_PNG_COMPILE_ENCODER
/*This function allocates the out buffer with standard malloc and stores the size in *outsize.*/
unsigned nexuspng_encode(unsigned char** out, size_t* outsize,
                        const unsigned char* image, unsigned w, unsigned h,
                        NexusPNGState* state);
#endif /*NEXUS_PNG_COMPILE_ENCODER*/

/*
The nexuspng_chunk functions are normally not needed, except to traverse the
unknown chunks stored in the NexusPNGInfo struct, or add new ones to it.
It also allows traversing the chunks of an encoded PNG file yourself.

PNG standard chunk naming conventions:
First byte: uppercase = critical, lowercase = ancillary
Second byte: uppercase = public, lowercase = private
Third byte: must be uppercase
Fourth byte: uppercase = unsafe to copy, lowercase = safe to copy
*/

/*
Gets the length of the data of the chunk. Total chunk length has 12 bytes more.
There must be at least 4 bytes to read from. If the result value is too large,
it may be corrupt data.
*/
unsigned nexuspng_chunk_length(const unsigned char* chunk);

/*puts the 4-byte type in null terminated string*/
void nexuspng_chunk_type(char type[5], const unsigned char* chunk);

/*check if the type is the given type*/
unsigned char nexuspng_chunk_type_equals(const unsigned char* chunk, const char* type);

/*0: it's one of the critical chunk types, 1: it's an ancillary chunk (see PNG standard)*/
unsigned char nexuspng_chunk_ancillary(const unsigned char* chunk);

/*0: public, 1: private (see PNG standard)*/
unsigned char nexuspng_chunk_private(const unsigned char* chunk);

/*0: the chunk is unsafe to copy, 1: the chunk is safe to copy (see PNG standard)*/
unsigned char nexuspng_chunk_safetocopy(const unsigned char* chunk);

/*get pointer to the data of the chunk, where the input points to the header of the chunk*/
unsigned char* nexuspng_chunk_data(unsigned char* chunk);
const unsigned char* nexuspng_chunk_data_const(const unsigned char* chunk);

/*returns 0 if the crc is correct, 1 if it's incorrect (0 for OK as usual!)*/
unsigned nexuspng_chunk_check_crc(const unsigned char* chunk);

/*generates the correct CRC from the data and puts it in the last 4 bytes of the chunk*/
void nexuspng_chunk_generate_crc(unsigned char* chunk);

/*iterate to next chunks. don't use on IEND chunk, as there is no next chunk then*/
unsigned char* nexuspng_chunk_next(unsigned char* chunk);
const unsigned char* nexuspng_chunk_next_const(const unsigned char* chunk);

/*
Appends chunk to the data in out. The given chunk should already have its chunk header.
The out variable and outlength are updated to reflect the new reallocated buffer.
Returns error code (0 if it went ok)
*/
unsigned nexuspng_chunk_append(unsigned char** out, size_t* outlength, const unsigned char* chunk);

/*
Appends new chunk to out. The chunk to append is given by giving its length, type
and data separately. The type is a 4-letter string.
The out variable and outlength are updated to reflect the new reallocated buffer.
Returne error code (0 if it went ok)
*/
unsigned nexuspng_chunk_create(unsigned char** out, size_t* outlength, unsigned length,
                              const char* type, const unsigned char* data);


/*Calculate CRC32 of buffer*/
unsigned nexuspng_crc32(const unsigned char* buf, size_t len);
#endif /*NEXUS_PNG_COMPILE_PNG*/


#ifdef NEXUS_PNG_COMPILE_ZLIB
/*
This zlib part can be used independently to zlib compress and decompress a
buffer. It cannot be used to create gzip files however, and it only supports the
part of zlib that is required for PNG, it does not support dictionaries.
*/

#ifdef NEXUS_PNG_COMPILE_DECODER
/*Inflate a buffer. Inflate is the decompression step of deflate. Out buffer must be freed after use.*/
unsigned nexuspng_inflate(unsigned char** out, size_t* outsize,
                         const unsigned char* in, size_t insize,
                         const NexusPNGDecompressSettings* settings);

/*
Decompresses Zlib data. Reallocates the out buffer and appends the data. The
data must be according to the zlib specification.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid
buffer and *outsize its size in bytes. out must be freed by user after usage.
*/
unsigned nexuspng_zlib_decompress(unsigned char** out, size_t* outsize,
                                 const unsigned char* in, size_t insize,
                                 const NexusPNGDecompressSettings* settings);
#endif /*NEXUS_PNG_COMPILE_DECODER*/

#ifdef NEXUS_PNG_COMPILE_ENCODER
/*
Compresses data with Zlib. Reallocates the out buffer and appends the data.
Zlib adds a small header and trailer around the deflate data.
The data is output in the format of the zlib specification.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid
buffer and *outsize its size in bytes. out must be freed by user after usage.
*/
unsigned nexuspng_zlib_compress(unsigned char** out, size_t* outsize,
                               const unsigned char* in, size_t insize,
                               const NexusPNGCompressSettings* settings);

/*
Find length-limited Huffman code for given frequencies. This function is in the
public interface only for tests, it's used internally by nexuspng_deflate.
*/
unsigned nexuspng_huffman_code_lengths(unsigned* lengths, const unsigned* frequencies,
                                      size_t numcodes, unsigned maxbitlen);

/*Compress a buffer with deflate. See RFC 1951. Out buffer must be freed after use.*/
unsigned nexuspng_deflate(unsigned char** out, size_t* outsize,
                         const unsigned char* in, size_t insize,
                         const NexusPNGCompressSettings* settings);

#endif /*NEXUS_PNG_COMPILE_ENCODER*/
#endif /*NEXUS_PNG_COMPILE_ZLIB*/

#ifdef NEXUS_PNG_COMPILE_DISK
/*
Load a file from disk into buffer. The function allocates the out buffer, and
after usage you should free it.
out: output parameter, contains pointer to loaded buffer.
outsize: output parameter, size of the allocated out buffer
filename: the path to the file to load
return value: error code (0 means ok)
*/
unsigned nexuspng_load_file(unsigned char** out, size_t* outsize, const char* filename);

/*
Save a file from buffer to disk. Warning, if it exists, this function overwrites
the file without warning!
buffer: the buffer to write
buffersize: size of the buffer to write
filename: the path to the file to save to
return value: error code (0 means ok)
*/
unsigned nexuspng_save_file(const unsigned char* buffer, size_t buffersize, const char* filename);
#endif /*NEXUS_PNG_COMPILE_DISK*/

#ifdef NEXUS_PNG_COMPILE_CPP
/* The NexusPNG C++ wrapper uses std::vectors instead of manually allocated memory buffers. */
namespace nexuspng
{
#ifdef NEXUS_PNG_COMPILE_PNG
class State : public NexusPNGState
{
  public:
    State();
    State(const State& other);
    virtual ~State();
    State& operator=(const State& other);
};

#ifdef NEXUS_PNG_COMPILE_DECODER
/* Same as other nexuspng::decode, but using a State for more settings and information. */
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                State& state,
                const unsigned char* in, size_t insize);
unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
                State& state,
                const std::vector<unsigned char>& in);
#endif /*NEXUS_PNG_COMPILE_DECODER*/

#ifdef NEXUS_PNG_COMPILE_ENCODER
/* Same as other nexuspng::encode, but using a State for more settings and information. */
unsigned encode(std::vector<unsigned char>& out,
                const unsigned char* in, unsigned w, unsigned h,
                State& state);
unsigned encode(std::vector<unsigned char>& out,
                const std::vector<unsigned char>& in, unsigned w, unsigned h,
                State& state);
#endif /*NEXUS_PNG_COMPILE_ENCODER*/

#ifdef NEXUS_PNG_COMPILE_DISK
/*
Load a file from disk into an std::vector.
return value: error code (0 means ok)
*/
unsigned load_file(std::vector<unsigned char>& buffer, const std::string& filename);

/*
Save the binary data in an std::vector to a file on disk. The file is overwritten
without warning.
*/
unsigned save_file(const std::vector<unsigned char>& buffer, const std::string& filename);
#endif /* NEXUS_PNG_COMPILE_DISK */
#endif /* NEXUS_PNG_COMPILE_PNG */

#ifdef NEXUS_PNG_COMPILE_ZLIB
#ifdef NEXUS_PNG_COMPILE_DECODER
/* Zlib-decompress an unsigned char buffer */
unsigned decompress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
                    const NexusPNGDecompressSettings& settings = nexuspng_default_decompress_settings);

/* Zlib-decompress an std::vector */
unsigned decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
                    const NexusPNGDecompressSettings& settings = nexuspng_default_decompress_settings);
#endif /* NEXUS_PNG_COMPILE_DECODER */

#ifdef NEXUS_PNG_COMPILE_ENCODER
/* Zlib-compress an unsigned char buffer */
unsigned compress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
                  const NexusPNGCompressSettings& settings = nexuspng_default_compress_settings);

/* Zlib-compress an std::vector */
unsigned compress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
                  const NexusPNGCompressSettings& settings = nexuspng_default_compress_settings);
#endif /* NEXUS_PNG_COMPILE_ENCODER */
#endif /* NEXUS_PNG_COMPILE_ZLIB */
} /* namespace nexuspng */
#endif /*NEXUS_PNG_COMPILE_CPP*/
#endif /*NEXUS_PNG_H inclusion guard*/