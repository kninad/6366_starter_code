#pragma once

#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// NOTE: do not define STB_IMAGE_IMPLEMENTATION
// 	     it would conflict with nanovg.c (libnanogui.a)
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Camera.h"
#include "Lighting.h"
#include "Object.h"
#include "Shader.h"

class Renderer
{
   public:
    GLFWwindow* m_window;

    static Camera* m_camera;
    // static Lighting* m_lightings;

    static nanogui::Screen* m_nanogui_screen;

    // std::vector<Object> obj_list;

    // Pointer to the current object?
    Object* cur_obj_ptr = nullptr;

    glm::vec4 background_color = glm::vec4(0.0, 0.0, 0.0, 0.0);

    bool is_scene_reset = true;

    /*
     * TODO: Define model name here.
     *
     * You can also import CLI11 (https://github.com/CLIUtils/CLI11)
     * to load model from command line
     */
    std::string model_name = "../src/objs/rock.obj"; // Default 3D object

    GLfloat delta_time = 0.0;
    GLfloat last_frame = 0.0;

    static bool keys[1024];

    float z_near = 1.0f;
    float z_far = 3000.0f;

   public:
    Renderer();

    ~Renderer();

    void run();

   private:

    // Size in bytes occupied by vector data
    // https://stackoverflow.com/a/17254518/9579260
    template<typename T>
    unsigned long vecdata_size(const typename std::vector<T>& vec){
        return sizeof(T) * vec.size();
    }


    void init();

    void nanogui_init(GLFWwindow* window);

    void display(GLFWwindow* window);

    void load_models();

    void draw_scene(Shader& shader);

    void camera_move();

    void draw_object(Shader& shader, Object& object);

    void bind_vaovbo(Object& cur_obj);

    void setup_uniform_values(Shader& shader);

    void scene_reset();

    void view_slicing_simple();

    void _view_slicing_simple();
};
