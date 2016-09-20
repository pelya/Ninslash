//#include "GLShader.hpp"
#include <GL/glew.h>
#include "shaders.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include <engine/console.h>

std::string readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        //std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        dbg_msg("gfx", "couldn't load the shader file %s", filePath);
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}


#if defined(GL_ES_VERSION_3_0)
static const char *SHADER_VER_PREFIX = "/gles3";
#elif defined(GL_ES_VERSION_2_0)
static const char *SHADER_VER_PREFIX = "/gles2";
#else
static const char *SHADER_VER_PREFIX = "";
#endif

GLuint LoadShader(const char *vertex_path_in, const char *fragment_path_in) {

    std::string vertex_path_s(vertex_path_in), fragment_path_s(fragment_path_in);
    std::string vertex_path_out = vertex_path_s.substr(0, vertex_path_s.rfind("/")) + SHADER_VER_PREFIX + vertex_path_s.substr(vertex_path_s.rfind("/"));
    std::string fragment_path_out = fragment_path_s.substr(0, fragment_path_s.rfind("/")) + SHADER_VER_PREFIX + fragment_path_s.substr(fragment_path_s.rfind("/"));
    const char *vertex_path = vertex_path_out.c_str();
    const char *fragment_path = fragment_path_out.c_str();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Read shaders
    std::string vertShaderStr = readFile(vertex_path);
    std::string fragShaderStr = readFile(fragment_path);
    const char *vertShaderSrc = vertShaderStr.c_str();
    const char *fragShaderSrc = fragShaderStr.c_str();

    GLint result = GL_FALSE;
    int logLength;

    // Compile vertex shader
    dbg_msg("gfx", "Loading vertex shader %s", vertex_path);
    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    // Check vertex shader
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
    if (!result)
    {
        dbg_msg("gfx", "Could not compile vertex shader %s", vertex_path);
        char *infoLog = (char *) malloc ( sizeof ( char ) * logLength );
        glGetShaderInfoLog(vertShader, logLength, NULL, infoLog);
        dbg_msg("gfx", "%s", infoLog);
        free ( infoLog );
        dbg_msg("gfx", "%s", vertShaderSrc);
        return 0;
    }

    // Compile fragment shader
    dbg_msg("gfx", "Loading fragment shader %s", fragment_path);
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Check fragment shader
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
    if (!result)
    {
        dbg_msg("gfx", "Could not compile fragment shader %s", fragment_path);
        char *infoLog = (char *) malloc ( sizeof ( char ) * logLength );
        glGetShaderInfoLog(fragShader, logLength, NULL, infoLog);
        dbg_msg("gfx", "%s", infoLog);
        free ( infoLog );
        dbg_msg("gfx", "%s", fragShaderSrc);
        return 0;
    }

    dbg_msg("gfx", "Linking program");
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (!result)
    {
        dbg_msg("gfx", "Could not link shaders %s %s", vertex_path, fragment_path);
        char *infoLog = (char *) malloc ( sizeof ( char ) * logLength );
        glGetShaderInfoLog(program, logLength, NULL, infoLog);
        dbg_msg("gfx", "%s", infoLog);
        free ( infoLog );
        return 0;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}
