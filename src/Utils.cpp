#include "Utils.h"

const std::string base_path = "../src/data/";

const std::string colorbar_path = base_path + "colorbar.png";

const float EPSILON = 1e-6;

bool valid_value(float value)
{
    return (value >= 0.0f) && (value <= 1.0f);
}

glm::vec3 Utils::get_dims(Utils::model3d_t model_type)
{
    if (model_type == Utils::BUCKY)
    {
        return glm::vec3(32, 32, 32);
    }
    else if (model_type == Utils::BONSAI)
    {
        return glm::vec3(512, 512, 154);
    }
    else if (model_type == Utils::TEAPOT)
    {
        return glm::vec3(256, 256, 178);
    }
    else if (model_type == Utils::HEAD)
    {
        return glm::vec3(256, 256, 225);
    }
    else
    {
        std::cout << "invalid model type!";
        // return glm::vec3(0,0,0);
        exit(100);
    }
}

GLubyte *Utils::load_3d_raw_data(std::string texture_path, glm::vec3 dimension)
{
    size_t size = dimension[0] * dimension[1] * dimension[2];

    FILE *fp;
    GLubyte *data = new GLubyte[size]; // 8bit
    if (!(fp = fopen(texture_path.c_str(), "rb")))
    {
        std::cout << "Error: opening .raw file failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "OK: open .raw file successed" << std::endl;
    }
    if (fread(data, sizeof(char), size, fp) != size)
    {
        std::cout << "Error: read .raw file failed" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "OK: read .raw file successed" << std::endl;
    }
    fclose(fp);
    return data;
}

GLubyte *Utils::load_3Dfrom_type(Utils::model3d_t model_type)
{
    GLubyte *data = nullptr;
    if (model_type == Utils::BUCKY)
    {
        data = Utils::load_3d_raw_data(base_path + "Bucky_32_32_32.raw", glm::vec3(32, 32, 32));
    }
    else if (model_type == Utils::BONSAI)
    {
        data = Utils::load_3d_raw_data(base_path + "Bonsai_512_512_154.raw", glm::vec3(512, 512, 154));
    }
    else if (model_type == Utils::TEAPOT)
    {
        data = Utils::load_3d_raw_data(base_path + "BostonTeapot_256_256_178.raw", glm::vec3(256, 256, 178));
    }
    else if (model_type == Utils::HEAD)
    {
        data = Utils::load_3d_raw_data(base_path + "Head_256_256_225.raw", glm::vec3(256, 256, 225));
    }

    // check for errors
    if (!data)
    {
        std::cout << "Error in reading raw data! data pointer is currently set to null." << std::endl;
    }
    return data;
}

void Utils::print_glmvec3(const glm::vec3 vec)
{
    std::cout << "\nVec3: ";
    for (int i = 0; i < 3; i++)
    {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
}

// Returns the coefficient in the line equation. For us (in cube), it has to be bounded
// between 0 and 1 (inclusive). So if its out of range, we return -1.0.
float Utils::get_intersection_coef(glm::vec3 plane_point, glm::vec3 normal, glm::vec3 line_point, glm::vec3 line_dir)
{
    // Plane Line Intersection
    // Plane: (point - plane_point) * n = 0
    // Line: point = line_point + (lambda) line_dir.
    // Need to find lambda (float)
    float numerator = glm::dot((plane_point - line_point), normal);
    float denom = glm::dot(line_dir, normal);
    float coef = -1.0f; // good default value
    if (glm::abs(denom) < EPSILON)
    {
        // parallel plane and line
        if (glm::abs(numerator) < EPSILON)
        {
            return 0.0f;
        }
        else
        {
            return -1.0f;
        }
    }
    else
    {
        coef = numerator / denom;
        if ((coef >= 0.0f - EPSILON) && (coef <= 1.0f + EPSILON)) // out of range!
        {
            return coef;
        }
        else
        {
            return -1.0f;
        }
    }
    return coef;
}

// Returns the center point for a polygon
glm::vec3 Utils::center_point(const std::vector<glm::vec3> &point_list)
{
    glm::vec3 avg(0, 0, 0);
    for (const auto &p : point_list)
    {
        avg += p;
    }
    avg /= point_list.size();
    return avg;
}

// Hacky and non optimal way to do sorting O(n^2)-- bUT n = 6 for us.
std::vector<glm::vec3> Utils::sort_points(const std::vector<glm::vec3> &point_list, const std::vector<float> &angles)
{
    std::unordered_set<int> seen_idxs;
    std::vector<int> ordered_idxs;
    std::vector<glm::vec3> ordered_points;
    for (int j = 0; j < point_list.size(); j++)
    {
        int min_idx;
        float min_val = INT_MAX;
        for (int i = 0; i < point_list.size(); i++)
        {
            if (seen_idxs.count(i) == 0)
            {
                if (angles[i] < min_val)
                {
                    min_val = angles[i];
                    min_idx = i;
                }
            }
        }
        seen_idxs.insert(min_idx);
        ordered_idxs.push_back(min_idx);
    }

    for (int i = 0; i < ordered_idxs.size(); i++)
    {
        int idx = ordered_idxs[i];
        ordered_points.push_back(point_list[idx]);
    }
    return ordered_points;
}

// Returns a new vector of points in sorted order wrt center
std::vector<glm::vec3> Utils::get_ordered_points(const std::vector<glm::vec3> &point_list, const glm::vec3 &center)
{
    std::vector<float> cos_angles;
    glm::vec3 fixed_vec(1.0, 0.0, 0.0); // For reference angle (X - axis)
    for (const auto &p : point_list)
    {
        float dot_prod = glm::dot(fixed_vec, glm::normalize(p - center));
        cos_angles.push_back(dot_prod);
    }
    // Now Sort and get ordered points.
    return Utils::sort_points(point_list, cos_angles);
}

bool Utils::is_valid_point(const glm::vec3 &point)
{
    return (valid_value(point.x)) && (valid_value(point.y)) && (valid_value(point.z));
}

glm::vec3 Utils::transform_pos(const glm::mat4 &modelViewmat, const glm::vec3 &pos)
{
    auto tmp_vec = modelViewmat * glm::vec4(pos, 1.0);
    return glm::vec3(tmp_vec.x, tmp_vec.y, tmp_vec.z);
}

glm::vec3 Utils::transform_dir(const glm::mat4 &modelViewmat, const glm::vec3 &dir)
{
    auto tmp_vec = modelViewmat * glm::vec4(dir, 0.0);
    return glm::vec3(tmp_vec.x, tmp_vec.y, tmp_vec.z);
}

std::vector<glm::vec3> Utils::transform_cube_vertices(const glm::mat4 &modelViewmat,
                                                      const std::vector<glm::vec3> &point_list)
{
    std::vector<glm::vec3> transformed_points;
    for(const auto& p : point_list)
    {
        transformed_points.push_back(Utils::transform_pos(modelViewmat, p));
    }
    return transformed_points;
}

