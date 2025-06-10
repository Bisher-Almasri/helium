//
// Created by Bisher Almasri on 2025-06-10.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#include "tokenizer.hpp"

// RETURNS VECTOR OF TOKENS
std::vector<Token> tokenize(std::string &text)
{

}

// TODO: ADD PARSER

std::string tokens_to_asm(std::vector<Token> &tokens)
{
    std::stringstream output;
    // output << "global _start\n_start:\n";
    output << "global _main\n_main:\n";
    for (int i = 0; i < tokens.size(); ++i)
    {
        const Token &token = tokens.at(i);
        if (token.type == TokenType::EXIT)
        {
            if (i+1 < tokens.size() && tokens[i+1].type == TokenType::INT_LIT)
            {
                if (i+2 < tokens.size() && tokens[i+2].type == TokenType::SEMI)
                {
                    // output << "    mov rax, 60\n";
                    output << "    mov rax, 0x2000001\n";
                    output << "    mov rdi, " << tokens[i + 1].value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }

    return output.str();
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path input_path(argv[1]);
    std::string base_filename = input_path.stem();

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        if (!input.is_open()) {
            std::cerr << "Error: Could not open file " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents);

    std::string asm_filename = base_filename + ".asm";
    {
        std::fstream file(asm_filename, std::ios::out);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create " << asm_filename << std::endl;
            return EXIT_FAILURE;
        }
        file << tokens_to_asm(tokens);
    }

    std::string obj_filename = base_filename + ".o";
    const std::string& exe_filename = base_filename;

    std::string nasmcmd = "nasm -f macho64 " + asm_filename + " -o " + obj_filename;
    std::string ldcmd = "ld -platform_version macos 10.7.0 10.15.0 -syslibroot $(xcrun --show-sdk-path) " +
                        obj_filename + " -o " + exe_filename + " -lSystem";
    std::string rmcmd = "rm -f " + obj_filename + " " + asm_filename;

    std::cout << "Executing: " << nasmcmd << std::endl;
    int nasm_result = system(nasmcmd.c_str());
    if (nasm_result != 0) {
        std::cerr << "nasm failed with error " << nasm_result << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Executing: " << ldcmd << std::endl;
    int ld_result = system(ldcmd.c_str());
    if (ld_result != 0) {
        std::cerr << "ld failed with error " << ld_result << std::endl;
        return EXIT_FAILURE;
    }

    system(rmcmd.c_str());
    return EXIT_SUCCESS;
}