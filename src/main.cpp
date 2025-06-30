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

int main(int argc, char* argv[])
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
    std::optional<NodeProg> tree = parser.parseProgram();

    if (!tree.has_value())
    {
        std::cerr << "Error: No exit statement found" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());

    std::string ir_filename = base_filename + ".ll";
    {
        std::fstream file(ir_filename, std::ios::out);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not create " << ir_filename << std::endl;
            return EXIT_FAILURE;
        }
        file << generator.genProgram();
    }

    const std::string& exe_filename = base_filename;

    std::string clang_cmd = "clang -o " + exe_filename + " " + ir_filename;
    // std::string rm_cmd = "rm -f " + ir_filename;

    std::cout << "Executing: " << clang_cmd << std::endl;
    if (int clang_result = system(clang_cmd.c_str()); clang_result != 0)
    {
        std::cerr << "clang failed with error " << clang_result << std::endl;
        return EXIT_FAILURE;
    }

    // system(rm_cmd.c_str());
    return EXIT_SUCCESS;
}