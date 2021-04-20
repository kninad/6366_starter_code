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

    glm::vec3 get_dims(model3d_t model_type);

    GLubyte* load_3d_raw_data(std::string texture_path, glm::vec3 dimension);

    GLubyte* load_3Dfrom_type(model3d_t model_type);

};

// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/Bonsai_512_512_154.raw
// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/BostonTeapot_256_256_178.raw
// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/Bucky_32_32_32.raw
// file:///home/ninad/Documents/study/sem4/cs6336_graphics/asg04/4_data_and_code/Head_256_256_225.raw




