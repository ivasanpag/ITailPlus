# ITailPlus
Read and copy huge files

Usage:<br/>
		-size <input_file> <output_file> <size_in_MBs> <br/>
		Example: ITailPlus.exe -size big_file.log small_file.log 500<br/>
		ITailPlus will generate X small_file.log of 500 MB each one<br/>
		<br/>
		-lines <input_file> <output_file> <number_of_lines> <br/>
		Example: ITailPlus.exe -lines big_file.log small_file.log 10000<br/>
		ITailPlus will generate X small_file.log of 10.000 lines each one<br/>
		<br/>
		-block <input_file> <br/>
		Example: ITailPlus.exe -block big_file.log <br/>
		ITailPlus will read the file in blocks<br/>
		<br/>
		-search [-i](optional) <input_file> <text_to_search> <br/>
		Example: ITailPlus.exe -search -i big_file.log error<br/>
		ITailPlus will read the file in blocks until found the error word<br/>
		<br/>
		-search_dir <directory> <text_to_search> <br/>
		Example: ITailPlus.exe -search-dir C:\myFolder "Special text"<br/>
		ITailPlus will read each file inside the directory and count you the occurrences of your searched text<br/>
