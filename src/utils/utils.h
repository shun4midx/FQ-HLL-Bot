/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HLL-Bot                      *
 * File Type: C++ Header file               *
 * File: utils.h                            *
 ****************************************** */

// ======== INCLUDE ======== //
#include <FQ-HLL/FQ-HLL.h>
#include <vector>
#include <string>
#include <utility>
#include <unordered_set>

// ======== FUNCTION PROTOTYPES ======== //
bool contains(const std::vector<std::string>& vec, const std::string& str);

// ~~~~~~~~ FILES ~~~~~~~~ //
std::filesystem::path filefy(const std::string& str);
std::vector<std::string> readFile(const std::filesystem::path& file);
std::unordered_map<std::string, std::string> readDictionaryFile(const std::filesystem::path& file);
void writeFile(std::filesystem::path& file, const std::vector<std::string>& vec);

// ~~~~~~~~ AUTOCORRECT ~~~~~~~~ //
bool isLower(const char c);
char toUpper(const char c);
int getCaseState(const std::string& word);

void initAC(const std::string& user);

std::string setKeyboard(const std::string& user, const std::string& keyboard);
std::string getKeyboard(const std::string& user);

std::unordered_map<std::string, std::vector<std::string>> autocor(const std::vector<std::string>& vec, int num, const std::string& user);

std::string display(const std::vector<std::string>& vec, const std::unordered_map<std::string, std::vector<std::string>>& suggestions, const int num);
std::string msg(const std::vector<std::string>& vec, const std::unordered_map<std::string, std::vector<std::string>>& suggestions);

std::string addToDict(const std::vector<std::string>& vec, const std::string& user);
std::string removeFromDict(const std::vector<std::string>& vec, const std::string& user);