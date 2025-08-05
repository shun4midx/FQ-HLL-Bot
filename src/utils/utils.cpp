/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HLL-Bot                      *
 * File Type: C++ file                      *
 * File: utils.cpp                          *
 ****************************************** */

// ======== INCLUDE ======== //
#include "utils.h"
#include "../env_parser/env_parser.h"

// ======== STATIC VARIABLES ======== //
static std::unordered_map<std::string, std::string> user_keyboards;
static std::unordered_map<std::string, Autocorrector> user_ac;

static std::unordered_map<std::string, std::vector<std::string>> env = parseEnvFile(std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / ".env");
static std::vector<std::string> MESSAGE_PERMS = env["MESSAGE_PERMS"];

// ======== FUNCTION IMPLEMENTATION ======== //
bool contains(const std::vector<std::string>& vec, const std::string& str) {
    return std::find(vec.begin(), vec.end(), str) != vec.end();
}


// ~~~~~~~~ FILES ~~~~~~~~ //
std::filesystem::path filefy(const std::string& str) {
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "files" / str;
}

std::vector<std::string> readFile(const std::filesystem::path& file) {
    std::ifstream curr_file(file);
    std::vector<std::string> curr_arr;
    std::string text_line = "";

    while (getline(curr_file, text_line)) {
        curr_arr.push_back(text_line);
    }
    
    curr_file.close();

    return curr_arr;
}

std::unordered_map<std::string, std::string> readDictionaryFile(const std::filesystem::path& file) {
    std::ifstream curr_file(file);
    std::unordered_map<std::string, std::string> curr_dict;
    std::string text_line = "";

    while (getline(curr_file, text_line)) {
        std::istringstream ss(text_line);
        std::string key, value;

        if (std::getline(ss, key, ':') && std::getline(ss, value)) {
            curr_dict[key] = value;
        }
    }
    
    curr_file.close();

    return curr_dict;
}

void writeFile(std::filesystem::path& file, const std::vector<std::string>& vec) {
    std::ofstream curr_file(file);
    
    for (int i = 0; i < vec.size(); ++i) {
        curr_file << vec[i] + "\n";
    }

    curr_file.close();
}


// ~~~~~~~~ AUTOCORRECT ~~~~~~~~ //
bool isLower(const char c) {
    return c >= 'a' && c <= 'z';
}

char toUpper(const char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    } else {
        return c;
    }
}

int getCaseState(const std::string& word) {
    for (int i = 0; i < word.length(); ++i) {
        if (isLower(word[i])) {
            return (i == 0) ? 0 : 1;
        }
    }

    return 2;
}

void initAC(const std::string& user) {
    std::cout << user << std::endl;
    std::string keyboard = "qwerty";

    if (user_keyboards.find(user) != user_keyboards.end()) {
        keyboard = user_keyboards[user];
    }

    if (user_ac.find(user) == user_ac.end()) {
        AutocorrectorCfg cfg;
        cfg.dictionary_list = "test_files/20k_texting.txt";

        if (keyboard != "None") {
            std::string keyboard_copy = keyboard;
            std::transform(keyboard_copy.begin(), keyboard_copy.end(), keyboard_copy.begin(), ::tolower);

            cfg.keyboard = keyboard_copy;
        } else {
            std::vector<std::string> keyb_vec;
            if (keyboard != "None") {
                std::istringstream iss(keyboard);
               std::string word;
                while (iss >> word) {
                    keyb_vec.push_back(word);
                }
            }
            cfg.keyboard = keyb_vec;
        }

        auto ac = Autocorrector(cfg);

        // Add dict
        if (contains(MESSAGE_PERMS, user)) {
            std::cout << "Got perms!" << std::endl;
            std::filesystem::path file = filefy("custom_words.txt");
            if (std::filesystem::exists(file)) {
                ac.add_dictionary(readFile(file));
                ac.save_dictionary();
            }
        }

        user_ac[user] = ac;
    }
}

std::string setKeyboard(const std::string& user, const std::string& keyboard) {
    std::filesystem::path keyb_file = filefy("user_keyboards.txt");

    std::unordered_map<std::string, std::string> keyb_dict = readDictionaryFile(keyb_file);

    bool unflagged_qwerty = false;

    // Rewrite user_keyboards.txt
    if (keyb_dict.find(user) == keyb_dict.end()) {
        keyb_dict[user] = keyboard;

        std::vector<std::string> keyb_dict_vec = readFile(keyb_file);
        keyb_dict_vec.push_back(user + ":" + keyboard);

        std::string keyboard_copy = keyboard;
        std::transform(keyboard_copy.begin(), keyboard_copy.end(), keyboard_copy.begin(), ::tolower);

        if (keyboard_copy == "qwerty") {
            unflagged_qwerty = true;
        }

        writeFile(keyb_file, keyb_dict_vec);
    } else if (keyb_dict[user] == keyboard) {
        return "This was already your keyboard!";
    } else {
        keyb_dict[user] = keyboard;

        std::vector<std::string> keyb_dict_vec = readFile(keyb_file);

        std::string user_colon = user + ":";

        keyb_dict_vec.erase(
            std::remove_if(keyb_dict_vec.begin(), keyb_dict_vec.end(), [user_colon](const std::string& s) {
                return s.rfind(user_colon, 0) == 0;
            }),
            keyb_dict_vec.end()
        );

        keyb_dict_vec.push_back(user + ":" + keyboard);

        writeFile(keyb_file, keyb_dict_vec);
    }

    // Init new AC
    if (user_ac.find(user) == user_ac.end()) {
        initAC(user);
    }

    // Add to user profile
    user_keyboards[user] = keyboard;

    return (unflagged_qwerty ? "This was already your keyboard by default!" : "Successfuly changed your keyboard to " + keyboard + " for the rest of your upcoming suggestions!");
}

std::string getKeyboard(const std::string& user) {
    std::string raw_keyboard, keyboard_copy;
    if (user_keyboards.find(user) != user_keyboards.end()) {
        raw_keyboard = user_keyboards[user];
    } else {
        std::filesystem::path keyb_file = filefy("user_keyboards.txt");

        std::unordered_map<std::string, std::string> keyb_dict = readDictionaryFile(keyb_file);

        if (keyb_dict.find(user) != keyb_dict.end()) {
            initAC(keyb_dict[user]);
            raw_keyboard = keyb_dict[user];
        } else {
            return "QWERTY (as set by default)";
        }
    }

    keyboard_copy = raw_keyboard;
    std::transform(keyboard_copy.begin(), keyboard_copy.end(), keyboard_copy.begin(), ::tolower);

    if (keyboard_copy == "qwerty" || keyboard_copy == "none" || keyboard_copy == "qwertz" || keyboard_copy == "azerty" || keyboard_copy == "dvorak" || keyboard_copy == "colemak") {
        std::transform(keyboard_copy.begin(), keyboard_copy.end(), keyboard_copy.begin(), ::toupper);
        return keyboard_copy;
    } else {
        std::vector<std::string> vec;

        std::istringstream iss(raw_keyboard);
        std::string word;
        while (iss >> word) {
            vec.push_back(word);
        }

        std::string stringify = "";

        for (std::string& str : vec) {
            stringify += str + "\n";
        }

        return "```" + stringify + "```";
    }
}

std::unordered_map<std::string, std::vector<std::string>> autocor(const std::vector<std::string>& vec, int num, const std::string& user) {
    initAC(user);
    
    Results res = user_ac[user].top3(vec);

    for (int i = 0; i < vec.size(); ++i) {
        std::string str = vec[i];

        // Adjust suggestions based on caps
        int case_state = getCaseState(str);

        if (case_state == 1) {
            for (int i = 0; i < 3; ++i) {
                if (res.suggestions[str][i].length() > 0) {
                    res.suggestions[str][i][0] = toUpper(res.suggestions[str][i][0]);
                }
            }
        } else if (case_state == 2) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < res.suggestions[str][i].length(); ++j) {
                    res.suggestions[str][i][j] = toUpper(res.suggestions[str][i][j]);
                }
            }
        }
    }

    return res.suggestions;
}

std::string display(const std::vector<std::string>& vec, const std::unordered_map<std::string, std::vector<std::string>>& suggestions, const int num) {
    std::string output = "";

    if (vec.empty()) {
        return "";
    }

    for (int i = 0; i < vec.size(); ++i) {
        std::string str = vec[i];

        output += ("`" + str + "`: ");
        
        // Output
        if (num == 1) {
            output += suggestions.at(str)[0];
        } else {
            for (int i = 0; i < num; ++i) {
                output += std::to_string(i + 1) + ". " + suggestions.at(str)[i] + " ";
            }
        }

        output += "\n";
    }

    return output;
}

std::string msg(const std::vector<std::string>& vec, const std::unordered_map<std::string, std::vector<std::string>>& suggestions) {
    std::string output = "";

    if (vec.empty()) {
        return "";
    }

    for (int i = 0; i < vec.size(); ++i) {
        std::string str = vec[i];
        output += suggestions.at(str)[0] + " ";
    }

    return output;
}


std::string addToDict(const std::vector<std::string>& vec, const std::string& user) {
    try {
        initAC(user);

        // See which words to add to dict
        std::vector<std::string> added = user_ac[user].add_dictionary(vec);

        if (added.empty()) {
            return "The word" + std::string(vec.size() == 1 ? " is " : "s are ") + "already in the dictionary.";
        }
        
        // Write it into file
        std::filesystem::path file = filefy("custom_words.txt");
        std::vector<std::string> file_vec = readFile(file);
        
        std::string output = "Successfully added the word" + std::string(added.size() == 1 ? ": " : "s: ");

        for (int i = 0; i < added.size(); ++i) {
            output += added[i];

            if (i + 1 < added.size()) {
                output += ", ";
            }

            file_vec.push_back(added[i]);
        }

        writeFile(file, file_vec);

        return output;

    } catch (const std::exception& e) {
        return e.what();
    }
}

std::string removeFromDict(const std::vector<std::string>& vec, const std::string& user) {
    try {
        initAC(user);

        // See which words to remove from dict
        std::vector<std::string> removed = user_ac[user].remove_dictionary(vec);

        if (removed.empty()) {
            if (vec.size() == 1) {
                return "This word isn't in the dictionary";
            } else {
                return "None of these words are in the dictionary";
            }
        }
        
        // Write it into file
        std::filesystem::path file = filefy("custom_words.txt");
        std::vector<std::string> file_vec = readFile(file);
        
        std::string output = "Successfully removed the word" + std::string(removed.size() == 1 ? ": " : "s: ");

        for (int i = 0; i < removed.size(); ++i) {
            output += removed[i];

            if (i + 1 < removed.size()) {
                output += ", ";
            }
        }

        std::unordered_set<std::string> to_remove(removed.begin(), removed.end());

        file_vec.erase(std::remove_if(file_vec.begin(), file_vec.end(), [&](const std::string& s){
            return to_remove.count(s);
        }), file_vec.end());

        writeFile(file, file_vec);

        return output;

    } catch (const std::exception& e) {
        return e.what();
    }
}