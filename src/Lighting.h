#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Lighting
{
    /*
     * TODO: Define lighting parameters
    */

    glm::vec3 pLightOrgLoc;

    struct Direction_Light
    {
        // TODO: Define direct light parameters
        glm::vec3 direction;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    struct Point_Light
    {
        // TODO: Define point light parameters
        glm::vec3 position;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

public:
    Direction_Light direction_light;
    Point_Light point_light;

    Lighting() {}

    ~Lighting() {}

    /*
    * TODO: Init all lighting parameters
    */
    void init()
    {   
        // Some placeholder init values
        direction_light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
        direction_light.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
        direction_light.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
        direction_light.specular = glm::vec3(0.5f, 0.5f, 0.5f);
        
        point_light.position = glm::vec3(0.0f, 0.0f, 10.0f);
        point_light.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
        point_light.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
        point_light.specular = glm::vec3(1.0f, 1.0f, 1.0f);

        pLightOrgLoc = point_light.position; // just for init

    };
};