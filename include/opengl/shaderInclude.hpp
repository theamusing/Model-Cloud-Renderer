#ifndef SHADER_INCLUDE_H
#define SHADER_INCLUDE_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

class ShaderInclude
{
public:
    static void include(std::string path, std::string content)
    {
        std::string basePath = path.substr(0, path.find_last_of("/"));
        size_t pos = content.find("#include");
        while (pos != std::string::npos)
        {
            size_t start = content.find("\"", pos);
            size_t end = content.find("\"", start + 1);
            std::string includeFile = content.substr(start + 1, end - start - 1);
            std::string includePath = basePath + includeFile;
            if (shaderIncludes.find(includePath) == shaderIncludes.end())
            {
                std::ifstream file(includePath);
                if (file.is_open())
                {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    shaderIncludes[includePath] = buffer.str();
                    // recursively include other includes
                    include(includePath, shaderIncludes[includePath]);

                    // add the include to the GL_SHADER_INCLUDE_ARB
                    glNamedStringARB(GL_SHADER_INCLUDE_ARB, includeFile.size(), includeFile.c_str(), shaderIncludes[includePath].size(), shaderIncludes[includePath].c_str());
                    std::cout << "Included: " << includePath << std::endl;
                }
                else
                {
                    std::cout << "ERROR::SHADER::INCLUDE_NOT_FOUND: " << includePath << std::endl;
                }
            }

            pos = content.find("#include", pos + 1);
        }
    }
private:
    static std::unordered_map<std::string, std::string> shaderIncludes;
};

std::unordered_map<std::string, std::string> ShaderInclude::shaderIncludes;

#endif