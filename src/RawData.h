#pragma once

#include <iostream>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace RawDataUtil {
 
    enum model3d_t {
        BUCKY,
        TEAPOT,
        BONSAI,
        HEAD,
    };

    const std::string base_path = "../src/data/";

    const std::string colorbar_path = base_path + "colorbar.png";

    GLubyte* load_3d_raw_data(std::string texture_path, glm::vec3 dimension)
    {
        size_t size = dimension[0] * dimension[1] * dimension[2];

        FILE *fp;
        GLubyte *data = new GLubyte[size];			  // 8bit
        if (!(fp = fopen(texture_path.c_str(), "rb"))) {
            std::cout << "Error: opening .raw file failed" << std::endl;
            exit(EXIT_FAILURE);
        }
        else {
            std::cout << "OK: open .raw file successed" << std::endl;
        }
        if (fread(data, sizeof(char), size, fp) != size) {
            std::cout << "Error: read .raw file failed" << std::endl;
            exit(1);
        }
        else {
            std::cout << "OK: read .raw file successed" << std::endl;
        }
        fclose(fp);
        return data;
    }

    GLubyte* load_3Dfrom_type(model3d_t model_type)
    {
        GLubyte* data = nullptr;
        if (model_type == BUCKY)
        {
            data = load_3d_raw_data(base_path + "Bucky_32_32_32.raw", glm::vec3(32, 32, 32));
        } 
        else if(model_type == BONSAI)
        {
            data = load_3d_raw_data(base_path + "Bonsai_512_512_154.raw", glm::vec3(512, 512, 154));
        }
        else if(model_type == TEAPOT)
        {
            data = load_3d_raw_data(base_path + "BostonTeapot_256_256_178.raw", glm::vec3(256, 256, 178));
        }
        else if(model_type == HEAD)
        {
            data = load_3d_raw_data(base_path + "Head_256_256_225.raw", glm::vec3(256, 256, 225));
        }

        // check for errors
        if(!data){
            std::cout << "Error in reading raw data! data pointer is currently set to null." << std::endl;
        }
        return data;
    }


    // Cube with edge data to render the 3D Texture on.
    GLfloat cube_vertices[24] = {
        0.0, 0.0, 0.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 1.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 1.0
    };

    GLuint cube_indices[36] = {
        1,5,7,
        7,3,1,
        0,2,6,
        6,4,0,
        0,1,3,
        3,2,0,
        7,5,4,
        4,6,7,
        2,3,7,
        7,6,2,
        1,0,4,
        4,5,1
    };

    GLuint cube_edges[24]{
        1,5,
        5,7,
        7,3,
        3,1,
        0,4,
        4,6,
        6,2,
        2,0,
        0,1,
        2,3,
        4,5,
        6,7
    };


};

// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/Bonsai_512_512_154.raw
// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/BostonTeapot_256_256_178.raw
// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/Bucky_32_32_32.raw
// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/Head_256_256_225.raw




