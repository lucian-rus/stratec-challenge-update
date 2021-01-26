#include "imagedisplay.h"
#include "inputmanager.h"

// todo and delete
// this needs an update, very NOT elegant approach to CLI implementation
void console_command()
{
	std::string input;
	std::cout << "> ";
	std::string user_path;
	bool user_path_provided = false;

	while (std::cin >> input)
	{
		if (input.find("path") != std::string::npos)
		{
			std::stringstream in(input);
			std::string value;
			while (std::getline(in, value, '='))
				user_path = value;

			user_path_provided = true;
			
		}
		if (input == "pathoff") { std::cout << "> "; continue; }
		if (input.find("level") != std::string::npos)
		{
			int map, level;
			std::string map_path;

			if (input == "level1") { map = MAP_1; map_path = ""; level = LEVEL_1; }
			if (input == "level2") { map = MAP_1; map_path = ""; level = LEVEL_2; }
			if (input == "level3") { map = MAP_2; map_path = ""; level = LEVEL_3; }
			if (input == "level4") { map = MAP_3; map_path = ""; level = LEVEL_4; }

			if (user_path_provided) { map = MAP_U; map_path = user_path; }

			if (!init_field_data(map, map_path)) { std::cout << INVALID_PATH_CALL << std::endl; break; }
			id_entry_point(level, update_ui_field());
			reset_globals();

			std::cout << "> ";
			continue;
		}
		
		if (input == "debugon")  { set_debug(DEBUG_ON);  std::cout << "> "; continue; }
		if (input == "debugoff") { set_debug(DEBUG_OFF); std::cout << "> "; continue; }

		if (input == "exit") { exit(EXIT_SUCCESS); }

		std::cout << "invalid command" << std::endl;
		std::cout << "> ";
	}
}

int main()
{
	std::thread console(console_command);
	std::thread display(display_image);

	display.join();
	console.join();

	return 0;
}