#include <cstring>
#include <iostream>
#include "libsearchfile.hpp"

int main(int argc, char ** argv){

    if(argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
        std::cout << "Usage: " << argv[0] << " FILE\n"; 
        return 1;
    }
    std::string fileName;
    for(int i = 1; i < argc; i++){
        fileName.append(argv[i]);
        if(i != argc - 1)
            fileName.append(" ");
    }
    std::cout << searchFile("/", fileName, 8) << std::endl;

    return 0;
}