// ======================================================================== //
// Copyright 2018-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

// OSPRay in an Afternoon //
// inspired by the excellent Ray Tracing in One Weekend //

#include <iterator>
#include <memory>
#include <random>
#include "GLFWOSPRayWindow.h"

#include "ospray_testing.h"

#include <imgui.h>

using namespace ospcommon;

std::string renderer_type = "pathtracer";

class Sphere
{
 public:
  Sphere(vec3f center, float radius)
  {
    sphere       = ospNewGeometry("spheres");
    OSPData data = ospNewData(1, OSP_FLOAT4, &center);
    ospCommit(data);
    ospSetData(sphere, "spheres", data);
    ospRelease(data);
    ospSet1i(sphere, "bytes_per_sphere", sizeof(vec3f));
    ospSet1f(sphere, "radius", radius);
    ospCommit(sphere);
  }
  OSPGeometry geom()
  {
    return sphere;
  }

  OSPGeometry sphere;
};

class DiffuseSphere : public Sphere
{
 public:
  DiffuseSphere(vec3f center, float radius, vec3f color)
      : Sphere(center, radius)
  {
    OSPMaterial diffuseMaterial =
        ospNewMaterial2(renderer_type.c_str(), "OBJMaterial");
    ospSet3f(diffuseMaterial, "Kd", color.x, color.y, color.z);
    ospSetf(diffuseMaterial, "Ns", 1.f);
    ospCommit(diffuseMaterial);
    ospSetMaterial(sphere, diffuseMaterial);
    ospRelease(diffuseMaterial);
    ospCommit(sphere);
  }
};

class GlassSphere : public Sphere
{
 public:
  GlassSphere(vec3f center, float radius, float eta) : Sphere(center, radius)
  {
    OSPMaterial glassMaterial = ospNewMaterial2(renderer_type.c_str(), "Glass");
    ospSetf(glassMaterial, "eta", eta);
    ospCommit(glassMaterial);
    ospSetMaterial(sphere, glassMaterial);
    ospRelease(glassMaterial);
    ospCommit(sphere);
  }
  OSPMaterial glassMaterial;
};

class MetalSphere : public Sphere
{
 public:
  MetalSphere(vec3f center, float radius, vec3f color, float roughness)
      : Sphere(center, radius)
  {
    OSPMaterial metalMaterial = ospNewMaterial2(renderer_type.c_str(), "Alloy");
    ospSet3f(metalMaterial, "color", color.x, color.y, color.z);
    ospSetf(metalMaterial, "roughness", roughness);
    ospCommit(metalMaterial);
    ospSetMaterial(sphere, metalMaterial);
    ospRelease(metalMaterial);
    ospCommit(sphere);
  }
  OSPMaterial metalMaterial;
};

OSPModel createRandomScene()
{
  // create the "scene" model which contains all of our geometries
  OSPModel scene = ospNewModel();

  // add in spheres geometry
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<float> chooseRnd(0.f, 1.f);
  std::uniform_real_distribution<float> centerRnd(0.f, 0.9f);
  std::uniform_real_distribution<float> diffuseRnd(0.f, 1.f);
  std::uniform_real_distribution<float> metalRnd(0.5f, 1.f);
  std::uniform_real_distribution<float> roughnessRnd(0.f, 1.f);

  // Create a vector of handles to hold all geometries
  std::vector<Sphere> spheres;

  // All spheres have different materials, so must be created as separate
  // geometries

  // Create "ground" sphere
  spheres.push_back(
      DiffuseSphere(vec3f(0, -1000, 0), 1000, vec3f(0.5, 0.5, 0.5)));

  // Create 3 static spheres
  spheres.push_back(GlassSphere(vec3f(0, 1, 0), 1.0, 1.5));
  spheres.push_back(DiffuseSphere(vec3f(-4, 1, 0), 1.0, vec3f(0.4, 0.2, 0.1)));
  spheres.push_back(
      MetalSphere(vec3f(4, 1, 0), 1.0, vec3f(0.7, 0.6, 0.5), 0.0));

  // Create all random spheres
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      vec3f center(a + centerRnd(gen), 0.2, b + centerRnd(gen));

      if (length(center - vec3f(4, 0.2, 0)) > 0.9) {
        float choose_mat = chooseRnd(gen);
        if (choose_mat < 0.8) {  // diffuse
          vec3f color(diffuseRnd(gen), diffuseRnd(gen), diffuseRnd(gen));
          spheres.push_back(DiffuseSphere(center, 0.2, color));
        } else if (choose_mat < 0.95) {  // metal
          vec3f color(metalRnd(gen), metalRnd(gen), metalRnd(gen));
          spheres.push_back(MetalSphere(center, 0.2, color, roughnessRnd(gen)));

        } else {  // glass
          spheres.push_back(GlassSphere(center, 0.2, 1.5));
        }
      }
    }
  }

  // Add all geometry to the scene
  for (auto &s : spheres) {
    ospAddGeometry(scene, s.geom());
    ospRelease(s.geom());
  }

  return scene;
}

int main(int argc, const char **argv)
{
  // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
  // e.g. "--osp:debug"
  OSPError initError = ospInit(&argc, argv);

  if (initError != OSP_NO_ERROR)
    return initError;

  // set an error callback to catch any OSPRay errors and exit the application
  ospDeviceSetErrorFunc(
      ospGetCurrentDevice(), [](OSPError error, const char *errorDetails) {
        std::cerr << "OSPRay error: " << errorDetails << std::endl;
        exit(error);
      });

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-r" || arg == "--renderer")
      renderer_type = argv[++i];
  }

  // create and commit the scene model
  OSPModel scene = createRandomScene();
  ospCommit(scene);

  // create OSPRay renderer
  OSPRenderer renderer = ospNewRenderer(renderer_type.c_str());

  // create OSPRay lights
  std::vector<OSPLight> lights;

  // XXX: Fix the lighting
  auto ambientLight = ospNewLight3("ambient");
  ospSet1f(ambientLight, "intensity", 1.0f);
  ospSet3f(ambientLight, "color", 0.5, 0.7, 1.0);
  ospCommit(ambientLight);
  lights.push_back(ambientLight);

#if 0
  auto directionalLight = ospNewLight3("distant");
  ospSet1f(directionalLight, "intensity", 0.9f);
  ospSet3f(directionalLight, "color", 1.f, 1.f, 1.f);
  ospSet3f(directionalLight, "direction", 0.f, -1.f, 0.f);
  ospCommit(directionalLight);
  lights.push_back(directionalLight);
#endif

  auto lightsData = ospNewData(lights.size(), OSP_OBJECT, lights.data());
  lights.clear();
  ospSetData(renderer, "lights", lightsData);
  ospRelease(lightsData);

  ospCommit(renderer);

  // XXX: Add default camera parameters
  // create a GLFW OSPRay window: this object will create and manage the OSPRay
  // frame buffer and camera directly
  auto glfwOSPRayWindow =
      std::unique_ptr<GLFWOSPRayWindow>(new GLFWOSPRayWindow(
          vec2i{1024, 768}, box3f(vec3f(-12.f), vec3f(15.f)), scene, renderer));

  glfwOSPRayWindow->registerImGuiCallback([=]() {
    static int spp = 1;
    if (ImGui::SliderInt("spp", &spp, 1, 64)) {
      ospSet1i(renderer, "spp", spp);
      ospCommit(renderer);
    }
  });

  // start the GLFW main loop, which will continuously render
  glfwOSPRayWindow->mainLoop();

  // cleanup remaining objects
  ospRelease(scene);
  ospRelease(renderer);

  // cleanly shut OSPRay down
  ospShutdown();

  return 0;
}
