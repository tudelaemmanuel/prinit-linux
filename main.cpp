#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <string>
#include <stdio.h>
#include <map>
#include <vector>

// For settings files
std::string comment_beg = "//";
// Command args
std::string project_name = "";
std::string project_type = "";
std::string project_workspace = "";
std::string project_path = "";
// Path of the executable
std::string executable_path = "";
std::string option_path = "";

std::string get_path() {
	std::string path = "";
	char* buffer;
	buffer = getenv("PATH");
	path = buffer;
	return path;
}

std::string find_path_keyword(std::string keyword) {
	/*  /!\For linux only /!\
	 *	This function parse the $PATH variable and return the first path containing a certain keyword.
	 */
	const std::string PATH = get_path();
	std::string parsed_path = "";
	long unsigned int keyword_match_index = 0;
	bool found_keyw = false;
	for (long unsigned int i = 0; i < PATH.length(); i++) {
		char ch = PATH[i];
		if (ch == ':') {
			if (found_keyw) { return parsed_path; }
			parsed_path = "";
			keyword_match_index = 0;
		} else {
			parsed_path += ch;
			if (ch == keyword[keyword_match_index]) {
				keyword_match_index++;
				if (keyword_match_index >= keyword.length()) { found_keyw = true; }
			} else {
				keyword_match_index = 0;
			}
		}
	}
	return parsed_path; 
}

int b_length(char* buffer) {
    int length = 0;
    while (buffer[length] != '\0') {
        length++;
    }
    return length;
}

void b_append(char* buff_target, char* buff_copy) {
    int buff_copy_curs = 0;
    int buff_target_curs = b_length(buff_target);
    while (buff_copy[buff_copy_curs] != '\0') {
        buff_target[buff_target_curs] = buff_copy[buff_copy_curs];
        buff_target_curs += 1;
        buff_copy_curs += 1;
    }
}

void str_remove(std::string* str, int amount_to_remove) {
    if (str->length() >= (unsigned long long)amount_to_remove) {
        str->resize(str->length() - amount_to_remove);
    }
}

void str_cut_end(std::string* str, int amount_to_remove) {
    if (str->length() >= (unsigned long long)amount_to_remove) {
        str->resize(str->length() - (amount_to_remove + 1));
    }
}

std::string replace_str(std::string str, std::string to_replace, std::string replace_by) {
    std::string result = "";
    bool replaced = false;
    for (long long unsigned int i = 0; i < str.size(); i++) {
        long long unsigned int j = 0;
        std::string temp = "";
        while (str[i] == to_replace[j] && !replaced && j < to_replace.size() && i < str.size()) {
            temp += str[i];
            j++;
            i++;
        }
        if (temp == "") {
            result += str[i];
        } else if (temp == to_replace) {
            result += replace_by;
            replaced = true;
            i--;
        } else {
            result += temp;
            i--;
        }
    }
    return result;
}

std::string replace_all_str(std::string str, std::string to_replace, std::string replace_by) {
    std::string result = "";
    for (long long unsigned int i = 0; i < str.size(); i++) {
        long long unsigned int j = 0;
        std::string temp = "";
        while (str[i] == to_replace[j] && j < to_replace.size() && i < str.size()) {
            temp += str[i];
            j++;
            i++;
        }
        if (temp == "") {
            result += str[i];
        } else if (temp == to_replace) {
            result += replace_by;
            i--;
        } else {
            result += temp;
            i--;
        }
    }
    return result;
}

std::string get_relative_path(std::string path1, std::string path2) {
    std::string relative_path = "";
    long long unsigned int i = path1.size();
    long long unsigned int j = path2.size();
    while (path1[i] == path2[j] && i * j != 0) {
        relative_path = path1[i] + relative_path;
        i--;
        j--;
    }
    return relative_path;
}

std::string find_file(std::string folder_path, std::string to_find) {
    std::vector<std::string> folders;
    for (const auto & file : std::experimental::filesystem::directory_iterator(folder_path)) {
        std::string file_path = file.path().generic_string();
        std::string file_name = file.path().filename().generic_string();
        if (file_name == to_find) {
            return file_path;
        }
        if (std::experimental::filesystem::is_directory(file.path())) {
            folders.push_back(file_path);
        }
    }
    for (long long unsigned int i = 0; i < folders.size(); i++) {
        std::string subfolder_path = folders[i];
        std::string result = find_file(subfolder_path, to_find);
        if (result != "") {
            return result;
        }
    }
    return "";
}

bool str_endsw(std::string str, std::string end) {
    // Return true if str ends with end, else otherwise
    if (end.length() > str.length()) { return false; };
    int i = str.length() - 1; // str cursor
    int j = end.length() - 1; // end cursor
    for (j = j; j >= 0; j--) {
        if (str[i] != end[j]) {
            return false;
        }
        i--;
    }
    return true;
}


std::string get_execpath(std::string command_path) {
	std::string path = "";
	bool exists_executable = std::experimental::filesystem::exists(command_path);
	bool executable_not_folder = std::experimental::filesystem::is_regular_file(command_path);
	if (exists_executable && executable_not_folder) {
		// remove executable name to get path of it's folder
		path = command_path;
		str_remove(&path, 6);
	} else {
		// retrieve prinit folder form $PATH
		path = find_path_keyword("prinit");
		if (!str_endsw(path, "/")) {
			path += '/';
		}
	}
	return path;	
}

bool str_beginw(std::string str, std::string beg) {
    // Return true if str begins with beg, else otherwise
    if (beg.length() > str.length()) { return false; };
    for (int i = 0; (unsigned long long)i < beg.length(); i++) {
        if (beg[i] != str[i]) {
            return false;
        }
    }
    return true;
}

void clear_prinit_files(std::string path) {
    /* Remove prinit files from project
     */
    std::vector<std::string> prinit_files = {"!prinit-options"};
    for (std::string file : prinit_files) {
        std::string file_path = path + "/" + file;
        std::experimental::filesystem::remove_all(file_path);
    }
}

void init_settings() {
    std::ifstream file;
    std::string settings_path = executable_path;
    settings_path += "settings.txt";
    file.open(settings_path);
    if (file.is_open()) {
        // Do something with the settings
	} else {
        std::cout << "Warning : Missing settings.txt file" << '\n';
        // throw "Missing settings.txt file";
    }
}

int init_type() {
    // path of the wanted template folder
    std::string tfolder_path = executable_path + "templates/" + project_type;
    if (!std::experimental::filesystem::exists(tfolder_path)) {
        std::cout << "Type template do not exists" << '\n';
        return 0;
    }
    return 1;
}

int init_files() {
    // if a folder with project's name exists in workspace
    // path then ask user if he wants to overwrite it
    // else create it
    if (!std::experimental::filesystem::exists(project_path)) {
        // copy files into the directory
        std::string template_path = executable_path + "templates/" + project_type;
        std::experimental::filesystem::copy(template_path, project_path, std::experimental::filesystem::copy_options::recursive);
        return 1;
    } else {
        std::cout << project_path << " already exists, do you want to delete it ? (Y/(other key for NO)) > ";
        char user_choice;
        std::cin >> user_choice;
        if (user_choice == 'Y') {
            std::experimental::filesystem::remove_all(project_path);
            // copy files into the directory
            std::string template_path = executable_path + "templates/" + project_type;
            std::experimental::filesystem::copy(template_path, project_path, std::experimental::filesystem::copy_options::recursive);
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

int init_workspace() {
    // check is workspace mentionned is a real directory
    // workspace is not a real directory => search for the 
    //      real path variable in workspaces.txt
    // workspace is a real directory => use path
    if (!std::experimental::filesystem::exists(project_workspace)) {
        std::ifstream file;
        std::string workspaces_path = executable_path;
        workspaces_path += "workspaces.txt";
        file.open(workspaces_path);
        if (file.is_open()) {
            std::string file_line;
            std::map<std::string, std::string> workspaces;
            // search for workspaces mentions in workspaces.txt
            while (file.good()) {
                std::getline(file, file_line);
                // check if line is not a comment and not empty line
                if (!str_beginw(file_line, comment_beg) && file_line.length() != 0) {
                    // workspace name - workspace path
                    std::string workspc_n = "";
                    std::string workspc_p = "";
                    int i = 0;
                    while (file_line[i] != ' ') {
                        workspc_n += file_line[i];
                        i++;
                    }
                    // skip line separator
                    // "workspace_name : workspace_path"
                    //               ^   
                    //               i + 3
                    i += 3;
                    while (file_line[i] != '\0') {
                        workspc_p += file_line[i];
                        i++;
                    }
                    workspaces[workspc_n] = workspc_p;
                }
            }
            // check if workspace is registered
            if (!workspaces[project_workspace].length() == 0) {
                // project folder path
                if (str_endsw(workspaces[project_workspace], "/")) {
                    project_path = workspaces[project_workspace] + project_name;
                } else {
                    project_path = workspaces[project_workspace] + "/" + project_name;
                }
                // return 1 if everyting gone well 0 otherwise
                return init_files();
            } else {
                return 0;
            }
        }
    } else {
        // project folder path
        if (str_endsw(project_workspace, "/")) {
            project_path = project_workspace + project_name;
        } else {
            project_path = project_workspace + "/" + project_name;
        }
        // return 1 if everyting gone well 0 otherwise
        return init_files();
    }
    return 0;
}

void merge_folders(std::string from_path, std::string to_path) {
    for (const auto & file : std::experimental::filesystem::directory_iterator(from_path)) {
        std::string file_path = file.path().generic_string();
        std::string relative_path = replace_str(from_path, option_path, "");
        if (relative_path == "") {relative_path = ".";}
        if (std::experimental::filesystem::is_directory(file.path())) {
            std::string folder_name = file.path().filename().generic_string();
            std::string existing_folder_path = find_file(to_path, folder_name);
            if (existing_folder_path != "") {
                merge_folders(file_path, existing_folder_path);
            } else {
                std::string new_folder_path = project_path + "/" + relative_path + "/" + folder_name;
                std::experimental::filesystem::copy(file_path, new_folder_path, std::experimental::filesystem::copy_options::recursive);
            }
        } else {
            std::string file_name = file.path().filename().generic_string();
            std::string existing_file_path = find_file(to_path, file_name);
            if (existing_file_path != "") {
                std::experimental::filesystem::remove_all(existing_file_path);
                std::experimental::filesystem::copy(file_path, existing_file_path, std::experimental::filesystem::copy_options::recursive);
            } else {
                std::string new_file_path = project_path + "/" + relative_path + "/" + file_name;
                std::experimental::filesystem::copy(file_path, new_file_path, std::experimental::filesystem::copy_options::recursive);
            }
        }
    }
}


void init_options(char** args) {
    /* Store the options from the command arguments
    */
    std::vector<std::string> options;
    int i = 4;
    while (args[i] != NULL) {
        std::string option = args[i];
        options.push_back(option);
        i++;
    }
    if (options.size() > 0) {
        for (long long unsigned int i = 0; i < options.size(); i++) {
            std::string option = options[i];
            // Search if the option folder exists
            if (option[0] == '.') {
                // Global option
                option = replace_str(option, ".", "");
                option_path = executable_path + "!prinit-options/" + option;
                if (std::experimental::filesystem::exists(option_path)) {
                    merge_folders(option_path, project_path);
                } else {
                    std::cout << "Warning : " << "Global option - " << option << ", doesn't exist" << '\n';
                }
            } else if (option[0] == '-') {
                // Template option
                option = replace_str(option, "-", "");
                option_path = executable_path + "templates/" + project_type + "/!prinit-options/" + option;
                if (std::experimental::filesystem::exists(option_path)) {
                    merge_folders(option_path, project_path);
                } else {
                    std::cout << "Warning : " << "Template option - " << option << ", doesn't exist for " << project_type << " template" << '\n';
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 4) {
        // argv[0] is the name of the executable
        executable_path = get_execpath(argv[0]);
        init_settings();
        // Store args
        project_name = argv[1];
        project_type = argv[2];
        project_workspace = argv[3];
        // itype_s = 1 if type is valid 0 otherwise
        int itype_s = init_type();
        if (itype_s) {
            int iworkspc_s = init_workspace();
            if (iworkspc_s) {
                std::cout << "Project succesfully created" << '\n';
                clear_prinit_files(project_path);
                init_options(argv);
            }else {
                std::cout << "An error has occured" << '\n';
            }
        }
    } else {
        std::cout << "Yout need to specify every parameters, see in settings.txt" << '\n';
    }
    return 0;
}
