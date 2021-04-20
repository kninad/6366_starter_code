#include "RawData.h"

const std::string base_path = "../src/data/";

const std::string colorbar_path = base_path + "colorbar.png";

glm::vec3 RawDataUtil::get_dims(RawDataUtil::model3d_t model_type)
{
    if (model_type == RawDataUtil::BUCKY)
    {
        return glm::vec3(32, 32, 32);
    } 
    else if(model_type == RawDataUtil::BONSAI)
    {
        return glm::vec3(512, 512, 154);
    }
    else if(model_type == RawDataUtil::TEAPOT)
    {
        return glm::vec3(256, 256, 178);
    }
    else if(model_type == RawDataUtil::HEAD)
    {
        return glm::vec3(256, 256, 225);
    }
}

GLubyte* RawDataUtil::load_3d_raw_data(std::string texture_path, glm::vec3 dimension)
{
    size_t size = dimension[0] * dimension[1] * dimension[2];

    FILE *fp;
    GLubyte *data = new GLubyte[size];			  // 8bit
    if (!(fp = fopen(texture_path.c_str(), "rb"))) {
        std::cout << "Error: opening .raw file failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "OK: open .raw file successed" << std::endl;
    }
    if (fread(data, sizeof(char), size, fp) != size) {
        std::cout << "Error: read .raw file failed" << std::endl;
        exit(1);
    }
    else {
        std::cout << "OK: read .raw file successed" << std::endl;
    }
    fclose(fp);
    return data;
}

GLubyte* RawDataUtil::load_3Dfrom_type(RawDataUtil::model3d_t model_type)
    {
        GLubyte* data = nullptr;
        if (model_type == RawDataUtil::BUCKY)
        {
            data = RawDataUtil::load_3d_raw_data(base_path + "Bucky_32_32_32.raw", glm::vec3(32, 32, 32));
        } 
        else if(model_type == RawDataUtil::BONSAI)
        {
            data = RawDataUtil::load_3d_raw_data(base_path + "Bonsai_512_512_154.raw", glm::vec3(512, 512, 154));
        }
        else if(model_type == RawDataUtil::TEAPOT)
        {
            data = RawDataUtil::load_3d_raw_data(base_path + "BostonTeapot_256_256_178.raw", glm::vec3(256, 256, 178));
        }
        else if(model_type == RawDataUtil::HEAD)
        {
            data = RawDataUtil::load_3d_raw_data(base_path + "Head_256_256_225.raw", glm::vec3(256, 256, 225));
        }

        // check for errors
        if(!data){
            std::cout << "Error in reading raw data! data pointer is currently set to null." << std::endl;
        }
        return data;
    }




