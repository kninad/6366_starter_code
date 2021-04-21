#include "Renderer.h"
#include <cmath>
#include <unordered_set>

const float EPSILON = 1e-6;

Camera *Renderer::m_camera = new Camera();

nanogui::Screen *Renderer::m_nanogui_screen = nullptr;

bool k_show = true;

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
std::string nano_model_name = "mycube.obj"; // Deafault to Rock

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

/*
 * TODO: Remeber to use these variables
 */
// render_type render_val = LINE;
// culling_type culling_val = CW;
// shading_type shading_val = SMOOTH;
// depth_type depth_val = LESS;

GLfloat cube_vertices[24] = {
    0.0, 0.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 1.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 1.0};

GLuint cube_edges[24] = {
    1, 5,
    5, 7,
    7, 3,
    3, 1,
    0, 4,
    4, 6,
    6, 2,
    2, 0,
    0, 1,
    2, 3,
    4, 5,
    6, 7};

// Returns the coefficient in the line equation. For us (in cube), it has to be bounded
// between 0 and 1 (inclusive). So if its out of range, we return -1.0.
float get_intersection_coef(glm::vec3 plane_point, glm::vec3 normal, glm::vec3 line_point, glm::vec3 line_dir)
{
    // Plane Line Intersection
    // Plane: (point - plane_point) * n = 0
    // Line: point = line_point + (lambda) line_dir.
    // Need to find lambda (float)
    float numerator = glm::dot((plane_point - line_point), normal);
    float denom = glm::dot(line_dir, normal);
    float coef;
    if (abs(denom) < EPSILON)
    {
        // parallel plane and line
        if (abs(numerator) < EPSILON)
        {
            coef = 0.0f;
        }
    }
    else
    {
        coef = numerator / denom;
        if(coef < 0.0f || coef > 1.0f) // out of range!
        {
            coef = -1.0f;
        }
    }

    return coef;
}

// Returns the center point for a polygon
glm::vec3 center_point(const std::vector<glm::vec3>& point_list)
{
    glm::vec3 avg(0,0,0);
    for(const auto& p : point_list)
    {
        avg += p;
    }
    avg /= point_list.size();
    return avg;
}

// Hacky and non optimal way to do sorting O(n^2)-- bUT n = 6 for us.
std::vector<glm::vec3> sort_points(const std::vector<glm::vec3>& point_list, const std::vector<float>& angles)
{
    std::unordered_set<int> seen_idxs;
    std::vector<int> ordered_idxs;
    std::vector<glm::vec3> ordered_points;
    for(int j = 0; j < point_list.size(); j++)
    {
        int min_idx;
        float min_val = INT_MAX;
        for(int i = 0; i < point_list.size(); i++)
        {
            if(seen_idxs.count(i) == 0)
            {
                if(angles[i] < min_val)
                {
                    min_val = angles[i];
                    min_idx = i;
                }
            }
        }
        seen_idxs.insert(min_idx);
        ordered_idxs.push_back(min_idx);
    }

    for(const auto& v : ordered_idxs)
    {
        ordered_points.push_back(point_list[v]);
    }
    return ordered_points;
}

// Returns a new vector of points in sorted order wrt center
std::vector<glm::vec3> get_ordered_points(const std::vector<glm::vec3>& point_list, const glm::vec3& center)
{
    std::vector<float> cos_angles;
    glm::vec3 fixed_vec (1.0, 0.0, 0.0); // For reference angle (X - axis)
    for(const auto& p : point_list)
    {
        float dot_prod = glm::dot(fixed_vec, glm::normalize(p - center));
        cos_angles.push_back(dot_prod);
    }
    // Now Sort and get ordered points.
    return sort_points(point_list, cos_angles);
}

bool is_valid_point(const glm::vec3& point)
{   
    // modern c++ lambda expression
    auto valid_value = [](float value) { return (value >= 0.0f) && (value <= 1.0f); };
    return (valid_value(point.x)) && (valid_value(point.y)) && (valid_value(point.z));
}


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
        glfwCreateWindow(m_camera->width, m_camera->height, "CS6366:Hw01Ninad", nullptr, nullptr);
    glfwMakeContextCurrent(this->m_window);

    glewExperimental = GL_TRUE;
    glewInit();

    nanogui_init(this->m_window);
}

void Renderer::nanogui_init(GLFWwindow *window)
{
    m_nanogui_screen = new nanogui::Screen();
    m_nanogui_screen->initialize(window, true);

    glViewport(0, 0, m_camera->width, m_camera->height);

    // Create nanogui gui
    nanogui::FormHelper *gui = new nanogui::FormHelper(m_nanogui_screen);
    nanogui::ref<nanogui::Window> nanoguiWindow =
        gui->addWindow(Eigen::Vector2i(0, 0), "Nanogui control bar_1");

    /*
     *	TODO: Define your GUI components here
     */
    bool enabled = true;

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
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.w() << "]" << std::endl;
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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, cur_obj_ptr->texture3dID);

        m_shader.use();

        // std::cout << "\n[DebugLog] Setting up uniform values\n";
        setup_uniform_values(m_shader);

        // Perform View Slicing and populate the vao.
        view_slicing();

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
    std::string obj_path = "../src/objs/";
    if (nano_reload_model)
    {
        model_name = obj_path + nano_model_name;
        std::cout << "\n[DebugLog] NanoObjPath: " << obj_path << nano_model_name;
        std::cout << "\n[DebugLog] Final Model Name: \n"
                  << model_name;
    }

    cur_obj_ptr = new Object(model_name);
    const int max_slices = 3000;
    glm::vec3 tmp_buffer[max_slices * 6]; // max 6 points per slice.

    glGenVertexArrays(1, &(cur_obj_ptr->vao)); // public member
    glGenBuffers(1, &(cur_obj_ptr->vbo));
    glGenBuffers(1, &(cur_obj_ptr->ebo));

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute
    // pointer(s).
    glBindVertexArray(cur_obj_ptr->vao); // model.vao);

    glBindBuffer(GL_ARRAY_BUFFER, cur_obj_ptr->vbo); // model.vbo);
    // Fill glBufferData with zeros initially with a buffer for max 5000 glm::vec3 points
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp_buffer), 0, GL_DYNAMIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    // Texture coords calculated on the fly in vertex shader
    glBindVertexArray(0); // Unbind VAO

    // Texture Loading
    cur_obj_ptr->texture3dID = cur_obj_ptr->load3dTexture(nano_3dmodel);

    /*
     * TODO: You can also set Camera parameters here
     */
    m_camera->model_center_position = cur_obj_ptr->center_cam_pos;
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
    // glEnable(GL_DEPTH_TEST);
    glPointSize(4.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // glEnable(GL_ALPHA_TEST);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    glDisable(GL_BLEND);
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

    glDrawArrays(our_mode, 0, object.vao_points.size());

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
    // glm::vec3 scale_down = glm::vec3(0.5f, 0.5f, 0.5f);
    // model_mat = glm::scale(model_mat, scale_down);

    // View Matrix
    glm::mat4 view = glm::mat4(1.0f);
    view = m_camera->GetViewMatrix();

    // Projection Matrix
    float aspect_ratio = (float)m_camera->width / (float)m_camera->height;
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(m_camera->zoom), aspect_ratio, nano_znear, nano_zfar);

    // glm::vec4 custom_color = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f); // placeholder value for now!
    glm::vec4 custom_color =
        glm::vec4(nano_col_val[0], nano_col_val[1], nano_col_val[2], nano_col_val[3]);

    // Set in shader program
    unsigned int colLoc = glGetUniformLocation(shader.program, "custom_color");
    unsigned int modelLoc = glGetUniformLocation(shader.program, "model_mat");
    unsigned int viewLoc = glGetUniformLocation(shader.program, "view");
    unsigned int projLoc = glGetUniformLocation(shader.program, "projection");

    glUniform4fv(colLoc, 1, glm::value_ptr(custom_color));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_mat));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
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

void Renderer::view_slicing()
{
    // View Matrix
    glm::mat4 view = glm::mat4(1.0f);
    view = m_camera->GetViewMatrix();
    // float _viewdirX, _viewdirY, _viewdirZ;
    // _viewdirX = view[2][0];
    glm::vec3 viewDir(view[2][0], view[2][1], view[2][2]);
    glm::vec3 camPosn = m_camera->position;

    auto vertexList = cur_obj_ptr->ori_positions;
    float max_dist = glm::dot(viewDir, vertexList[0]);
    float min_dist = max_dist;
    int max_index = 0;
    int min_index = 0;
    for (int i = 1; i < 8; i++)
    {
        float dist = glm::dot(viewDir, vertexList[i]);
        if (dist > max_dist)
        {
            max_dist = dist;
            max_index = i;
        }
        if (dist < min_dist)
        {
            min_dist = dist;
            min_index = i;
        }
    }    
    min_dist -= EPSILON;
    max_dist += EPSILON;
    float delta = (max_dist - min_dist) / nano_sampling_rate;
    float start_lamb = min_dist;

    // Max 6 intersection points per slicing plane
    // Store tesselated triangle vertices in the vector
    std::vector<glm::vec3> slice_vao_vertices;
    for (int i = 0; i < nano_sampling_rate; i++)
    {   
        std::vector<glm::vec3> intersection_points;
        glm::vec3 plane_point = camPosn + (start_lamb + i * delta) * viewDir;
        // Check intersection with all edges
        for(int j = 0; j < 12; j++)
        {
            glm::vec3 edge_point = cur_obj_ptr->edges_parametric[j][0];
            glm::vec3 edge_direc = cur_obj_ptr->edges_parametric[j][1];
            float coef = get_intersection_coef(plane_point, viewDir, edge_point, edge_direc);
            if(coef >= -1.0f * EPSILON) // just to be safe to include 0.0f as well
            {
                glm::vec3 tmp_point;
                if(abs(coef - 0.0f) < EPSILON){
                    tmp_point = edge_point;
                }
                else
                {
                    tmp_point = edge_point + coef * edge_direc;
                }
                if(!is_valid_point(tmp_point))
                {   
                    std::cout << "  [DEBUG] point out of bounds!" << std::endl;
                    cur_obj_ptr->print_glmvec3(tmp_point);
                    exit(1);
                }
                // else
                // {
                //     cur_obj_ptr->print_glmvec3(tmp_point);
                // }
                intersection_points.push_back(tmp_point);
            }
        }
        // Obtained a polygon in intersection_points
        // Sort in counter clockwise order: order wrt angle with x-axis. 
        auto center = center_point(intersection_points);
        auto sorted_pts = get_ordered_points(intersection_points, center);
        // auto sorted_pts = intersection_points;
        // Tesselate it and store in the vector for VAO points.        
        for(int i = 0; i < sorted_pts.size(); i++)
        {
            // if(k_show)
            // {
            //     std::cout << "face: ";
            //     cur_obj_ptr->print_glmvec3(sorted_pts[i]);
            //     slice_vao_vertices.push_back(center);
            // }

            // p(i), center, p(i+1)
            slice_vao_vertices.push_back(sorted_pts[i]);
            slice_vao_vertices.push_back(center);
            if(i == (sorted_pts.size() - 1))
            {
                slice_vao_vertices.push_back(sorted_pts[0]);
            }
            else
            {
                slice_vao_vertices.push_back(sorted_pts[i + 1]);
            }
        }
    }
    // std::cout << "[DEBUG] Slicing VAO vertices size: " << slice_vao_vertices.size() << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, cur_obj_ptr->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * slice_vao_vertices.size(), &(slice_vao_vertices.front()));
    k_show = nano_transfer_func_sign;
}







std::vector<glm::vec3> get_vertices_simple(float zval)
{
    std::vector<glm::vec3> verts;
    // glm::vec3 p1(0.0, 0.0, zval);
    // glm::vec3 p2(0.0, 1.0, zval);
    // glm::vec3 p3(1.0, 1.0, zval);
    // glm::vec3 p4(1.0, 0.0, zval);
    // glm::vec3 p5(0.5, 0.5, zval);

    verts.push_back(glm::vec3(0.0, 0.0, zval));
    verts.push_back(glm::vec3(0.0, 1.0, zval));
    verts.push_back(glm::vec3(1.0, 1.0, zval));
    verts.push_back(glm::vec3(1.0, 0.0, zval));
    verts.push_back(glm::vec3(0.5, 0.5, zval));

    // 12 vertices total for the 4 triangles
    std::vector<glm::vec3> face_vert;

    for (int i = 3; i >= 0; i--)
    {
        int tmp_idx;
        if (i == 0)
        {
            tmp_idx = 3;
        }
        else
        {
            tmp_idx = i - 1;
        }
        face_vert.push_back(verts[4]);
        face_vert.push_back(verts[tmp_idx]);
        face_vert.push_back(verts[i]);
    }
    return face_vert;
}

void Renderer::simple_slice()
{
    std::vector<glm::vec3> vertSlices;
    int num_samples = nano_sampling_rate;
    float delta = 1.0f / num_samples;

    for (int i = num_samples; i > 0; i--)
    {
        float zval = 0.0f + i * delta;
        std::vector<glm::vec3> tmp = get_vertices_simple(zval);
        vertSlices.insert(vertSlices.end(), tmp.begin(), tmp.end());
    }
    // std::cout << "vertSlice size: " << vertSlices.size() << std::endl;
    cur_obj_ptr->vao_points = vertSlices;

    glBindBuffer(GL_ARRAY_BUFFER, cur_obj_ptr->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertSlices.size(), &(vertSlices[0].x));
}
