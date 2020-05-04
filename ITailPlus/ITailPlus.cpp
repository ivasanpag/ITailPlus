// ITailPlus.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include <fstream>
#include <string>
#include <boost/lexical_cast.hpp>

#define MAX_SIZE_BLOCK (2048*5)

int copyIntoFilesBySize(int argc, char **argv);
int copyIntoFilesByNumLines(int argc, char **argv);
int readFileInBlock(int argc, char **argv);
int searchInFile(int argc, char **argv, bool ignoreCase);
void printUsage();

int main(int argc, char **argv)
{

	if (argc > 5 || argc < 2) {
		printUsage();
		return 1;
	}

	if (strcmp(argv[1], "-size") == 0) {
		std::cout << "Filtering by size..." << std::endl;
		return copyIntoFilesBySize(argc, argv);
	}
	
	if (strcmp(argv[1], "-lines") == 0) {
		std::cout << "Filtering by lines..." << std::endl;
		return copyIntoFilesByNumLines(argc, argv);
	}

	if (strcmp(argv[1], "-block") == 0) {
		std::cout << "Reading file in blocks..." << std::endl;
		return readFileInBlock(argc, argv);
	}

	if (strcmp(argv[1], "-search") == 0) {
		
		bool ignoreCase = false;
		if (strcmp(argv[2], "-i") == 0) {
			std::cout << "Searching in file ignore case activated..." << std::endl;
			ignoreCase = true;
		}
		return searchInFile(argc, argv, ignoreCase);
	}

	printUsage();
	return 1;
	
}

int searchInFile(int argc, char **argv, bool ignoreCase) {
	std::ifstream input;
	char buffer[2048]; // Buffer de 2 Kbytes
	int bytesRead;
	std::string textToSearch;

	if (ignoreCase) {
	
		input.open(argv[3]);
		if (!input.good()) {
			std::cout << "File " << argv[3] << " does not exist or cannot be opened" << std::endl;
			return 1;
		}
		textToSearch = std::string(argv[4]);
	}
	else {
		input.open(argv[2]);
		if (!input.good()) {
			std::cout << "File " << argv[2] << " does not exist or cannot be opened" << std::endl;
			return 1;
		}
		textToSearch = std::string(argv[3]);
	}
	
	int size = 0;

	std::string file;
	bool showMore = false;
	do {
		input.read(buffer, 2048);
		bytesRead = input.gcount();

		if (showMore) {
			printf("%s", buffer);
			size++;
			
			if ((size * 2048) >= MAX_SIZE_BLOCK) {
				std::cout << "\nPress Enter to show the next block or 'n' to search another ocurrence..." << std::endl;
				char letter = std::cin.get();
				if (letter == 'n') {
					showMore = false;
					continue;
				}
				if (letter == '\n') {
					showMore = true;
					size = 0;
				}
				else {
					break;
				}

			}
			
		}

		if (!showMore && std::string(buffer).find(textToSearch) != std::string::npos) {

			printf("%s", buffer);
			std::cout << "\nPress Enter to show the next block or 'n' to search another ocurrence..." << std::endl;
			char letter = std::cin.get();
			if (letter == 'n') {
				showMore = false;
				continue;
			}
			if (letter == '\n') {
				showMore = true;
				size = 0;
			}
			else {
				break;
			}

		}
		
	

	} while (bytesRead > 0);


	input.close();

	return 0;
}

int readFileInBlock(int argc, char **argv) {
	std::ifstream input;
	std::string nextblock("c");

	char buffer[2048]; // Buffer de 2 Kbytes
	int bytesRead;


	input.open(argv[2]);
	if (!input.good()) {
		std::cout << "File " << argv[2] << " does not exist or cannot be opened" << std::endl;
		return 1;
	}

	int size = 0;
	
	std::string file;
	do {
		input.read(buffer, 2048);
		bytesRead = input.gcount();
		
		printf("%s", buffer);
		size++;

		if ((size * 2048) >= MAX_SIZE_BLOCK) {


			std::cout << "\nPress Enter to continue..." << std::endl;
			if (std::cin.get() != '\n') {
				break;
			}
	
			size = 0;
		}

	} while (bytesRead > 0);


	input.close();

	return 0;
}

int  copyIntoFilesByNumLines(int argc, char **argv) {
	std::ifstream input;
	std::ofstream output;
	const std::string output_file(argv[3]);

	std::cout << "Output: " << output_file << std::endl;

	int maxLines = 0;

	input.open(argv[2]);
	if (!input.good()) {
		std::cout << "File " << argv[2] << " does not exist or cannot be opened" << std::endl;
		return 1;
	}

	try
	{
		maxLines = boost::lexical_cast<int>(argv[4]);
		std::cout << "Number of lines: " << maxLines << std::endl;
	}
	catch (const boost::bad_lexical_cast&)
	{
		std::cout << "Incorrect format " << argv[4] << std::endl;
		input.close();
		return 1;
	}


	output.open(output_file);
	if (!output.good()) {
		std::cout << "File " << output_file << " cannot be created." << std::endl;
		input.close();
		return 2;
	}

	int nLine = 0;
	int cont = 1;
	std::string file;
	std::string line;
	while (std::getline(input, line))
	{
		
		output.write(line.c_str(), line.length());
		output.write("\n", strlen("\n"));
		nLine++;

		if (nLine >= maxLines) {
			file = output_file + "_" + std::to_string(cont);
			cont++;
			output.close();

			std::cout << "Output: " << file << std::endl;
			output.open(file);
			if (!output.good()) {
				std::cout << "File " << file << " cannot be created." << std::endl;
				input.close();
				return 2;
			}
	
			nLine = 0;

		}
	}

	input.close();
	output.close();
	return 0;
}

int copyIntoFilesBySize(int argc, char **argv) {
	std::ifstream input;
	std::ofstream output;
	const std::string output_file(argv[3]);

	std::cout << "Output: " << output_file << std::endl;
	char buffer[2048]; // Buffer de 2 Kbytes
	int bytesRead;
	double maxSizeInMB = 0;

	input.open(argv[2]);
	if (!input.good()) {
		std::cout << "File " << argv[2] << " does not exist or cannot be opened" << std::endl;
		return 1;
	}

	try
	{
		maxSizeInMB = boost::lexical_cast<double>(argv[4]); 
		std::cout << "Max size: " << maxSizeInMB << std::endl;
	}
	catch (const boost::bad_lexical_cast&)
	{
		std::cout << "Incorrect format " << argv[4] << std::endl;
		input.close();
		return 1;
	}

	
	output.open(output_file);
	if (!output.good()) {
		std::cout << "File " << output_file << " cannot be created." << std::endl;
		input.close();
		return 2;
	}

	int size = 0;
	int cont = 1;
	std::string file;
	do {
		input.read(buffer, 2048);
		bytesRead = input.gcount();
		output.write(buffer, bytesRead);
		size++;
		// std::cout << "Size " << (size * 2048)/(1000*1000) << " MBs read" << std::endl;
		
		double tmp = (size * 2048) / (1000 * 1000);
		

		if (tmp >= maxSizeInMB) {
			file = output_file + "_" + std::to_string(cont);

			output.close();
	
			std::cout << "Output: " << file << std::endl;
			output.open(file);
			if (!output.good()) {
				std::cout << "File " << file << " cannot be created." << std::endl;
				input.close();
				return 2;
			}
			
			
			size = 0;
			cont++;
		}

	} while (bytesRead > 0);


	input.close();
	output.close();
	return 0;
}


void printUsage() {
	printf("Usage:\n"
		"-size <input_file> <output_file> <size_in_MBs> \n"
		"Example: ITailPlus.exe -size big_file.log small_file.log 500\n"
		"ITailPlus will generate X small_file.log of 500 MB each one\n"
		"\n"
		"-lines <input_file> <output_file> <number_of_lines> \n"
		"Example: ITailPlus.exe -lines big_file.log small_file.log 10000\n"
		"ITailPlus will generate X small_file.log of 10.000 lines each one\n"
		"\n"
		"-block <input_file> \n"
		"Example: ITailPlus.exe -block big_file.log \n"
		"ITailPlus will read the file in blocks\n"
		"\n"
		"-search [-i](optional) <input_file> <text_to_search> \n"
		"Example: ITailPlus.exe -search -i big_file.log error\n"
		"ITailPlus will read the file in blocks\n");
}