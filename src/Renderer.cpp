#include "Renderer.h"


Camera *Renderer::m_camera = new Camera();

nanogui::Screen *Renderer::m_nanogui_screen = nullptr;


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
RawDataUtil::model3d_t nano_3dmodel = RawDataUtil::TEAPOT;

nanogui::Color nano_col_val(0.10f, 0.4f, 0.8f, 1.0f);
bool nano_transfer_func_sign = false;
int nano_sampling_rate = 100;

// good init for 1st run and to ensure Renderer::is_scene_reset starts with a true
bool nano_reload_model = true;
bool nano_reset = false;

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
        glfwCreateWindow(m_camera->width, m_camera->height, "CS6366:Hw4-Ninad", nullptr, nullptr);
    glfwMakeContextCurrent(this->m_window);

    glewExperimental = GL_TRUE;
    glewInit();

    nanogui_init(this->m_window);

    //add lighting init somewhere here?
    // m_lightings->init();
}

void Renderer::nanogui_init(GLFWwindow *window)
{
    m_nanogui_screen = new nanogui::Screen();
    m_nanogui_screen->initialize(window, true);

    glViewport(0, 0, m_camera->width, m_camera->height);

    bool enabled = true;

    // Create nanogui gui - 1
    nanogui::FormHelper *gui = new nanogui::FormHelper(m_nanogui_screen);
    nanogui::ref<nanogui::Window> nanoguiWindow =
        gui->addWindow(Eigen::Vector2i(0, 0), "Nanogui control bar_1");

    /* ************************************************* */
    // GUI Components        
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

    // gui->addVariable("Model Name", nano_model_name);
    gui->addVariable("Model Name", nano_3dmodel, enabled)
        ->setItems({"BUCKY", "TEAPOT", "BONSAI", "HEAD"});
    
    gui->addVariable("Render Type", nano_enum_render, enabled)
        ->setItems({"POINT", "LINE", "TRIANGLE"});
    
    gui->addVariable("Cull Type", nano_enum_cull, enabled)->setItems({"CW", "CCW"});
    
    gui->addButton("Reload model", []() { nano_reload_model = true; })
        ->setTooltip("Testing a much longer tooltip.");
    
    gui->addButton("Reset Camera", []() { nano_reset = true; })
        ->setTooltip("Testing a much longer tooltip.");

    
    gui->addGroup("Volume Rendering");    
    
    gui->addVariable("Object Color", nano_col_val)->setFinalCallback([](const nanogui::Color &c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b()<< ", " << c.w() << "]" << std::endl;
        nano_col_val = c;
    });
    gui->addVariable("Transfer Function Sign", nano_transfer_func_sign);
    gui->addVariable("Sampling Rate", nano_sampling_rate);  

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
    Shader m_shader = Shader("../src/shader/volrender.vert", "../src/shader/volrender.frag");
 
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
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_3D, cur_obj_ptr->tex3dID);

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
    */

    if (cur_obj_ptr)
    {
        delete cur_obj_ptr;
    }

    cur_obj_ptr = new Object(); // default object from given cube data 

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    // Texture (3D) attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Unbind VAO
    glBindVertexArray(0);
    
    // Texture Loading 
    // cur_obj_ptr->tex3dID = cur_obj_ptr->load3dTexture(nano_3dmodel);

    /*
     * Set Camera parameters here
     */
    m_camera->model_center_position = cur_obj_ptr->center_cam_pos;
    std::cout << "[DEBUG] Camera Position: ";
    cur_obj_ptr->print_glmvec3(cur_obj_ptr->center_cam_pos) ;

    // Sane default for camera position options in nano gui
    nano_campos_x = cur_obj_ptr->center_cam_pos[0];
    nano_campos_y = cur_obj_ptr->center_cam_pos[1];
    nano_campos_z = cur_obj_ptr->center_cam_pos[2];

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
    // glDrawArrays(our_mode, 0, object.vao_vertices.size());    
    glDrawElements(our_mode, 36, GL_UNSIGNED_INT, 0);

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

    glm::vec3 custom_color = glm::vec3(nano_col_val[0], nano_col_val[1], nano_col_val[2]);
    // glm::vec3 custom_color = glm::vec3(1.0f, 1.0f, 1.0f);

    // Set in shader program
    // Uniforms for GLSL: halfway -- ab
    unsigned int modelLoc = glGetUniformLocation(shader.program, "model_mat");
    unsigned int viewLoc = glGetUniformLocation(shader.program, "view");
    unsigned int projLoc = glGetUniformLocation(shader.program, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_mat));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int objcolLoc = glGetUniformLocation(shader.program, "object_color");
    glUniform3fv(objcolLoc, 1, glm::value_ptr(custom_color));
    // unsigned int camposLoc = glGetUniformLocation(shader.program, "view_position");
    // glUniform3fv(camposLoc, 1, glm::value_ptr(m_camera->position));

    // Textures
    // unsigned int ourTextureLoc = glGetUniformLocation(shader.program, "ourTexture");
    // unsigned int normalMapLoc = glGetUniformLocation(shader.program, "normalMap");
    // glUniform1i(ourTextureLoc, 0);
    // glUniform1i(normalMapLoc, 1);

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
