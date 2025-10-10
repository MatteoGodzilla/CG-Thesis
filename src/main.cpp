#include "mainUI.h"
#include "mainRenderer.h"
#include "cxxopts.hpp"
//THIS HAS TO BE THE LAST INCLUDE BECAUSE IT ALSO DEFINES THE FUNCTIONS
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main(int argc, char** argv){
    cxxopts::Options options(argv[0], "Raytracer renderer for visualizing gravitational lensing");
    options.custom_help("[Mode] [Input options] [Mode Options]");
    options.add_options("Mode")
        ("g,gui", "Show GUI editor (default)", cxxopts::value<bool>()->default_value("true"))
        ("r,render", "Render PNG image directly", cxxopts::value<bool>()->default_value("false"))
    ;
    options.add_options("Input options")
        ("i", "Universe input file", cxxopts::value<std::string>()->default_value(UNIVERSE))
        ("stdin", "Load input file from standard input", cxxopts::value<bool>()->default_value("false"))
    ;

    options.add_options("Render mode options")
        ("w", "Render width", cxxopts::value<int>())
        ("h", "Render height", cxxopts::value<int>())
        ("o", "Output png file", cxxopts::value<std::string>()->default_value("output.png"))
    ;
    options.allow_unrecognised_options();
   
    auto result = options.parse(argc, argv);

    if(result.unmatched().size() > 0){
        std::cout << options.help() << std::endl;
    } else if(argc == 1 || result["gui"].as<bool>()){
        if(result.count("stdin") == 1){
            return mainUI(std::cin, "");
        } else {
            //Attempt to load from file (default behaviour)
            std::string filename = result["i"].as<std::string>();
            std::ifstream input = std::ifstream(filename);
            if(!input.is_open()){
                std::cerr << "COULD NOT OPEN FILE" << std::endl;
                return 1;
            }
            int ret = mainUI(input, filename);
            input.close();
            return ret;
        }
    } else if(result["render"].as<bool>()){
        //Setup render
        int width;
        int height;
        std::string output;
        if(result.count("w") == 0){
            std::cerr << "Did not specify width" << std::endl;
            std::cerr << "You must both specify width and height using -w and -h when in render mode" << std::endl;
            return 1;
        }
        width = result["w"].as<int>();

        if(result.count("h") == 0){
            std::cerr << "Did not specify height" << std::endl;
            std::cerr << "You must both specify width and height using -w and -h when in render mode" << std::endl;
            return 1;
        }
        height = result["h"].as<int>();

        if(result.count("o") == 1){
            output = result["o"].as<std::string>();
        } else {
            output = "output.png";
        }
       
        if(result.count("stdin") == 1){
            return mainRenderer(width, height, std::cin, output);
        } else {
            //Attempt to load from file (default behaviour)
            std::ifstream input = std::ifstream(result["i"].as<std::string>());
            if(!input.is_open()){
                std::cerr << "COULD NOT OPEN FILE" << std::endl;
                return 1;
            }
            int ret = mainRenderer(width, height, input, output);
            input.close();
            return ret;
        }
    }
}
