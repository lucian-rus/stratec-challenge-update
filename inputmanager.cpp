#include <fstream>

#include "inputmanager.h"

namespace im
{
	/************************* globals **************************/

	int LINE_COUNT = NULL;
	int COL_COUNT = NULL;

	/*********************** functions *************************/

	int ctoi(char c) { return (c - '0'); }

	std::vector<std::vector<int>> get_file_input(std::string path)
	{
		std::ifstream input_file;
		input_file.open(path);

		if (!input_file.is_open())
			std::cout << FILE_OPEN_ERROR;

		std::vector<std::vector<int>> field;
		while (true)
		{
			std::string input_line;
			if (!std::getline(input_file, input_line)) break;

			std::vector<int> line;

			for (int i = 0; i < input_line.size(); i++)
			{
				if (input_line[i] == ',') continue;
				line.push_back(ctoi(input_line[i]));
			}

			field.push_back(line);
		}

		input_file.close();

		LINE_COUNT = field.size();
		COL_COUNT = field[0].size();

		return field;
	}

	std::tuple<int, int, std::vector<std::vector<int>>> get_input_data(int level_id)
	{
		if (level_id == LEVEL_BASICS)
			return std::make_tuple(LINE_COUNT, COL_COUNT, get_file_input(BASICS_PATH));

		if (level_id == LEVEL_DUPLICATES)
			return std::make_tuple(LINE_COUNT, COL_COUNT, get_file_input(DUPLICATES_PATH));

		if (level_id == LEVEL_ADVANCED)
			return std::make_tuple(LINE_COUNT, COL_COUNT, get_file_input(ADVANCED_PATH));

		throw std::invalid_argument(INVALID_ID_CALL);
	}
}