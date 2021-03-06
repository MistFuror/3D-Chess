//
// Created by aca on 19.2.21..
//

#ifndef RG_3D_SAH_LIGHT_H
#define RG_3D_SAH_LIGHT_H

#include <glm/glm.hpp>
#include "Shader.h"

class Light {
    std::string prefix;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
public:
    Light(const std::string &prefix, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular);
    Light(const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular);
    virtual void activate(const Shader &shader) const = 0;
    virtual void activate(const Shader &shader, const std::string &prefix) const = 0;

    const std::string &getPrefix() const;

    void setPrefix(const std::string &prefix);

    const glm::vec3 &getAmbient() const;

    void setAmbient(const glm::vec3 &ambient);

    const glm::vec3 &getDiffuse() const;

    void setDiffuse(const glm::vec3 &diffuse);

    const glm::vec3 &getSpecular() const;

    void setSpecular(const glm::vec3 &specular);
};


#endif //RG_3D_SAH_LIGHT_H
