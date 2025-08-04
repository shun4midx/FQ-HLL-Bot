/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HLL-Bot                      *
 * File Type: C++ file                      *
 * File: utils.cpp                          *
 ****************************************** */

// ======== INCLUDE ======== //
#include "utils.h"

// ======== STATIC VARIABLES ======== //
static std::unique_ptr<Autocorrector> ac;

// ======== FUNCTION IMPLEMENTATION ======== //
bool contains(const std::vector<std::string>& vec, const std::string& str) {
    return std::find(vec.begin(), vec.end(), str) != vec.end();
}


// ~~~~~~~~ FILES ~~~~~~~~ //
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

void init_ac() {
    if (!ac) {
        AutocorrectorCfg cfg;
        cfg.dictionary_list = "test_files/20k_texting.txt";
        ac = std::make_unique<Autocorrector>(cfg);

        std::filesystem::path file = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "files" / "custom_words.txt";
        
        if (std::filesystem::exists(file)) {
            ac->add_dictionary(readFile(file));
        }
    }
}

std::unordered_map<std::string, std::vector<std::string>> autocor(const std::vector<std::string>& vec, int num) {
    init_ac();

    Results res = ac->top3(vec);

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


std::string addToDict(const std::vector<std::string>& vec) {
    try {
        init_ac();

        // See which words to add to dict
        std::vector<std::string> added = ac->add_dictionary(vec);

        if (added.empty()) {
            return "The word" + std::string(vec.size() == 1 ? " is " : "s are ") + "already in the dictionary.";
        }
        
        // Write it into file
        std::filesystem::path file = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "files" / "custom_words.txt";
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

std::string removeFromDict(const std::vector<std::string>& vec) {
    try {
        init_ac();

        // See which words to remove from dict
        std::vector<std::string> removed = ac->remove_dictionary(vec);

        if (removed.empty()) {
            if (vec.size() == 1) {
                return "This word isn't in the dictionary";
            } else {
                return "None of these words are in the dictionary";
            }
        }
        
        // Write it into file
        std::filesystem::path file = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "files" / "custom_words.txt";
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