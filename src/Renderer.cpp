#include "Renderer.h"

Camera *Renderer::m_camera = new Camera();

nanogui::Screen *Renderer::m_nanogui_screen = nullptr;

Lighting *Renderer::m_lightings = new Lighting();

glm::vec3 CCS_lightDir = glm::vec3(0.0f, -1.0f, -1.0f);

/*
 * TODO: Deprecate these
 * 		and use Object::vao and Object::vbo instead for your loaded model
 */
// GLuint VBO, VAO;

/*
 * TODO: Deprecate these
 * 		and create global variables for your GUI
 */
// enum test_enum
// {
//     Item1 = 0,
//     Item2,
//     Item3
// };
// bool bvar = true;
// int ivar = 12345678;
// double dvar = 3.1415926;
// float fvar = (float)dvar;
// std::string strval = "A string";
// test_enum enumval = Item2;
// nanogui::Color col_val(1.0f, 1.0f, 1.0f, 1.0f);

// Pre-defined
bool Renderer::keys[1024];

enum render_type
{
    POINT = 0,
    LINE = 1,
    TRIANGLE = 2
};
enum culling_type
{
    CW = 0,
    CCW = 1
};
enum shading_type
{
    SMOOTH = 0,
    FLAT = 1,
};
enum depth_type
{
    LESS = 0,
    ALWAYS = 1,
};

// Global Vars for Nano GUI
nanogui::Color nano_col_val(0.10f, 0.4f, 0.8f, 1.0f);
std::string nano_model_name = "cube.obj"; // Deafault to Rock

float nano_campos_x = 0.0f;
float nano_campos_y = 0.0f;
float nano_campos_z = 0.0f;

float nano_rotate_val = 10.0f;
float nano_znear = 1.0f;
float nano_zfar = 1000.0f;
float nano_fov = 45.0f;

bool n_rotate_xup = false;
bool n_rotate_yup = false;
bool n_rotate_zup = false;
bool n_rotate_xdown = false;
bool n_rotate_ydown = false;
bool n_rotate_zdown = false;

render_type nano_enum_render = TRIANGLE;
culling_type nano_enum_cull = CCW;

// good init for 1st run and to ensure Renderer::is_scene_reset starts with a true
bool nano_reload_model = true;
bool nano_reset = false;

// Second Nano GUI Global Vars (Lighting)

bool n_pLight_reset = false; // Reset dynamic point light?

bool n_pLight_rotX = false;
bool n_pLight_rotY = false;
bool n_pLight_rotZ = false;

bool n_on_dirL = false;
nanogui::Color n_dirL_amb(0.05f, 0.05f, 0.05f, 1.0f);
nanogui::Color n_dirL_dif(0.05f, 0.05f, 0.05f, 1.0f);
nanogui::Color n_dirL_spc(0.05f, 0.05f, 0.05f, 1.0f);

bool n_on_posL = false;
nanogui::Color n_posL_amb(0.05f, 0.05f, 0.05f, 1.0f);
nanogui::Color n_posL_dif(0.05f, 0.05f, 0.05f, 1.0f);
nanogui::Color n_posL_spc(0.05f, 0.05f, 0.05f, 1.0f);

bool n_on_diffuseTex = false;
bool n_on_normalTex = false;

/*
 * TODO: Remeber to use these variables
 */
// render_type render_val = LINE;
// culling_type culling_val = CW;
// shading_type shading_val = SMOOTH;
// depth_type depth_val = LESS;

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::run()
{
    init();
    display(this->m_window);
}

void Renderer::init()
{
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_camera->init();

    this->m_window =
        glfwCreateWindow(m_camera->width, m_camera->height, "CS6366:Hw03Ninad", nullptr, nullptr);
    glfwMakeContextCurrent(this->m_window);

    glewExperimental = GL_TRUE;
    glewInit();

    nanogui_init(this->m_window);

    // todo: add lighting init somewhere here?
    m_lightings->init();
}

void Renderer::nanogui_init(GLFWwindow *window)
{
    m_nanogui_screen = new nanogui::Screen();
    m_nanogui_screen->initialize(window, true);

    glViewport(0, 0, m_camera->width, m_camera->height);

    bool enabled = true;

    // ********************************************************************

    // 2nd nanogui GUI
    nanogui::FormHelper *gui_2 = new nanogui::FormHelper(m_nanogui_screen);
    nanogui::ref<nanogui::Window> nanoguiWindow2 =
        gui_2->addWindow(Eigen::Vector2i(10, 10), "Nanogui control bar2: Lights");  
    
    gui_2->addVariable("Direction Light Status", n_on_dirL);

    gui_2->addVariable("DirL Ambient Color", n_dirL_amb)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.w() << "]" << std::endl;
        n_dirL_amb = c;     });

    gui_2->addVariable("DirL Diffuse Color", n_dirL_dif)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.w() << "]" << std::endl;
        n_dirL_dif = c;    });

    gui_2->addVariable("DirL Specular Color", n_dirL_spc)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.w() << "]" << std::endl;
        n_dirL_spc = c;    });

    gui_2->addVariable("Point Light Status", n_on_posL);
    
    gui_2->addVariable("PosL Ambient Color", n_posL_amb)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.w() << "]" << std::endl;
        n_posL_amb = c; });    

    gui_2->addVariable("PosL Diffuse Color", n_posL_dif)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.w() << "]" << std::endl;
        n_posL_dif = c;    });    

    gui_2->addVariable("PosL Specular Color", n_posL_spc)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.w() << "]" << std::endl;
        n_posL_spc = c;    });    

    gui_2->addVariable("Point Light Rot X", n_pLight_rotX);

    gui_2->addVariable("Point Light Rot Y", n_pLight_rotY);
    
    gui_2->addVariable("Point Light Rot Z", n_pLight_rotZ);

    gui_2->addButton("Reset Point Light", []() { n_pLight_reset = true; });

    // Textures
    gui_2->addVariable("Diffuse Texture Status", n_on_diffuseTex);

    gui_2->addVariable("Normal Map Status", n_on_normalTex);

    // ********************************************************************

    // Create nanogui gui - 1
    nanogui::FormHelper *gui = new nanogui::FormHelper(m_nanogui_screen);
    nanogui::ref<nanogui::Window> nanoguiWindow =
        gui->addWindow(Eigen::Vector2i(0, 0), "Nanogui control bar_1");

    /*
     *	TODO: Define your GUI components here
     */

    gui->addGroup("Color");
    gui->addVariable("Object Color", nano_col_val)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b()
                  << ", " << c.w() << "]" << std::endl;
        nano_col_val = c;
    });

    // personal todo: how to set the default x,y,z to centered camera position.
    // right now they are at (0,0,0) during init.
    gui->addGroup("Position");
    gui->addVariable("X", nano_campos_x)->setSpinnable(true);
    gui->addVariable("Y", nano_campos_y)->setSpinnable(true);
    gui->addVariable("Z", nano_campos_z)->setSpinnable(true);

    gui->addGroup("Rotate");
    gui->addVariable("Rotate Value", nano_rotate_val)->setSpinnable(true);
    gui->addButton("Rotate Right +", []() { n_rotate_xup = true; })
        ->setTooltip("Testing a much longer tooltip.");
    gui->addButton("Rotate Right -", []() { n_rotate_xdown = true; })
        ->setTooltip("Testing a much longer tooltip.");
    gui->addButton("Rotate Up +", []() { n_rotate_yup = true; })
        ->setTooltip("Testing a much longer tooltip.");
    gui->addButton("Rotate Up -", []() { n_rotate_ydown = true; })
        ->setTooltip("Testing a much longer tooltip.");
    gui->addButton("Rotate Front +", []() { n_rotate_zup = true; })
        ->setTooltip("Testing a much longer tooltip.");
    gui->addButton("Rotate Front -", []() { n_rotate_zdown = true; })
        ->setTooltip("Testing a much longer tooltip.");

    gui->addGroup("Configuration");
    gui->addVariable("Z Near", nano_znear)->setSpinnable(true);
    gui->addVariable("Z Far", nano_zfar)->setSpinnable(true);
    gui->addVariable("FOV", nano_fov)->setSpinnable(true);

    gui->addVariable("Model Name", nano_model_name);
    gui->addVariable("Render Type", nano_enum_render, enabled)
        ->setItems({"POINT", "LINE", "TRIANGLE"});
    gui->addVariable("Cull Type", nano_enum_cull, enabled)->setItems({"CW", "CCW"});
    gui->addButton("Reload model", []() { nano_reload_model = true; })
        ->setTooltip("Testing a much longer tooltip.");
    gui->addButton("Reset", []() { nano_reset = true; })
        ->setTooltip("Testing a much longer tooltip.");

    // ********************************************************************

    m_nanogui_screen->setVisible(true);
    m_nanogui_screen->performLayout();

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y) {
        m_nanogui_screen->cursorPosCallbackEvent(x, y);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *, int button, int action, int modifiers) {
        m_nanogui_screen->mouseButtonCallbackEvent(button, action, modifiers);
    });

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        if (key >= 0 && key < 1024)
        {
            if (action == GLFW_PRESS)
                keys[key] = true;
            else if (action == GLFW_RELEASE)
                keys[key] = false;
        }
    });

    glfwSetCharCallback(window, [](GLFWwindow *, unsigned int codepoint) {
        m_nanogui_screen->charCallbackEvent(codepoint);
    });

    glfwSetDropCallback(window, [](GLFWwindow *, int count, const char **filenames) {
        m_nanogui_screen->dropCallbackEvent(count, filenames);
    });

    glfwSetScrollCallback(window, [](GLFWwindow *, double x, double y) {
        m_nanogui_screen->scrollCallbackEvent(x, y);
        // m_camera->ProcessMouseScroll(y);
    });

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, int width, int height) {
        m_nanogui_screen->resizeCallbackEvent(width, height);
    });
}

void Renderer::display(GLFWwindow *window)
{    
    Shader m_shader = Shader("../src/shader/light_texture.vert", "../src/shader/light_texture.frag");

    // Main frame while loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call
        // corresponding response functions
        glfwPollEvents();

        /*
         * TODO: Able to reset scene and model as needed
         */
        // note, nano_reload_model is init with true for 1st run
        is_scene_reset = (nano_reload_model || nano_reset);
        if (is_scene_reset)
        {
            scene_reset();
            std::cout << "\n[DebugLog] Scene has been reset!\n";
            is_scene_reset = false; // just a safety check!
            nano_reload_model = false;
            nano_reset = false;
        }

        camera_move();

        // Textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cur_obj_ptr->diffuse_textureID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, cur_obj_ptr->normal_textureID);


        m_shader.use();

        // std::cout << "\n[DebugLog] Setting up uniform values\n";
        setup_uniform_values(m_shader);

        // std::cout << "\n[DebugLog] Drawing the scene!\n";
        draw_scene(m_shader);

        m_nanogui_screen->drawWidgets();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return;
}

void Renderer::scene_reset()
{
    load_models();
    m_camera->reset();
}

void Renderer::load_models()
{
    /*
     * TODO: Create Object class and bind VAO & VBO.
     * Here we just use show simple triangle
     * DONE?
     */

    if (cur_obj_ptr)
    {
        delete cur_obj_ptr;
    }
    std::string obj_path = "../src/objs/";
    if (nano_reload_model)
    {
        model_name = obj_path + nano_model_name;
        std::cout << "\n[DebugLog] NanoObjPath: " << obj_path << nano_model_name;
        std::cout << "\n[DebugLog] Final Model Name: \n"
                  << model_name;
    }
    cur_obj_ptr = new Object(model_name);

    // Object model(model_name);
    glGenVertexArrays(1, &(cur_obj_ptr->vao)); // public member
    glGenBuffers(1, &(cur_obj_ptr->vbo));
    glGenBuffers(1, &(cur_obj_ptr->ebo));

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute
    // pointer(s).
    glBindVertexArray(cur_obj_ptr->vao); // model.vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, cur_obj_ptr->vbo); // model.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Object::Vertex) * cur_obj_ptr->vao_vertices.size(),
                 &(cur_obj_ptr->vao_vertices[0]), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cur_obj_ptr->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * cur_obj_ptr->veo_indices.size(),
                 &(cur_obj_ptr->veo_indices[0]), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Tangent attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    
    // Bitangent attribute
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    glBindVertexArray(0); // Unbind VAO


    // Textures
    std::string texture_path = "../src/textures/";
    std::string texture_file;
    if(nano_model_name == "cyborg.obj")
    {
        texture_file = texture_path + "cyborg";
    }
    else if(nano_model_name == "cube.obj" || nano_model_name == "two_cubes.obj")
    {
        texture_file = texture_path + "cube";
    }
    std::string _diffuse_file = texture_file + "_diffuse.png";
    std::string _normal_file  = texture_file + "_normal.png";

    cur_obj_ptr->diffuse_textureID = cur_obj_ptr->loadTexture(_diffuse_file.c_str());
    cur_obj_ptr->normal_textureID  = cur_obj_ptr->loadTexture(_normal_file.c_str());

    /*
     * TODO: Set Camera parameters here
     */
    m_camera->model_center_position = cur_obj_ptr->center_cam_pos;
    // Sane default for camera position options in nano gui
    nano_campos_x = cur_obj_ptr->center_cam_pos[0];
    nano_campos_y = cur_obj_ptr->center_cam_pos[1];
    nano_campos_z = cur_obj_ptr->center_cam_pos[2];

    // Also Set the Initial Location for the Point Light
    m_lightings->point_light.position = cur_obj_ptr->center_cam_pos;
    m_lightings->set_reset_pos(cur_obj_ptr->center_cam_pos); // Save the reset position for point light

    nanogui_init(this->m_window);
}

void Renderer::draw_scene(Shader &shader)
{
    /*
     * TODO: Remember to enable GL_DEPTH_TEST and GL_CULL_FACE
     */
    glEnable(GL_DEPTH_TEST);
    glPointSize(4.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    if (nano_enum_cull == CW)
    {
        glFrontFace(GL_CW);
    }
    else if (nano_enum_cull == CCW)
    {
        glFrontFace(GL_CCW);
    }

    // Set up some basic parameters
    glClearColor(background_color[0], background_color[1], background_color[2],
                 background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /*
     * TODO: Draw object
     */
    draw_object(shader, *cur_obj_ptr);
}

void Renderer::draw_object(Shader &shader, Object &object)
{
    /*
     * TODO: Draw object
     */

    glBindVertexArray(object.vao);
    
    GLenum our_mode;
    if (nano_enum_render == LINE)
    {
        // our_mode = GL_LINES;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        our_mode = GL_TRIANGLES;
    }
    else if (nano_enum_render == TRIANGLE)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        our_mode = GL_TRIANGLES;
    }
    else if (nano_enum_render == POINT)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        our_mode = GL_POINTS;
    }
    glDrawArrays(our_mode, 0, object.vao_vertices.size());
    // glDrawElements(our_mode, object.vao_vertices.size(), GL_UNSIGNED_INT, 0);

    // Reset back Polygon Mode to GL_FILL so as to not mess up the Nano GUI.
    // Otherwise it just make everything in the GUI to be in lineframe mode.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glBindVertexArray(0); // unbind vao.
}

void Renderer::setup_uniform_values(Shader &shader)
{
    /*
     * TODO: Define uniforms for your shader
    */

    // Model matrix.
    glm::mat4 model_mat = glm::mat4(1.0f);

    // View Matrix
    glm::mat4 view = glm::mat4(1.0f);
    view = m_camera->GetViewMatrix();

    // Projection Matrix
    float aspect_ratio = (float)m_camera->width / (float)m_camera->height;
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(m_camera->zoom), aspect_ratio, nano_znear, nano_zfar);

    // glm::vec3 custom_color = glm::vec3(nano_col_val[0], nano_col_val[1], nano_col_val[2]);
    glm::vec3 custom_color = glm::vec3(1.0f,1.0f,1.0f);
    

    // Set the values of Lightings object from Nanogui Controls
    m_lightings->direction_light.direction = glm::vec3(glm::inverse(view) * glm::vec4(CCS_lightDir, 0.0f));
    // m_lightings->direction_light.direction = glm::vec3(0.0f, 0.0f, 10.0f);

    m_lightings->direction_light.ambient = Renderer::ngolor_to_glm(n_dirL_amb);
    m_lightings->direction_light.diffuse = Renderer::ngolor_to_glm(n_dirL_dif);
    m_lightings->direction_light.specular = Renderer::ngolor_to_glm(n_dirL_spc);

    m_lightings->point_light.ambient = Renderer::ngolor_to_glm(n_posL_amb);
    m_lightings->point_light.diffuse = Renderer::ngolor_to_glm(n_posL_dif);
    m_lightings->point_light.specular = Renderer::ngolor_to_glm(n_posL_spc);

    // Rotation of Point Light
    glm::vec3 pointL_pos = m_lightings->point_light.position;
    float theta = (float) glfwGetTime();
    float sin_theta = glm::sin(theta);
    float cos_theta = glm::cos(theta);
    if(n_pLight_rotX) {
        float y = pointL_pos[1];
        float z = pointL_pos[2];
        float mag = glm::sqrt(y*y + z*z);
        pointL_pos[1] = mag * cos_theta;
        pointL_pos[2] = mag * sin_theta;
        m_lightings->point_light.position = pointL_pos;
    } else if(n_pLight_rotY) {
        float x = pointL_pos[1];
        float z = pointL_pos[2];
        float mag = glm::sqrt(x*x + z*z);
        pointL_pos[0] = mag * cos_theta;
        pointL_pos[2] = mag * sin_theta;
        m_lightings->point_light.position = pointL_pos;
    } else if(n_pLight_rotZ) {
        float x = pointL_pos[0];
        float y = pointL_pos[1];
        float mag = glm::sqrt(y*y + x*x);
        pointL_pos[0] = mag * cos_theta;
        pointL_pos[1] = mag * sin_theta;
        m_lightings->point_light.position = pointL_pos;
    } else if(n_pLight_reset) {
        m_lightings->point_light.position = m_lightings->get_reset_pos();
        n_pLight_reset = false; // reset the value back to sane default.
    }

    // Set in shader program
    // Uniforms for GLSL: halfway -- ab
    unsigned int modelLoc = glGetUniformLocation(shader.program, "model_mat");
    unsigned int viewLoc = glGetUniformLocation(shader.program, "view");
    unsigned int projLoc = glGetUniformLocation(shader.program, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_mat));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int objcolLoc = glGetUniformLocation(shader.program, "object_color");
    unsigned int camposLoc = glGetUniformLocation(shader.program, "view_position");
    glUniform3fv(objcolLoc, 1, glm::value_ptr(custom_color));
    glUniform3fv(camposLoc, 1, glm::value_ptr(m_camera->position));

    // Directional Light Uniforms
    unsigned int onDLightLoc = glGetUniformLocation(shader.program, "on_Dlight");
    unsigned int dLightDirLoc = glGetUniformLocation(shader.program, "dlight_dir");
    unsigned int dLightAmbLoc = glGetUniformLocation(shader.program, "dlight_amb");
    unsigned int dLightDifLoc = glGetUniformLocation(shader.program, "dlight_dif");
    unsigned int dLightSpcLoc = glGetUniformLocation(shader.program, "dlight_spc");

    glUniform1i(onDLightLoc, n_on_dirL);
    glUniform3fv(dLightDirLoc, 1, glm::value_ptr(m_lightings->direction_light.direction));
    glUniform3fv(dLightAmbLoc, 1, glm::value_ptr(m_lightings->direction_light.ambient));
    glUniform3fv(dLightDifLoc, 1, glm::value_ptr(m_lightings->direction_light.diffuse));
    glUniform3fv(dLightSpcLoc, 1, glm::value_ptr(m_lightings->direction_light.specular));

    // Positional Light Uniforms
    unsigned int onPLightLoc = glGetUniformLocation(shader.program, "on_Plight");
    unsigned int pLightPosLoc = glGetUniformLocation(shader.program, "plight_pos");
    unsigned int pLightAmbLoc = glGetUniformLocation(shader.program, "plight_amb");
    unsigned int pLightDifLoc = glGetUniformLocation(shader.program, "plight_dif");
    unsigned int pLightSpcLoc = glGetUniformLocation(shader.program, "plight_spc");

    glUniform1i(onPLightLoc, n_on_posL);
    glUniform3fv(pLightPosLoc, 1, glm::value_ptr(m_lightings->point_light.position));
    glUniform3fv(pLightAmbLoc, 1, glm::value_ptr(m_lightings->point_light.ambient));
    glUniform3fv(pLightDifLoc, 1, glm::value_ptr(m_lightings->point_light.diffuse));
    glUniform3fv(pLightSpcLoc, 1, glm::value_ptr(m_lightings->point_light.specular));


    // Textures

    unsigned int ourTextureLoc = glGetUniformLocation(shader.program, "ourTexture");
    unsigned int normalMapLoc = glGetUniformLocation(shader.program, "normalMap");
    glUniform1i(ourTextureLoc,0);
    glUniform1i(normalMapLoc, 1);

    unsigned int on_diffuseMapLoc = glGetUniformLocation(shader.program, "on_diffuseMap");
    unsigned int on_normalMapLoc  = glGetUniformLocation(shader.program, "on_normalMap");
    glUniform1i(on_diffuseMapLoc, n_on_diffuseTex);
    glUniform1i(on_normalMapLoc, n_on_normalTex);
}

void Renderer::camera_move()
{
    GLfloat current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    // Change of camera position (x, y, z) according to NanoGui values
    glm::vec3 current_cam_pos = glm::vec3(nano_campos_x, nano_campos_y, nano_campos_z);
    m_camera->position = current_cam_pos;

    // Change camera FOV or Zoom
    // Extra part!
    if (nano_fov < 1.0f)
        nano_fov = 1.0f;
    if (nano_fov > 75.0f)
        nano_fov = 75.0f;
    m_camera->zoom = nano_fov;

    // Camera controls
    if (keys[GLFW_KEY_W])
    {
        m_camera->process_keyboard(FORWARD, delta_time);
    }
    if (keys[GLFW_KEY_S])
    {
        m_camera->process_keyboard(BACKWARD, delta_time);
    }
    if (keys[GLFW_KEY_A])
    {
        m_camera->process_keyboard(LEFT, delta_time);
    }
    if (keys[GLFW_KEY_D])
    {
        m_camera->process_keyboard(RIGHT, delta_time);
    }
    if (keys[GLFW_KEY_Q])
    {
        m_camera->process_keyboard(UP, delta_time);
    }
    if (keys[GLFW_KEY_E])
    {
        m_camera->process_keyboard(DOWN, delta_time);
    }

    // For Rotation, use the "velocity" as the nanogui rotation value
    if (keys[GLFW_KEY_I] || n_rotate_xup)
    {
        m_camera->process_keyboard(ROTATE_X_UP, nano_rotate_val);
        n_rotate_xup = false;
    }
    if (keys[GLFW_KEY_K] || n_rotate_xdown)
    {
        m_camera->process_keyboard(ROTATE_X_DOWN, nano_rotate_val);
        n_rotate_xdown = false;
    }
    if (keys[GLFW_KEY_J] || n_rotate_yup)
    {
        m_camera->process_keyboard(ROTATE_Y_UP, nano_rotate_val);
        n_rotate_yup = false;
    }
    if (keys[GLFW_KEY_L] || n_rotate_ydown)
    {
        m_camera->process_keyboard(ROTATE_Y_DOWN, nano_rotate_val);
        n_rotate_ydown = false;
    }
    if (keys[GLFW_KEY_U] || n_rotate_zup)
    {
        m_camera->process_keyboard(ROTATE_Z_UP, nano_rotate_val);
        n_rotate_zup = false;
    }
    if (keys[GLFW_KEY_O] || n_rotate_zdown)
    {
        m_camera->process_keyboard(ROTATE_Z_DOWN, nano_rotate_val);
        n_rotate_zdown = false;
    }

    // Update nanogui pos x,y,z to reflect updated postion.
    nano_campos_x = m_camera->position[0];
    nano_campos_y = m_camera->position[1];
    nano_campos_z = m_camera->position[2];
}
