/********************************************
 * Copyright (c) 2025 Shun/翔海 (@shun4midx) *
 * Project: FQ-HLL-Bot                      *
 * File Type: C++ file                      *
 * File: main.cpp                           *
 ****************************************** */

#include <FQ-HLL/FQ-HLL.h>
#include <dpp/dpp.h>
#include <dpp/user.h>
#include <dpp/cluster.h>
#include "env_parser/env_parser.h"
#include "utils/utils.h"

#include <iostream>
#include <format>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <cstdlib>
#include <regex>

int main() {
    std::srand(unsigned(time(NULL)));

    // Env
    static std::unordered_map<std::string, std::vector<std::string>> env = parseEnvFile();
    static std::string BOT_TOKEN = env["BOT_TOKEN"][0];
    static std::vector<std::string> MESSAGE_PERMS = env["MESSAGE_PERMS"];
    static std::string BOT_USERNAME = env["BOT_USERNAME"][0];
    static std::vector<std::string> CONFLICTING_BOTS = env["CONFLICTING_BOTS"];

    static bool correct_messages = true;

    dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);

    // Main Program
    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](const dpp::slashcommand_t& event) {
        const std::string cmd = event.command.get_command_name();

        // ======== SHUN/SHUN4MIDI BASIC INFO ======= //
        if (cmd == "shun_names") {
            event.reply("Shun\n翔海\nShun/翔海\nしゅう\nしゅん\nShun4mi");
        } else if (cmd == "toggle_autocorrect_messages") {
            if (!contains(MESSAGE_PERMS, event.command.usr.username)) {
                event.reply("You do not have message perms!");
            } else {
                correct_messages = !(correct_messages);
                event.reply("You have successfully toggled correct messages to be " + std::string(correct_messages ? "true!" : "false!"));
            }
        } else if (cmd == "custom_dict") {
            if (!contains(MESSAGE_PERMS, event.command.usr.username)) {
                event.reply("You do not have message perms!");
            } else {
                std::filesystem::path file = filefy("custom_words.txt");
                
                if (!std::filesystem::exists(file)) {
                    event.reply("Empty!");
                }
                
                std::vector<std::string> file_vector = readFile(file);

                std::string stringify = "";

                for (std::string& str : file_vector) {
                    stringify += str + "\n";
                }

                if (stringify == "") {
                    event.reply("Empty!");
                } else {
                    event.reply("```" + stringify + "```");
                }
            }
        } else if (cmd == "get_keyboard") {
            event.reply(getKeyboard(event.command.usr.username));
        }

        try {
            if (cmd == "set_keyboard") {
                std::string keyboard = "qwerty";
                
                auto keyb_param = event.get_parameter("keyboard");
                if (std::holds_alternative<std::string>(keyb_param)) {
                    std::string user_input = std::get<std::string>(keyb_param);
                    if (!user_input.empty()) {
                        keyboard = user_input;
                    }
                }

                event.reply(setKeyboard(event.command.usr.username, keyboard));
            }

            if (cmd == "autocorrect" || cmd == "top3") {
                std::string query = std::get<std::string>(event.get_parameter("query"));
                
                std::string number_arg;
                if (cmd == "autocorrect") {
                    number_arg = std::get<std::string>(event.get_parameter("suggestion_number"));
                }

                int num = 3;

                std::string warnings = "";

                if (cmd == "autocorrect") {
                    try {
                        num = std::stoi(number_arg);

                        if (num > 3 || num < 1) {
                            num = 3;
                            warnings += "This was an invalid number. You can only get the top 1 to 3 results\n=> I will default to suggest the top 3 results:\n";
                        }
        
                    } catch (const std::exception& e) {
                        warnings += e.what() + std::string("\n=> I will default to suggest the top 3 results:\n");
                    }
                }

                std::string separator = " ";

                auto sep_param = event.get_parameter("separator");
                if (std::holds_alternative<std::string>(sep_param)) {
                    std::string user_input = std::get<std::string>(sep_param);
                    if (!user_input.empty() && user_input.length() == 1) {
                        separator = user_input;
                    } else if (!user_input.empty()) {
                        warnings += "Invalid separator, separators can only either be a space (empty) or one character long.\n=> I will default to using the space as a separator:\n";
                    }
                }
                
                std::vector<std::string> text_args;
                std::stringstream ss(query);
                std::string word;

                while (std::getline(ss, word, separator[0])) {
                    text_args.push_back(word);
                }

                event.reply(warnings + display(text_args, autocor(text_args, num, event.command.usr.username), num));
            }

            if (cmd == "add_to_dict" || cmd == "remove_from_dict") {
                
                if (!contains(MESSAGE_PERMS, event.command.usr.username)) {
                    event.reply("You do not have message perms!");
                }

                std::string query = std::get<std::string>(event.get_parameter("query"));
                std::string separator = " ";
    
                std::string warnings = "";
    
                auto sep_param = event.get_parameter("separator");
                if (std::holds_alternative<std::string>(sep_param)) {
                    std::string user_input = std::get<std::string>(sep_param);
                    if (!user_input.empty() && user_input.length() == 1) {
                        separator = user_input;
                    } else if (!user_input.empty()) {
                        warnings += "Invalid separator, separators can only either be a space (empty) or one character long.\n=> I will default to using the space as a separator:\n";
                    }
                }

                std::vector<std::string> text_args;
                std::stringstream ss(query);
                std::string word;

                while (std::getline(ss, word, separator[0])) {
                    text_args.push_back(word);
                }
    
                // Add to dict
                if (cmd == "add_to_dict") {
                    event.reply(warnings + addToDict(text_args, event.command.usr.username));
                }
    
                // Remove from dict
                if (cmd == "remove_from_dict") {
                    event.reply(warnings + removeFromDict(text_args, event.command.usr.username));
                }
            }
        } catch (const std::exception& e) {
            event.reply(e.what());
        }
    });

    // ========= LISTENING ======== //
    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        // ======== MY SPECIAL FUNCTIONS ======== //
        // Carry on only if it's not from the bot itself
        if (event.msg.author.format_username() != BOT_USERNAME && !contains(CONFLICTING_BOTS, event.msg.author.format_username())) {
            std::string message = event.msg.content;

            // ======== FQ-HLL-Bot Says (My function only) ======= //
            if (message.find("!bot_say") == 0 && contains(MESSAGE_PERMS, event.msg.author.username)) {
                std::string command = "!bot_say";
                std::string quote = message.substr(command.length() + 1, message.length() - command.length());
                bot.message_delete(event.msg.id, event.msg.channel_id);
                event.send(quote);
            }

            // ======== SPECIAL FUNCTIONS ======== //
            if (message.find("!fqhll ac") == 0) {
                std::string command = "!fqhll ac";
                std::string input = message.substr(command.length() + 1);

                std::regex pattern(R"(^\s*(?:\[([^\]]+)\]|(\S+))\s+(\d+)\s*$)");
                std::smatch match;

                if (std::regex_match(input, match, pattern)) {
                    std::string raw_text = match[1].matched ? match[1].str() : match[2].str();
                    std::string number_arg = match[3].str();
                    int num = 3;

                    try {
                        num = std::stoi(number_arg);
                    } catch (const std::exception& e) {
                        event.send(e.what() + std::string("\n=> I will default to suggest the top 3 results"));
                    }

                    if (num > 3 || num < 1) {
                        num = 3;
                        event.send("This was an invalid number. You can only get the top 1 to 3 results\n=> I will default to suggest the top 3 results");
                    }

                    std::vector<std::string> text_args;
                    std::istringstream iss(raw_text);
                    std::string word;
                    while (iss >> word) {
                        text_args.push_back(word);
                    }

                    event.reply(display(text_args, autocor(text_args, num, event.msg.author.username), num));
                } else {
                    event.reply("Invalid format. Use: `!fqhll ac [text here] num` for several inputs or `!fqhll ac word num`");
                }
            } else if (!message.empty() && message[0] == '[' && message[message.length() - 1] == ']') {
                std::string input = message.substr(1, message.length() - 2);

                std::vector<std::string> text_args;
                std::istringstream iss(input);
                std::string word;
                while (iss >> word) {
                    text_args.push_back(word);
                }

                event.reply(display(text_args, autocor(text_args, 3, event.msg.author.username), 3));
            }

            // ======== AUTORESPONDER ======== //
            std::string og_message = message;
            std::transform(og_message.begin(), og_message.end(), og_message.begin(), ::tolower);
            if (og_message.find("!fqhll") != 0 && (og_message.find("fqhll") != std::string::npos || og_message.find("fq-hll") != std::string::npos || og_message.find("fq_hll") != std::string::npos || og_message.find("dyslexicloglog") != std::string::npos || og_message.find("autocorrector") != std::string::npos)) {
                event.reply("Omg me mention! :D I love FQ-HLL Autocorrect! :red_circle::purple_circle::blue_circle:", true);
            }

            // ======== AUTOCORRECT MESSAGES ======== //
            if (correct_messages && contains(MESSAGE_PERMS, event.msg.author.username) && message[0] != '!' && (message[0] != '[' || message[message.length() - 1] != ']')) { // Don't autocorrect bot messages or autocorrect msgs
                std::vector<std::string> text_args;
                std::istringstream iss(message);
                std::string word;
                while (iss >> word) {
                    text_args.push_back(word);
                }

                event.send(msg(text_args, autocor(text_args, 1, event.msg.author.username)));
            }
        }
    });

    // ======== INIT PART OF THE CODE ======== //
    bot.on_ready([&bot](const dpp::ready_t& event) {
        // Bot status
        bot.set_presence(dpp::presence(dpp::ps_idle, dpp::at_competing, "being the best Autocorrector!"));

        if (dpp::run_once<struct register_bot_commands>()) {
            // bot.global_command_create(dpp::slashcommand("shun_names", "Outputs all forms of Shun's names", bot.me.id));
            
            // bot.global_command_create(dpp::slashcommand("set_keyboard", "Choose your keyboard you want to use (it's default to QWERTY)! Choose any of the autocomplete results, or if they do not suit you, please type your own with each row separated by a space!", bot.me.id)
            //     .add_option(dpp::command_option(dpp::co_string, "keyboard", "Your keyboard", true).set_auto_complete(true)));

            bot.global_command_create(dpp::slashcommand("get_keyboard", "Outputs your current keyboard type", bot.me.id));

            // bot.global_command_create(dpp::slashcommand("toggle_autocorrect_messages", "MESSAGE_PERM users only: Can use this to toggle whether they want their messages autocorrected", bot.me.id));

            // bot.global_command_create(dpp::slashcommand("autocorrect", "Autocorrects a given query", bot.me.id)
            //     .add_option(dpp::command_option(dpp::co_string, "query", "Query", true))
            //     .add_option(dpp::command_option(dpp::co_string, "suggestion_number", "Number of suggestions (from 1 - 3)", true))
            //     .add_option(dpp::command_option(dpp::co_string, "separator", "A single character separator between queries, defaults to a space")));
            // bot.global_command_create(dpp::slashcommand("top3", "Gives top 3 suggestions for a given query", bot.me.id)
            //     .add_option(dpp::command_option(dpp::co_string, "query", "Query", true))
            //     .add_option(dpp::command_option(dpp::co_string, "separator", "A single character separator between queries, defaults to a space")));
        
            // bot.global_command_create(dpp::slashcommand("add_to_dict", "MESSAGE_PERM users only: Adds a custom word to the custom dictionary on top of 20k_texting.txt", bot.me.id)
            //     .add_option(dpp::command_option(dpp::co_string, "query", "Word(s) to be added", true))
            //     .add_option(dpp::command_option(dpp::co_string, "separator", "A single character separator between queries, defaults to a space")));

            // bot.global_command_create(dpp::slashcommand("remove_from_dict", "MESSAGE_PERM users only: Adds a custom word to the custom dictionary on top of 20k_texting.txt", bot.me.id)
            //     .add_option(dpp::command_option(dpp::co_string, "query", "Word(s) to be removed", true))
            //     .add_option(dpp::command_option(dpp::co_string, "separator", "A single character separator between queries, defaults to a space")));

            // bot.global_command_create(dpp::slashcommand("custom_dict", "MESSAGE_PERM users only: Outputs the custom dict of the bot", bot.me.id));
        }
    });

    // ======== AUTOCOMPLETE ======== //
    bot.on_autocomplete([&bot](const dpp::autocomplete_t & event) {
        for (auto & opt : event.options) {
            if (opt.focused) {
                std::string uservalue = std::get<std::string>(opt.value);
                bot.interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply)
                    .add_autocomplete_choice(dpp::command_option_choice("QWERTY", std::string("QWERTY")))
                    .add_autocomplete_choice(dpp::command_option_choice("None", std::string("None")))
                    .add_autocomplete_choice(dpp::command_option_choice("QWERTZ", std::string("QWERTZ")))
                    .add_autocomplete_choice(dpp::command_option_choice("AZERTY", std::string("AZERTY")))
                    .add_autocomplete_choice(dpp::command_option_choice("Dvorak", std::string("Dvorak")))
                    .add_autocomplete_choice(dpp::command_option_choice("Colemak", std::string("Colemak")))
                );
                break;
            }
        }
    });

    bot.start(dpp::st_wait);

    return 0;
}