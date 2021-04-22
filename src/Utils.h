#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Utils
{

    enum model3d_t
    {
        BUCKY,
        TEAPOT,
        BONSAI,
        HEAD,
    };

    glm::vec3 get_dims(model3d_t model_type);

    GLubyte *load_3d_raw_data(std::string texture_path, glm::vec3 dimension);

    GLubyte *load_3Dfrom_type(model3d_t model_type);

    void print_glmvec3(const glm::vec3 vec);

    // Returns the coefficient in the line equation. For us (in cube), it has to be bounded
    // between 0 and 1 (inclusive). So if its out of range, we return -1.0.
    float get_intersection_coef(glm::vec3 plane_point, glm::vec3 normal, glm::vec3 line_point, glm::vec3 line_dir);

    // Returns the center point for a polygon
    glm::vec3 center_point(const std::vector<glm::vec3> &point_list);

    // Hacky and non optimal way to do sorting O(n^2)-- bUT n = 6 for us.
    std::vector<glm::vec3> sort_points(const std::vector<glm::vec3> &point_list, const std::vector<float> &angles);

    // Returns a new vector of points in sorted order wrt center
    std::vector<glm::vec3> get_ordered_points(const std::vector<glm::vec3> &point_list, const glm::vec3 &center);

    bool is_valid_point(const glm::vec3 &point);

    glm::vec3 transform_pos(const glm::mat4 &modelViewmat, const glm::vec3 &pos);
   
    glm::vec3 transform_dir(const glm::mat4 &modelViewmat, const glm::vec3 &dir);

    std::vector<glm::vec3> transform_cube_vertices(const glm::mat4 &modelViewmat,
                                                   const std::vector<glm::vec3> &points);

};

// // Bonsai_512_512_154.raw
// // BostonTeapot_256_256_178.raw
// // Bucky_32_32_32.raw
// // Head_256_256_225.raw
