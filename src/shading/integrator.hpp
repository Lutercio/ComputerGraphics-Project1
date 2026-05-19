#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include <memory>
#include <optional>

#include "input/paramset.hpp"
#include "scene/scene.hpp"

namespace gc {

class Integrator {
public:
  virtual ~Integrator() = default;
  virtual void render(const Scene& scene) = 0;
};

class SamplerIntegrator : public Integrator {
public:
  ~SamplerIntegrator() override = default;

  [[nodiscard]] virtual std::optional<Spectrum> Li(const Ray& ray, const Scene& scene) const = 0;
  void render(const Scene& scene) override;

protected:
  virtual void preprocess(const Scene& scene);
};

class RayCastIntegrator : public SamplerIntegrator {
public:
  [[nodiscard]] std::optional<Spectrum> Li(const Ray& ray, const Scene& scene) const override;
};

class FlatIntegrator : public RayCastIntegrator {};

class DepthMapIntegrator : public SamplerIntegrator {
public:
  DepthMapIntegrator(std::optional<real_type> z_min,
                     std::optional<real_type> z_max,
                     const Spectrum& near_color,
                     const Spectrum& far_color);

  [[nodiscard]] std::optional<Spectrum> Li(const Ray& ray, const Scene& scene) const override;

protected:
  void preprocess(const Scene& scene) override;

private:
  std::optional<real_type> m_requested_z_min;
  std::optional<real_type> m_requested_z_max;
  real_type m_z_min{ 0 };
  real_type m_z_max{ 1 };
  Spectrum m_near_color{ 0, 0, 0 };
  Spectrum m_far_color{ 1, 1, 1 };
};

class NormalMapIntegrator : public SamplerIntegrator {
public:
  [[nodiscard]] std::optional<Spectrum> Li(const Ray& ray, const Scene& scene) const override;
};

class BlinnPhongIntegrator : public SamplerIntegrator {
public:
  explicit BlinnPhongIntegrator(int max_depth = 0);
  [[nodiscard]] std::optional<Spectrum> Li(const Ray& ray, const Scene& scene) const override;

private:
  [[nodiscard]] std::optional<Spectrum> Li(const Ray& ray,
                                           const Scene& scene,
                                           int depth) const;

  int m_max_depth{ 0 };
};

std::unique_ptr<Integrator> create_integrator(const ParamSet& ps);

}  // namespace gc

#endif  // INTEGRATOR_HPP
