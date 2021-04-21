#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "RawData.h"

class Object
{
 public:
    struct Vertex
    {
        // Position
        glm::vec3 Position;
        // TexCoords
        glm::vec3 TexCoords;
    };

    struct Face_Index
    {
        int vertex[3];
    };

    // veo and vao vector
    std::vector<Vertex> vao_vertices;
    std::vector<unsigned int> veo_indices;

    // obj original data vector
    std::vector<glm::vec3> ori_positions;

    // obj face index vector
    std::vector<Face_Index> indexed_faces;

    glm::vec2 z_max = glm::vec2(0.0f, 0.0f);            // z coord for the vertices having max x and y values.
    glm::vec2 z_min = glm::vec2(0.0f, 0.0f);            // z coord for the vertices having min x and y values.
    
    glm::vec3 max_bound = glm::vec3(INT_MIN, INT_MIN, INT_MIN);
    glm::vec3 min_bound = glm::vec3(INT_MAX, INT_MAX, INT_MAX);
    glm::vec3 center_cam_pos = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec4 obj_color = glm::vec4(0.7, 0.7, 0.7, 1.0);
    GLfloat shininess = 32.0f;

    GLuint vao, vbo, ebo;
    GLuint tex3dID;
    

 private:

    void add_vertex_from_face(const Face_Index& face)
    {           
        const glm::vec3 k_FixedPosition(1.0,1.0,1.0);
        // Populate the position and texture information.
        for (int i = 0; i < 3; i++)
        {
            Vertex point;
            int vert_idx = face.vertex[i];
            point.Position = ori_positions[vert_idx];
            point.TexCoords = k_FixedPosition - point.Position;
            vao_vertices.push_back(point);
        }
        // Can set and use edge info here ? or somewhere?
    }

    void add_face_to_veo(const Face_Index& face)
    {
        for(int i=0; i < 3; i++)
        {
            veo_indices.push_back(face.vertex[i]);
        }
    }

    void update_bounds(const glm::vec3& point)
    {   
        // std::cout << "[DEBUG] POINT: ";
        // print_glmvec3(point);

        // X coordinates
        if (point[0] > max_bound[0])
        {
            max_bound[0] = point[0];
            z_max[0] = point[2];
        }
        if (point[0] < min_bound[0])
        {
            min_bound[0] = point[0];
            z_min[0] = point[2];
        }

        // Y coordinates
        if (point[1] > max_bound[1])
        {
            max_bound[1] = point[1];
            z_max[1] = point[2];
        }
        if (point[1] < min_bound[1])
        {
            min_bound[1] = point[1];
            z_min[1] = point[2];
        }
        // Z coordinates
        if (point[2] > max_bound[2])
            max_bound[2] = point[2];
        if (point[2] < min_bound[2])
            min_bound[2] = point[2];
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


 public:

    Object()
    {
        // model3d = model3d_type;
        load_cube_with_edges();
    };

    ~Object(){};

    void print_glmvec3(const glm::vec3& vec)
    {
        std::cout << "\nVec3: ";
        for(int i=0; i<3; i++)
        {
            std::cout << vec[i] << " ";
        }
        std::cout << std::endl;
    }

    void update_center_camera_position(float fov = 45.0f)
    {
        center_cam_pos[0] = (max_bound[0] + min_bound[0]) / 2.0f; // X center
        std::cout << "[DEBUG] Cx " << center_cam_pos[0] << std::endl;
        center_cam_pos[1] = (max_bound[1] + min_bound[1]) / 2.0f; // Y center
        std::cout << "[DEBUG] Cy " << center_cam_pos[1] << std::endl;

        // Z coord -- Trignometry skills
        float tan_fov_by2 = (float)glm::tan(glm::radians(fov) / 2);
        float x_range = max_bound[0] - min_bound[0];
        float y_range = max_bound[1] - min_bound[1];
        float z_x = (x_range / (2 * tan_fov_by2)); // + glm::max(z_max[0], z_min[0]);
        float z_y = (y_range / (2 * tan_fov_by2)); // + glm::max(z_max[1], z_min[1]);
        std::cout << "\n[Debug:Log] z_x, z_y:" << z_x << " " << z_y;
        center_cam_pos[2] = glm::max(z_x, z_y) + max_bound[2];
    }

    void load_cube_with_edges()
    {
        vao_vertices.clear();
        veo_indices.clear();
        indexed_faces.clear();
        ori_positions.clear();

        // Iterate over vertex data
        // 3 positions at a time for 3-coords of a vertex
        for (int i = 0; i < 24; i += 3)
        {
            glm::vec3 vert_pos(cube_vertices[i],
                               cube_vertices[i + 1],
                               cube_vertices[i + 2]);
            ori_positions.push_back(vert_pos);
            update_bounds(vert_pos);
        }

        // Now the face data using face indices
        // Can also optinally just set the veo_indices vector directly here!
        for (int i = 0; i < 36; i+=3)
        {
            Face_Index face;
            for(int j = 0; j < 3; j++) 
            {
                face.vertex[j] = cube_indices[i+j];            
            }
            indexed_faces.push_back(face);
        }

        // Retrieve data from index and assign to vao and veo
        // for(int i = 0; i < indexed_faces.size(); i++)
        // {
        //     // populate vao_vertices 
        //     add_vertex_from_face(indexed_faces[i]);
        //     // pop veo indices
        //     add_face_to_veo(indexed_faces[i]);            
        // }

        for(const auto& pos : ori_positions)
        {
            Vertex point;
            point.Position = pos;
            point.TexCoords = glm::vec3(1.0,1.0,1.0) - pos;
            vao_vertices.push_back(point);
        }
        
        // populate veo_indices
        for(int i = 0; i < 36; i++)
        {
            veo_indices.push_back(cube_indices[i]);
        }

        // for(const auto &p : vao_vertices)
        // {
        //     print_glmvec3(p.Position);
        //     print_glmvec3(p.TexCoords);
        // }

        update_center_camera_position();           // use default fov of 45
        std::cout << "\n\n [DebugLog] Cent Pos: ";
        print_glmvec3(center_cam_pos);
        print_glmvec3(max_bound);
        print_glmvec3(min_bound);
        std::cout << std::endl;
    }

    GLuint load3dTexture(RawDataUtil::model3d_t model_type)
    {
        GLuint textureID;
        glGenTextures(1, &textureID);

        GLubyte* data = RawDataUtil::load_3Dfrom_type(model_type);
        if(!data)
        {
            std::cout << "Raw Data Model Loading failed! " << std::endl;
        }
        glm::vec3 dims = RawDataUtil::get_dims(model_type);
        glBindTexture(GL_TEXTURE_3D, textureID);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, dims[0], dims[1], dims[2], 0, GL_RED, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_3D);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }


};
