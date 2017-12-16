#include "Nexus.h"
#include <fstream>
#include <streambuf>
#include <direct.h>

auto main(int argc, char* argv[]) -> int
{

	std::string input1 = "";
	std::string input2 = "";
	std::string input3 = "";
	std::string input4 = "";
	std::string input5 = "";
	std::string input6 = "";

	if (argc >= 2) { input1 = argv[1]; }
	if (argc >= 3) { input2 = argv[2]; }
	if (argc >= 4) { input3 = argv[3]; }
	if (argc >= 5) { input4 = argv[4]; }
	if (argc >= 6) { input5 = argv[5]; }
	if (argc >= 7) { input6 = argv[6]; }

	if (argc == 1) { input1 = "-h"; }

	if (input1 == "-h")
	{
		std::cout << std::endl;
		std::cout << "Nexus Data Injector Usage: " << std::endl << std::endl;
		std::cout << "Inject       : Nexus -i [Image Format] [Input Image] [Input Data] [Output Image] [Optional Password]" << std::endl;
		std::cout << "Retrieve     : Nexus -r [Image Format] [Input Image] [Output Data] [Optional Password]" << std::endl;
		std::cout << "Convert      : Nexus -c [Output Format] [Input Image] [Output Image]" << std::endl;
		std::cout << "Compress     : Nexus -p [Format In Use] [Input Image] [Output Image]" << std::endl;
		std::cout << "Help Menu    : Nexus -h" << std::endl;
		std::cout << "About        : Nexus -a" << std::endl;
		std::cout << "Changelog    : Nexus -l" << std::endl << std::endl;
		std::cout << "Example      : Nexus -i png image.png secret.text output.png AwesomePassword!" << std::endl << std::endl;
		return false;
	}

	// Changelog
	// input1 = option
	if (input1 == "-l")
	{
		std::cout << "1.0.0: Initial Release." << std::endl;
		std::cout << "1.1.0: Encryption Added." << std::endl;
		std::cout << "1.1.1: Fixed A bug with the Encryption System." << std::endl;
		std::cout << "     : Fixed A bug with the input System." << std::endl;
		std::cout << "1.1.3: Fixed A bug with the input System." << std::endl;
		std::cout << "1.2.0: Added Support for \".PNG\" format." << std::endl;
		std::cout << "1.2.2: Added lossless Compression (DEPTH)." << std::endl;
		std::cout << "1.2.4: Added Binary Data Support." << std::endl;
		return false;
	}

	// About
	// input1 = option
	if (input1 == "-a")
	{
		std::cout << std::endl;
		std::cout << "About Nexus Data Injector: " << std::endl;
		std::cout << "Nexus is a Steganography tool, used to hide data within bitmap Images with ease" << std::endl;
		std::cout << "What makes Nexus unique is its ability to use a powerful custom made encryption" << std::endl;
		std::cout << "Algorithm that is made only for this application." << std::endl;
		std::cout << "As of yet the supported image formats are \"BMP\" \"PNG\" However PNG has to be con" << std::endl;
		std::cout << "verted to BMP first and then back to PNG when the operation is done." << std::endl;
		std::cout << std::endl;
		std::cout << "Nexus Data Injector [Version 1.2.5]" << std::endl;
		std::cout << "(C) 2017 Nirex. All rights Reseved." << std::endl;
		std::cout << "Email: Nirex.0 [at] Gmail [dot] Com" << std::endl << std::endl;
		return false;
	}

	// Compress
	// input1 = option
	// input2 = formatInUse
	// input3 = inputIMG
	// input4 = outputIMG
	if (input1 == "-p")
	{
		std::cout << std::endl;
		if (input2 == "png")
		{
			std::cout << "[COMPRESSING]" << std::endl;
			std::cout << "[PHASE 1]" << std::endl;
			std::vector<NDI_BYTE> vecNewBMP = Nexus_Converter::PNG2BMP(input3.c_str());
			_mkdir("TEMP");
			lodepng::save_file(vecNewBMP, "TEMP\\tmp.bmp");

			std::cout << "[PHASE 2]" << std::endl;
			std::vector<NDI_BYTE> vecNewPNG = Nexus_Converter::BMP2PNG("TEMP\\tmp.bmp");
			lodepng::save_file(vecNewPNG, input4.c_str());

			remove("TEMP\\tmp.bmp");
			_rmdir("TEMP");
			std::cout << "[DONE]" << std::endl;
			return 0;
		}
		if (input2 == "bmp")
		{
			std::cout << "[COMPRESSING]" << std::endl;
			std::cout << "[PHASE 1]" << std::endl;
			std::vector<NDI_BYTE> vecNewPNG = Nexus_Converter::BMP2PNG(input3.c_str());
			_mkdir("TEMP");
			lodepng::save_file(vecNewPNG, "TEMP\\tmp.png");

			std::cout << "[PHASE 2]" << std::endl;
			std::vector<NDI_BYTE> vecNewBMP = Nexus_Converter::PNG2BMP("TEMP\\tmp.png");
			lodepng::save_file(vecNewBMP, input4.c_str());

			remove("TEMP\\tmp.png");
			_rmdir("TEMP");
			std::cout << "[DONE]" << std::endl;
			return 0;
		}
		std::cout << "Compress: Nexus -p [Format In Use] [Input Image] [Output Image]" << std::endl;
	}

	// Convert
	// input1 = option
	// input2 = desiredFormat
	// input3 = inputImage
	// input4 = outputImage
	if (input1 == "-c")
	{
		std::cout << std::endl;
		if (input2 == "png")
		{
			std::cout << "[CONVERTING]" << std::endl;
			std::vector<NDI_BYTE> vecNewBMP = Nexus_Converter::BMP2PNG(input3.c_str());
			lodepng::save_file(vecNewBMP, input4);
			std::cout << "[DONE]" << std::endl;
			return 0;
		}
		if (input2 == "bmp")
		{
			std::cout << "[CONVERTING]" << std::endl;
			std::vector<NDI_BYTE> vecNewPNG = Nexus_Converter::PNG2BMP(input3.c_str());
			lodepng::save_file(vecNewPNG, input4);
			std::cout << "[DONE]" << std::endl;
			return 0;
		}
		std::cout << "Convert: Nexus -c [Output Format] [Input Image] [Output Image]" << std::endl;
	}

	// INJECT
	// input1 = option
	// input2 = imageFormat
	// input3 = inputImage
	// input4 = inputData
	// input5 = outputImage
	// input6 = optPassword
	if (input1 == "-i")
	{
		std::cout << std::endl;
		if (input2 == "png")
		{
			std::cout << "[CONVERTING THE PNG FILE TO BMP]" << std::endl;
			std::vector<NDI_BYTE> vecNewBMP = Nexus_Converter::PNG2BMP(input3.c_str());
			_mkdir("TEMP");
			lodepng::save_file(vecNewBMP, "TEMP\\tmp.bmp");
			input3 = "TEMP\\tmp.bmp";
		}

		// Read The image
		std::cout << "[READING IMAGE]" << std::endl;
		BMP inputImage;
		inputImage.ReadFromFile(input3.c_str());

		// Read Data From The File
		std::cout << "[READING DATA]" << std::endl;
		std::ifstream dataFile(input4, ::std::ios::binary);

		// Turn Read Data into a std::string
		std::string data((std::istreambuf_iterator<char>(dataFile)),
			std::istreambuf_iterator<char>());

		// Encrypt The data if the Password Is given
		if (input6 != "")
		{
			// Encrypt The Data
			std::cout << "[ENCRYPTING DATA]" << std::endl;
			std::string EncryptedData = Entropy::Nexus_Encrypt(data, input6);

			// Inject The data into the bits of the Image and Write it back into a new Image
			std::cout << "[CREATING OUTPUT IMAGE]" << std::endl;

			if (input2 == "png")
			{
				std::cout << "[CONVERTING THE BMP FILE TO PNG]" << std::endl;
				Nexus::BMPEmbedText(EncryptedData, inputImage).WriteToFile("TEMP\\tmp.bmp");
				std::vector<NDI_BYTE> vecNewPNG = Nexus_Converter::BMP2PNG("TEMP\\tmp.bmp");
				lodepng::save_file(vecNewPNG, input5.c_str());
				remove("TEMP\\tmp.bmp");
				_rmdir("TEMP");
			}
			else
			{
				Nexus::BMPEmbedText(EncryptedData, inputImage).WriteToFile(input5.c_str());
			}
		}
		else
		{
			// Inject The data into the bits of the Image and Write it back into a new Image
			std::cout << "[CREATING OUTPUT IMAGE]" << std::endl;
			if (input2 == "png")
			{
				std::cout << "[CONVERTING THE BMP FILE TO PNG]" << std::endl;
				Nexus::BMPEmbedText(data, inputImage).WriteToFile("TEMP\\tmp.bmp");
				std::vector<NDI_BYTE> vecNewPNG = Nexus_Converter::BMP2PNG("TEMP\\tmp.bmp");
				lodepng::save_file(vecNewPNG, input5.c_str());
				remove("TEMP\\tmp.bmp");
				_rmdir("TEMP");
			}
			else
			{
				Nexus::BMPEmbedText(data, inputImage).WriteToFile(input5.c_str());
			}
		}
		std::cout << "[DONE]" << std::endl;
	}

	// RETRIEVE
	// input1 = option
	// input2 = imageFormat
	// input3 = inputImage
	// input4 = outputData
	// input5 = optPassword
	if (input1 == "-r")
	{
		std::cout << std::endl;
		if (input2 == "png")
		{
			std::cout << "[CONVERTING THE PNG FILE TO BMP]" << std::endl;
			std::vector<NDI_BYTE> vecNewBMP = Nexus_Converter::PNG2BMP(input3.c_str());
			_mkdir("TEMP");
			lodepng::save_file(vecNewBMP, "TEMP\\tmp.bmp");
			input3 = "TEMP\\tmp.bmp";
		}
		// Read The image
		std::cout << "[READING THE IMAGE]" << std::endl;
		BMP inputImage;
		inputImage.ReadFromFile(input3.c_str());

		// Retrieve The data from the bits of the Image
		std::cout << "[RETRIEVING POSSIBLE DATA]" << std::endl;
		std::string data = Nexus::BMPExtractText(inputImage);
		std::ofstream dataFile(input4, ::std::ios::binary);

		// Decrypt The data if the Password Is given
		if (input5 != "")
		{
			std::cout << "[DECRYPTING POSSIBLE DATA]" << std::endl;
			std::string DecryptedData = Entropy::Nexus_Decrypt(data, input5);

			// Write the Data into the output file
			std::cout << "[CREATING OUTPUT FILE]" << std::endl;
			dataFile << DecryptedData;
			dataFile.close();

			if (input2 == "png")
			{
				remove("TEMP\\tmp.bmp");
				_rmdir("TEMP");
			}
		}
		else
		{
			// Write the Data into the output file
			std::cout << "[CREATING OUTPUT FILE]" << std::endl;
			dataFile << data;
			dataFile.close();

			if (input2 == "png")
			{
				remove("TEMP\\tmp.bmp");
				_rmdir("TEMP");
			}
		}
		std::cout << "[DONE]" << std::endl;
	}


	return false;
}
