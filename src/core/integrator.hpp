#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "scene.hpp"

namespace gc {

class Integrator {
public:
    virtual ~Integrator() = default;
    virtual void render(const Scene& scene) = 0;
};

class FlatIntegrator : public Integrator {
public:
    void render(const Scene& scene) override;
};

}  // namespace gc

#endif  // INTEGRATOR_HPP
