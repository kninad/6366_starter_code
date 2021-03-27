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

class Object
{
   public:
    struct Vertex
    {
        // Position
        glm::vec3 Position;
        // Normal
        glm::vec3 Normal;
        // TexCoords
        glm::vec2 TexCoords;
        // Tangent T
        glm::vec3 Tangent;
        // BiTangent B
        glm::vec3 Bitangent;
    };

    struct Vertex_Index
    {
        int pos_idx;
        int normal_idx;
        int texcoord_idx;
    };

    struct Face_Index
    {
        Vertex_Index vertex[3];
    };

    // veo and vao vector
    std::vector<Vertex> vao_vertices;
    std::vector<unsigned int> veo_indices;

    // obj original data vector
    std::vector<glm::vec3> ori_positions;
    std::vector<glm::vec3> ori_normals;
    std::vector<glm::vec2> ori_texcoords;

    // obj face index vector
    std::vector<Face_Index> indexed_faces;

    glm::vec2 z_max = glm::vec2(0.0f,0.0f); // z coord for the vertices having max x and y values.
    glm::vec2 z_min = glm::vec2(0.0f,0.0f); // z coord for the vertices having min x and y values.
    glm::vec3 obj_center = glm::vec3(0.0f,0.0f,0.0f); // UNUSED!
    glm::vec3 max_bound = glm::vec3(INT_MIN);
    glm::vec3 min_bound = glm::vec3(INT_MAX);
    glm::vec3 center_cam_pos = glm::vec3(0.0f,0.0f,0.0f);

    glm::vec4 obj_color = glm::vec4(0.7, 0.7, 0.7, 1.0);
    GLfloat shininess = 32.0f;

    std::string m_obj_path;
    std::string obj_name;

    GLuint vao, vbo, ebo;
    GLuint diffuse_textureID, normal_textureID;

   private:

    void add_vertex_from_face(const Face_Index& face)
    {   
        Vertex points[3]; 
        glm::vec3 tangent, bitangent;
        // Populate the position, normal and texture information but not the T and B info.
        for (int i = 0; i < 3; i++)
        {
            Vertex_Index v = face.vertex[i];
            Vertex tmp;
            tmp.Position = ori_positions[v.pos_idx];
            tmp.Normal = ori_normals[v.normal_idx];
            tmp.TexCoords = ori_texcoords[v.texcoord_idx];
            points[i] = tmp;
        }

        glm::vec3 edge1 = points[1].Position - points[0].Position;
        glm::vec3 edge2 = points[2].Position - points[0].Position;
        glm::vec2 deltaUV1 = points[1].TexCoords - points[0].TexCoords;
        glm::vec2 deltaUV2 = points[2].TexCoords - points[0].TexCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        
        // Set the same T and B for all 3 points in face and push to vao_vertices.
        for(int i = 0; i < 3; i++)
        {
            points[i].Tangent = tangent;
            points[i].Bitangent = bitangent;
            vao_vertices.push_back(points[i]);
        }
    }


    void update_bounds(const glm::vec3& point)
    {
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
        if (point[2] > max_bound[2]) max_bound[2] = point[2];
        if (point[2] < min_bound[2]) min_bound[2] = point[2];
     }


   public:

    unsigned int loadTexture(char const * path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }


    Object(std::string obj_path)
    {
        this->m_obj_path = obj_path;
        load_obj(this->m_obj_path);
    };


    ~Object(){};


    void print_glmvec3(const glm::vec3 vec)
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
        center_cam_pos[1] = (max_bound[1] + min_bound[1]) / 2.0f; // Y center
        // Z coord -- Trignometry skills
        float tan_fov_by2 = (float) glm::tan(glm::radians(fov) / 2);
        float x_range = max_bound[0] - min_bound[0];
        float y_range = max_bound[1] - min_bound[1];
        float z_x = (x_range / (2 * tan_fov_by2)); // + glm::max(z_max[0], z_min[0]);
        float z_y = (y_range / (2 * tan_fov_by2)); // + glm::max(z_max[1], z_min[1]);
        std::cout << "\n[Debug:Log] z_x, z_y:" << z_x << " " << z_y;
        center_cam_pos[2] = glm::max(z_x, z_y) + max_bound[2];
    }


    void load_obj(std::string obj_path)
    {
        int path_str_length = obj_path.size();
        std::string suffix = obj_path.substr(path_str_length - 3, path_str_length);

        if (suffix == "obj")
        {
            this->vao_vertices.clear();
            this->veo_indices.clear();
            this->indexed_faces.clear();

            this->ori_positions.clear();
            this->ori_normals.clear();
            this->ori_texcoords.clear();

            std::ifstream ifs;
            // Store original data vector
            try
            {
                ifs.open(obj_path);
                std::cout << "\n\n [DebugLog] Opened the object model file." << std::endl;
                std::string one_line;
                // int counter = 0; // For object center, running average computation
                while (getline(ifs, one_line))
                {
                    std::stringstream ss(one_line);
                    std::string type;
                    ss >> type;
                    if (type == "v")
                    {
                        glm::vec3 vert_pos;
                        ss >> vert_pos[0] >> vert_pos[1] >> vert_pos[2];
                        this->ori_positions.push_back(vert_pos);
                        update_bounds(vert_pos);
                    }
                    else if (type == "vt")
                    {
                        glm::vec2 tex_coord;
                        ss >> tex_coord[0] >> tex_coord[1];
                        this->ori_texcoords.push_back(tex_coord);
                    }
                    else if (type == "vn")
                    {
                        glm::vec3 vert_norm;
                        ss >> vert_norm[0] >> vert_norm[1] >> vert_norm[2];
                        this->ori_normals.push_back(vert_norm);
                    }
                    else if (type == "f")
                    {
                        Face_Index face_idx;
                        // Here we only accept face number 3 i.e triangle
                        // In other words, face with only 3 vertices.
                        for (int i = 0; i < 3; i++)
                        {
                            std::string s_vertex;
                            ss >> s_vertex;
                            int pos_idx = -1;
                            int tex_idx = -1;
                            int norm_idx = -1;
                            sscanf(s_vertex.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);
                            // We have to use index -1 because the obj index starts at 1
                            // Incorrect input will be set as -1
                            face_idx.vertex[i].pos_idx = pos_idx > 0 ? pos_idx - 1 : -1;
                            face_idx.vertex[i].texcoord_idx = tex_idx > 0 ? tex_idx - 1 : -1;
                            face_idx.vertex[i].normal_idx = norm_idx > 0 ? norm_idx - 1 : -1;
                        }
                        indexed_faces.push_back(face_idx);
                    }
                }
            }
            catch (const std::exception&)
            {
                std::cout << "Error: Obj file cannot be read\n";
            }

            /*
             * TODO: Retrieve data from index and assign to vao and veo
             */
            for (int i = 0; i < indexed_faces.size(); i++)
            {
                // Populate vao_vertices
                add_vertex_from_face(indexed_faces[i]);
                // Push the vertex indices!
                veo_indices.push_back(3 * i);
                veo_indices.push_back(3 * i + 1);
                veo_indices.push_back(3 * i + 2);
            }

            update_center_camera_position(); // use default fov of 45
            std::cout << "\n\n [DebugLog] Cent Pos: "; // center_cam_pos[0] << " " << center_cam_pos[1] << " " << center_cam_pos[2];
            print_glmvec3(center_cam_pos);
            print_glmvec3(max_bound);
            print_glmvec3(min_bound);
            print_glmvec3(obj_center);
            std::cout << std::endl;
        }
    };

};