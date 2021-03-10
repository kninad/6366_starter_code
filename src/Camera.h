#pragma once

#include <GL/glew.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    ROTATE_X_UP,
    ROTATE_X_DOWN,
    ROTATE_Y_UP,
    ROTATE_Y_DOWN,
    ROTATE_Z_UP,
    ROTATE_Z_DOWN,
};

// Default camera values. Adopted from learnopengl.com Camera class.

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float ROLL = 0.0f;
const float SPEED = 2.0f;
const float ZOOM = 45.0f;  // Default FOV value.

// 4:3 aspect ratio
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

class Camera
{
   public:
    /*
     * TODO: Define Camera parameters
     */
    glm::vec3 model_center_position;
    glm::vec3 position, front, up, world_up;
    glm::vec3 right;  // defined using front and up vectors (cross product)
    float yaw, pitch, roll;
    float zoom;  // field of view (fov)

    unsigned int width;
    unsigned int height;

    /*
     * TODO: Initialize Camera parameters
     */
    Camera(glm::vec3 position_ = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 w_up_ = glm::vec3(0.0f, 1.0f, 0.0f), float yaw_ = YAW, float pitch_ = PITCH,
           float roll_ = ROLL, unsigned int width_ = SCR_WIDTH, unsigned int height_ = SCR_HEIGHT)
        : front(glm::vec3(0.0f, 0.0f, -1.0f)), zoom(ZOOM)
    {
        position = position_;
        world_up = w_up_;
        width = width_;
        height = height_;
        set_camera_params();
    }

    void init() { reset(); };

    /*
     * TODO: Implement camera reset
     * Reset the camera to its original position (i.e., object centered inside the window)
    */
    void reset()
    {
        position = model_center_position;
        front = glm::vec3(0.0f, 0.0f, -1.0f);
        world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        yaw = YAW;
        pitch = PITCH;
        roll = ROLL;
        width = SCR_WIDTH;
        height = SCR_HEIGHT;
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
        zoom = ZOOM;
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    // learnopengl.com
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    /*
     * TODO: Implement each movement
     */
    void process_keyboard(Camera_Movement direction, GLfloat velocity)
    {
        // float rotation_sensitivity = 0.3f;
        if (direction == FORWARD)
        {
            position += front * velocity;
        }
        if (direction == BACKWARD)
        {
            position -= front * velocity;
        }
        if (direction == LEFT)
        {
            position -= glm::normalize(glm::cross(front, up)) * velocity;
        }
        if (direction == RIGHT)
        {
            position += glm::normalize(glm::cross(front, up)) * velocity;
        }
        if (direction == UP)
        {
            position += up * velocity;
        }
        if (direction == DOWN)
        {
            position -= up * velocity;
        }
        if (direction == ROTATE_X_UP)
        {
            pitch += velocity;
        }
        if (direction == ROTATE_X_DOWN)
        {
            pitch -= velocity;
        }
        if (direction == ROTATE_Y_UP)
        {
            yaw += velocity;
        }
        if (direction == ROTATE_Y_DOWN)
        {
            yaw -= velocity;
        }
        if (direction == ROTATE_Z_UP)
        {
            roll += velocity;
        }
        if (direction == ROTATE_Z_DOWN)
        {
            roll -= velocity;
        }
        set_camera_params(); // rotation changes it!
    }

   private:

    void constrain_euler_angles()
    {
        if(pitch > 89.0f)  pitch =  89.0f;
        if(pitch < -89.0f) pitch = -89.0f;
        yaw = glm::mod( yaw, 360.0f); // modulo
        roll = glm::mod( roll, 360.0f); // modulo
    }

    void set_camera_params()
    {
        constrain_euler_angles();
        glm::vec3 tmp_front;
        tmp_front.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
        tmp_front.y = glm::sin(glm::radians(pitch));
        tmp_front.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
        front = glm::normalize(tmp_front);

        // Using the current roll value, change the world_up
        // this will change the right and up vectors!
        glm::vec3 tmp_world_up;
        // note roll is against the y-axis. hence sin and cos are flipped.
        tmp_world_up.x = glm::sin(glm::radians(roll));
        tmp_world_up.y = glm::cos(glm::radians(roll));
        tmp_world_up.z = 0.0;
        world_up = tmp_world_up;

        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }
};