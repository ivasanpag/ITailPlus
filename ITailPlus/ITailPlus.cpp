// ITailPlus.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//
#include <windows.h>  
#include <iostream>
#include <fstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include "boost/filesystem.hpp"
#include <future>       


using namespace boost::posix_time;


#define MAX_SIZE_BLOCK				(2048*5)
#define MAX_LINES					20
#define SIZE_TO_CREATE_THREADS		500000000

// MAIN FUNCTIONS
int copyIntoFilesBySize(int argc, char **argv);
int copyIntoFilesByNumLines(int argc, char **argv);
int readFileInBlock(int argc, char **argv);
int searchInFile(int argc, char **argv, bool ignoreCase);
int searchInDirectory(int argc, char** argv);
void printUsage();

// UTILS (Move to another file in the future)
int createThreadsToRead(double begin, double end, std::string* file, std::string* textToSearch);
void sendOutputToFileMT(uintmax_t* length, std::string* file, std::string* textToSearch, bool inLines);
int createThreadsToWrite(double begin, double end, std::string* file, std::string* textToSearch, bool inLines, int thread);


int main(int argc, char **argv)
{
	ptime t_begin(microsec_clock::local_time());

	if (argc > 5 || argc < 2) {
		printUsage();
		return 1;
	}

	if (strcmp(argv[1], "-size") == 0) {
		std::cout << "Filtering by size..." << std::endl;
		int rt = copyIntoFilesBySize(argc, argv);

		time_duration diff = microsec_clock::local_time() - t_begin;
		std::cout << "Finish in  " << diff.total_milliseconds() << " ms" << std::endl;

		return rt;
	}
	
	if (strcmp(argv[1], "-lines") == 0) {
		std::cout << "Filtering by lines..." << std::endl;
		int rt = copyIntoFilesByNumLines(argc, argv);

		time_duration diff = microsec_clock::local_time() - t_begin;
		std::cout << "Finish in  " << diff.total_milliseconds() << " ms" << std::endl;
		return rt;
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

	if (strcmp(argv[1], "-search_dir") == 0) {

		int rt = searchInDirectory(argc, argv);
		time_duration diff = microsec_clock::local_time() - t_begin;
		std::cout << "Finish in  " << diff.total_milliseconds() << " ms" << std::endl;
		return rt;
	}
	

	printUsage();
	return 1;
	
}

int searchInFile(int argc, char **argv, bool ignoreCase) {
	std::ifstream input;
	std::string bufferCopy; 
	std::string textToSearch;

	if (ignoreCase) {
	
		input.open(argv[3]);
		if (!input.good()) {
			std::cout << "File " << argv[3] << " does not exist or cannot be opened" << std::endl;
			return 1;
		}
		textToSearch = std::string(argv[4]);
		boost::algorithm::to_lower(textToSearch);
	}
	else {
		input.open(argv[2]);
		if (!input.good()) {
			std::cout << "File " << argv[2] << " does not exist or cannot be opened" << std::endl;
			return 1;
		}
		textToSearch = std::string(argv[3]);
	}
	
	int nLine = 0;

	std::string file;
	std::string line;
	bool showMore = false;
	while (std::getline(input, line))
	{
		nLine++;

		if (ignoreCase) {
			bufferCopy = boost::algorithm::to_lower_copy(line);
		}
		else {
			bufferCopy = line;
		}

		if (showMore) {
			if (bufferCopy.find(textToSearch) != std::string::npos) {
				#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED); // Red
				#endif
			}
			std::cout << line << std::endl;

			#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // White
			#endif
			nLine++;

			if (nLine >= MAX_LINES) {
				std::cout << "\nPress 'c' to show the next block or 'n' to search another occurrence..." << std::endl;
				
				std::string letter;
				std::cin >> letter;

				if (letter[0] == 'n') {
					showMore = false;
					std::cout << std::string('\n', 2);
				}
				else {
					if (letter[0] == 'c') {
						showMore = true;
						nLine = 0;
					}
					else {
						break;
					}
				}

				

			}
		}


		if (!showMore && bufferCopy.find(textToSearch) != std::string::npos) {
			#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED); // Red
			#endif

			std::cout << line << std::endl;
			#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // White
			#endif

			std::cout << "\nPress 'c' to show the next block or 'n' to search another occurrence..." << std::endl;
			std::string letter;
			std::cin >> letter;

			if (letter[0] == 'n') {
				showMore = false;
				std::cout << std::string('\n', 2);
			} else {
				if (letter[0] == 'c') {
					showMore = true;
					nLine = 0;
				} else {
					break;
				}
			}
			
			

		}
	}

	input.close();

	return 0;
}

int readFileInBlock(int argc, char **argv) {
	std::ifstream input;
	std::string nextblock("c");

	char buffer[2048]; // Buffer de 2 Kbytes+
	int bytesRead;


	input.open(argv[2]);
	if (!input.good()) {
		std::cout << "File " << argv[2] << " does not exist or cannot be opened" << std::endl;
		return 1;
	}

	int size = 0;
	
	std::string file;
	do {
		memset(buffer, 0, 2048 * sizeof(char));
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
	memset(buffer, 0, 2048);
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
		memset(buffer, 0, 2048 * sizeof(char));
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

int searchInDirectory(int argc, char** argv) {
	boost::filesystem::path p = argv[2];
	std::string textToSearch(argv[3]);

	int cont = 0;
	std::string bufferCopy;
	

	std::cout << "Reading files... it could take several seconds..." << std::endl;
	for (auto& path : boost::filesystem::directory_iterator(p)) {
		cont = 0;
		if (!boost::filesystem::is_regular_file(path)) {
			continue;
		}

		std::string file = boost::filesystem::canonical(path).string();
		/*
		// std::ifstream input;
		input.open(file);
		if (!input.good()) {
			std::cout << "File " << file << " cannot be opened" << std::endl;
			input.close();
			continue;
		}*/

		std::string name = boost::filesystem::canonical(path).string();
		uintmax_t length = boost::filesystem::file_size(path); //ull


		if (length > SIZE_TO_CREATE_THREADS) {
			// 4 threads
			double count = length / 4;
			double offset_begin[4] = { 0, count, count * 2, count * 3 };
			double offset_end[4] = { count - 1, (count * 2) - 1, (count * 3) - 1, length };

			std::future<int> fut1 = std::async(createThreadsToRead, offset_begin[0], offset_end[0], &file, &textToSearch);
			std::future<int> fut2 = std::async(createThreadsToRead, offset_begin[1], offset_end[1], &file, &textToSearch);
			std::future<int> fut3 = std::async(createThreadsToRead, offset_begin[2], offset_end[2], &file, &textToSearch);
			std::future<int> fut4 = std::async(createThreadsToRead, offset_begin[3], offset_end[3], &file, &textToSearch);

			cont = fut1.get() + fut2.get() + fut3.get() + fut4.get();
		}
		else {
			std::ifstream input;
			input.open(file);
			if (!input.good()) {
				std::cout << "File " << file << " cannot be opened" << std::endl;
				input.close();
				continue;
			}
			std::string line;
			while (std::getline(input, line))
			{

				if (line.find(textToSearch) != std::string::npos) {
					cont++;
				}

			}
			input.close();
		}

		std::cout << "Found " << cont << " occurrences in file " << file << std::endl;
		
		if (cont <= 0) {
			continue;
		}

		std::cout << "Do you want to write these occurrences in a file? (y/N)";
		std::string letter;
		std::cin >> letter;

		if (letter[0] != 'y' && letter[0] != 'Y') {
			continue;
		}
		

		std::cout << "Do you want to send the output in blocks or only the affected lines? (block (recommend) / line)";
		std::string option;
		std::cin >> option;

		if (option == "block") {
			if (length > SIZE_TO_CREATE_THREADS) {
				// We will divide the file in four chunks. In this way we avoid problems with threads writing in the same file
				sendOutputToFileMT(&length, &file, &textToSearch, false);
			}
			else {
				// TODO
				// sendOutputToFileInBlock();
			
			}
		}
		else {
			if (length > SIZE_TO_CREATE_THREADS) {
				// We will divide the file in several chunks. In this way we avoid problems with threads writing in the same file
				sendOutputToFileMT(&length, &file, &textToSearch, true);
			}
			else {
				// TODO
				// sendOutputToFileInLines();
			}
		}
		
	
	}

	return 0;
}


int createThreadsToRead(double begin, double end, std::string* file, std::string *textToSearch) {

	char buffer[2048]; // Buffer de 2 Kbytes+
	int bytesRead;
	double totalBytesRead = 0;

	std::ifstream input;

	double totalBytesToRead = end - begin;

	input.open(*file);
	if (!input.good()) {
		std::cout << "File " << *file << " does not exist or cannot be opened" << std::endl;
		return 0;
	}
	
	std::string bufferCopy;
	int cont = 0;
	
	int len = 2048;
	do {
		if (input.tellg() >= end) {
			break;
		}

		if (totalBytesToRead - totalBytesRead <= 2048) {
			len = totalBytesToRead - totalBytesRead;
		}

		input.seekg(begin + totalBytesRead);

		memset(buffer, 0, len);

		(input).read(buffer, len);
		bytesRead = (input).gcount();
		totalBytesRead += bytesRead;

		bufferCopy = buffer;
		
		if (bufferCopy.find(*textToSearch) != std::string::npos) {
			cont++;
		}
		
		if (totalBytesRead >= totalBytesToRead) {
			break;
		}

	} while (bytesRead > 0);

	input.close();
	return cont;
}

void sendOutputToFileMT(uintmax_t* length, std::string* file, std::string* textToSearch, bool inLines) {
	
	unsigned int processor_count = std::thread::hardware_concurrency();
	if (processor_count <= 0)
		processor_count = 4;

	std::cout << "This file will generate " << processor_count << " output files because of its size" << std::endl;
	double count = *length / processor_count;
	std::vector<std::future<int>> futures;
	int j = 1;
	for (int i = 0; i < processor_count; i++)
	{
		double offset_begin = count * i;
		double offset_end = (count * j) - 1;
		futures.push_back(std::async(createThreadsToWrite, offset_begin, offset_end, file, textToSearch, inLines, i));
		j++;
	}
	int cont = 0;
	for (auto it = futures.begin(); it != futures.end(); it++)
		cont += (*it).get();
	
	
	/*


	// double offset_begin[] = { 0, count, count * 2, count * 3 };
	// double offset_end[4] = { count - 1, (count * 2) - 1, (count * 3) - 1, *length };
	
	
	std::future<int> fut1 = std::async(createThreadsToWrite, offset_begin[0], offset_end[0], file, textToSearch, inLines, 0);
	std::future<int> fut2 = std::async(createThreadsToWrite, offset_begin[1], offset_end[1], file, textToSearch, inLines, 1);
	std::future<int> fut3 = std::async(createThreadsToWrite, offset_begin[2], offset_end[2], file, textToSearch, inLines, 2);
	std::future<int> fut4 = std::async(createThreadsToWrite, offset_begin[3], offset_end[3], file, textToSearch, inLines, 3);
	int cont = fut1.get() + fut2.get() + fut3.get() + fut4.get();
	*/

	
}



int createThreadsToWrite(double begin, double end, std::string* file, std::string* textToSearch, bool inLines, int thread) {

	char buffer[2048]; // Buffer de 2 Kbytes+
	int bytesRead;
	double totalBytesRead = 0;

	std::ifstream input;
	std::ofstream output;

	double totalBytesToRead = end - begin;

	std::string outputFile = (*file) + "_output " + std::to_string(thread) + ".txt";

	input.open(*file);
	if (!input.good()) {
		std::cout << "File " << *file << " cannot be opened" << std::endl;
		return 1;
	}


	output.open(outputFile);
	if (!output.good()) {
		std::cout << "File " << outputFile << " cannot be opened" << std::endl;
		return 2;
	}

	std::string bufferCopy;

	int len = 2048;
	do {
		if (input.tellg() >= end) {
			break;
		}

		if (totalBytesToRead - totalBytesRead <= 2048) {
			len = totalBytesToRead - totalBytesRead;
		}

		input.seekg(begin + totalBytesRead);

		memset(buffer, 0, len);

		(input).read(buffer, len);
		bytesRead = (input).gcount();
		totalBytesRead += bytesRead;

		bufferCopy = buffer;

		if (bufferCopy.find(*textToSearch) != std::string::npos) {
			if (inLines) {
				// Firstly, we will split the buffer by lines. Later, will write 
				// the line with the text requested
				using V = std::vector<std::string>;
				V v;

				for (auto&& r : iter_split(v, bufferCopy, boost::token_finder(boost::is_any_of("\n")))) {
					if (r.find(*textToSearch) != std::string::npos) {
						output.write(r.c_str(), r.length());
						output.write("\n", strlen("\n"));
						// Avoid break because the block can have several occurrences
					}
				}
					

			}
			else {
				output.write(buffer, bytesRead);
			}
		}

		if (totalBytesRead >= totalBytesToRead) {
			break;
		}

	} while (bytesRead > 0);

	input.close();
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
		"ITailPlus will read the file in blocks\n"
		"\n"
		"-search_dir <directory> <text_to_search> \n"
		"Example: ITailPlus.exe -search_dir -i myfolder error\n"
		"ITailPlus will show you the occurrences of the text for each file inside the directory chosen\n");
}