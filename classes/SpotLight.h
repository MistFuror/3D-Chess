//
// Created by aca on 3.2.21..
//

#ifndef RG_3D_SAH_SPOTLIGHT_H
#define RG_3D_SAH_SPOTLIGHT_H

#include "Light.h"

class SpotLight: public Light {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float constant;
    float linear;
    float quadratic;
public:
    SpotLight(const std::string &prefix, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular,
              const glm::vec3 &position, const glm::vec3 &direction,
              float cutOff, float constant, float linear, float quadratic);
    SpotLight(const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular,
              const glm::vec3 &position, const glm::vec3 &direction,
              float cutOff, float constant, float linear, float quadratic);
    void activate(const Shader &shader) const override;
    void activate(const Shader &shader, const std::string &prefix) const override;

    const glm::vec3 &getPosition() const;

    void setPosition(const glm::vec3 &position);

    const glm::vec3 &getDirection() const;

    void setDirection(const glm::vec3 &direction);

    float getCutOff() const;

    void setCutOff(float cutOff);

    float getConstant() const;

    void setConstant(float constant);

    float getLinear() const;

    void setLinear(float linear);

    float getQuadratic() const;

    void setQuadratic(float quadratic);
};


#endif //RG_3D_SAH_SPOTLIGHT_H
