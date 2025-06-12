//
// Created by Bisher Almasri on 2025-06-10.
//

#include "generation.hpp"
#include "parser.hpp"
#include "tokenization.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

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
        if (!input.is_open())
        {
            std::cerr << "Error: Could not open file " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));

    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(std::move(tokens));
    std::optional<NodeExit> tree = parser.parse();

    if (!tree.has_value())
    {
        std::cerr << "Error: No exit statement found" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());


    std::string asm_filename = base_filename + ".asm";
    {
        std::fstream file(asm_filename, std::ios::out);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not create " << asm_filename << std::endl;
            return EXIT_FAILURE;
        }
        file << generator.generate();
    }

    std::string obj_filename = base_filename + ".o";
    const std::string &exe_filename = base_filename;

    std::string nasmcmd = "nasm -felf64 " + asm_filename + " -o " + obj_filename;
    std::string ldcmd = "ld " + obj_filename + " -o " + exe_filename;
    std::string rmcmd = "rm -f " + obj_filename + " " + asm_filename;
    std::cout << "Executing: " << nasmcmd << std::endl;
    int nasm_result = system(nasmcmd.c_str());
    if (nasm_result != 0)
    {
        std::cerr << "nasm failed with error " << nasm_result << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Executing: " << ldcmd << std::endl;
    int ld_result = system(ldcmd.c_str());
    if (ld_result != 0)
    {
        std::cerr << "ld failed with error " << ld_result << std::endl;
        return EXIT_FAILURE;
    }

    // system(rmcmd.c_str());
    return EXIT_SUCCESS;
}
