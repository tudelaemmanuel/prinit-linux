#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <string>
#include <stdio.h>
#include <map>

std::string comment_beg = "//";
std::string command_scheme = "";
std::string project_name = "";
std::string project_workspace = "";
std::string project_type = "";
std::string executable_path = "";

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

void init_settings(std::string args[]) {
    std::ifstream file;
    std::string settings_path = executable_path;
    settings_path += "settings.txt";
    file.open(settings_path);
    if (file.is_open()) {
        std::string file_line;
        while (file.good()) {
            std::getline(file, file_line);
            // check if line is not a comment and not empty line
            if (!str_beginw(file_line, comment_beg) && file_line.length() != 0) {
                // COMMAND SCHEME
                if (str_beginw(file_line, "command_scheme : ")) {
                    std::string arg_order[3];
                    int i = 0;
                    int argo_i = -1;
                    // read settings line to find commands order
                    while (file_line[i] != '\0') {
                        if (file_line[i] == '[') {
                            argo_i++;
                        }
                        if (argo_i != -1) {
                            if (file_line[i] != ']' && file_line[i] != '[') { 
                                arg_order[argo_i] += file_line[i];
                            }
                        }
                        i++;
                    }
                    // set project infos depending on commands order and
                    // args given by the user
                    for (int j = 0; j < 3; j++) {
                        if (arg_order[j] == "project_name") {
                            project_name = args[j];
                        }else if (arg_order[j] == "project_workspace") {
                            project_workspace = args[j];
                        }else {
                            project_type = args[j];
                        }
                    }
                }
            }
        }
    } else {
        std::cout << "Missing settings.txt file" << '\n';
        throw "Missing settings.txt file";
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

int init_files(std::string projectf_p) {
    // if a folder with project's name exists in workspace
    // path then ask user if he wants to overwrite it
    // else create it
    if (!std::experimental::filesystem::exists(projectf_p)) {
        // copy files into the directory
        std::string template_path = executable_path + "templates/" + project_type;
        std::experimental::filesystem::copy(template_path, projectf_p, std::experimental::filesystem::copy_options::recursive);
        return 1;
    } else {
        std::cout << projectf_p << " already exists, do you want to delete it ? (Y/(other key for NO)) > ";
        char user_choice;
        std::cin >> user_choice;
        if (user_choice == 'Y') {
            std::experimental::filesystem::remove_all(projectf_p);
            // copy files into the directory
            std::string template_path = executable_path + "templates/" + project_type;
            std::experimental::filesystem::copy(template_path, projectf_p, std::experimental::filesystem::copy_options::recursive);
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
                std::string projectf_p = "";
                if (str_endsw(workspaces[project_workspace], "/")) {
                    projectf_p = workspaces[project_workspace] + project_name;
                } else {
                    projectf_p = workspaces[project_workspace] + "/" + project_name;
                }
                // return 1 if everyting gone well 0 otherwise
                return init_files(projectf_p);
            } else {
                return 0;
            }
        }
    } else {
        // project folder path
        std::string projectf_p = "";
        if (str_endsw(project_workspace, "/")) {
            projectf_p = project_workspace + project_name;
        } else {
            projectf_p = project_workspace + "/" + project_name;
        }
        // return 1 if everyting gone well 0 otherwise
        return init_files(projectf_p);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 4) {
        // argv[0] is the name of the executable
        executable_path = get_execpath(argv[0]);
        // put args in array
        std::string args[3] = {argv[1], argv[2], argv[3]};
        init_settings(args);
        // itype_s = 1 if type is valid 0 otherwise
        int itype_s = init_type();
        if (itype_s) {
            int iworkspc_s = init_workspace();
            if (iworkspc_s) {
                std::cout << "Project succesfully created" << '\n';
            }else {
                std::cout << "An error has occured" << '\n';
            }
        }
    } else {
        std::cout << "Yout need to specify every parameters, see in settings.txt" << '\n';
    }
    return 0;
}
