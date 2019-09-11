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

// OSPRay in an Afternoon (version for OSPRay v1.8.5)
// Inspired by Pete Shirley's excellent Ray Tracing in One Weekend

#include <iterator>
#include <memory>
#include <random>
#include "GLFWOSPRayWindow.h"

#include <imgui.h>

using namespace ospcommon;

static std::string renderer_type = "pathtracer";

class Sphere
{
 public:
  Sphere(vec3f center, float radius)
  {
    sphereGeometry = ospNewGeometry("spheres");
    OSPData data   = ospNewData(1, OSP_FLOAT4, &center);
    ospSetData(sphereGeometry, "spheres", data);
    ospRelease(data);
    ospSet1i(sphereGeometry, "bytes_per_sphere", sizeof(vec3f));
    ospSet1f(sphereGeometry, "radius", radius);
    ospCommit(sphereGeometry);
  }

  OSPGeometry g()
  {
    return sphereGeometry;
  }

  OSPGeometry sphereGeometry;
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
    ospSetMaterial(sphereGeometry, diffuseMaterial);
    ospRelease(diffuseMaterial);
    ospCommit(sphereGeometry);
  }
};

class GlassSphere : public Sphere
{
 public:
  GlassSphere(vec3f center,
              float radius,
              float eta,
              vec3f color = {1.f, 1.f, 1.f})
      : Sphere(center, radius)
  {
    OSPMaterial glassMaterial = ospNewMaterial2(renderer_type.c_str(), "Glass");
    ospSet3f(glassMaterial, "attenuationColor", color.x, color.y, color.z);
    ospSetf(glassMaterial, "eta", eta);
    ospCommit(glassMaterial);
    ospSetMaterial(sphereGeometry, glassMaterial);
    ospRelease(glassMaterial);
    ospCommit(sphereGeometry);
  }
};

class MetalSphere : public Sphere
{
 public:
  MetalSphere(vec3f center, float radius, float roughness, vec3f color)
      : Sphere(center, radius)
  {
    OSPMaterial metalMaterial = ospNewMaterial2(renderer_type.c_str(), "Alloy");
    ospSet3f(metalMaterial, "color", color.x, color.y, color.z);
    ospSetf(metalMaterial, "roughness", roughness);
    ospCommit(metalMaterial);
    ospSetMaterial(sphereGeometry, metalMaterial);
    ospRelease(metalMaterial);
    ospCommit(sphereGeometry);
  }
};

OSPModel createRandomScene()
{
  // create the "scene" model which contains all of our geometries
  OSPModel scene = ospNewModel();

  // add in spheres geometry
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<float> matRnd(0.f, 1.f);
  std::uniform_real_distribution<float> centerRnd(0.f, 0.9f);
  std::uniform_real_distribution<float> colorRnd(0.f, 1.f);
  std::uniform_real_distribution<float> roughnessRnd(0.f, 0.9f);

  // All spheres have different materials, so must be created as separate
  // geometries

  std::vector<OSPGeometry> spheres;

  // Create "ground" sphere
  spheres.push_back(
      DiffuseSphere(vec3f(0, -1000, 0), 1000, vec3f(0.5, 0.5, 0.5)).g());

  // Create 3 static spheres
  spheres.push_back(GlassSphere(vec3f(0, 1, 0), 1.0, 1.5).g());
  spheres.push_back(
      DiffuseSphere(vec3f(-4, 1, 0), 1.0, vec3f(0.4, 0.2, 0.1)).g());
  spheres.push_back(
      MetalSphere(vec3f(4, 1, 0), 1.0, 0.0, vec3f(0.7, 0.6, 0.5)).g());

  // Create all random spheres
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      vec3f center(a + centerRnd(gen), 0.2, b + centerRnd(gen));
      float type_mat(matRnd(gen));
      vec3f color(colorRnd(gen), colorRnd(gen), colorRnd(gen));

      if (length(center - vec3f(4, 0.2, 0)) > 0.9) {
        if (type_mat < 0.5) {  // diffuse
          spheres.push_back(DiffuseSphere(center, 0.2, color).g());
        } else if (type_mat < 0.75) {  // metal
          spheres.push_back(
              MetalSphere(center, 0.2, roughnessRnd(gen), color).g());
        } else {  // glass
          spheres.push_back(GlassSphere(center, 0.2, 1.5, color).g());
        }
      }
    }
  }

    // Add geometries to scene model, then free geometry
  for (auto &s : spheres) {
    ospAddGeometry(scene, s);
    ospRelease(s);
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

  auto ambientLight = ospNewLight3("ambient");
  ospSet1f(ambientLight, "intensity", 0.7f);
  ospSet3f(ambientLight, "color", 0.5, 0.7, 1.0);
  ospCommit(ambientLight);
  lights.push_back(ambientLight);

  auto directionalLight = ospNewLight3("distant");
  ospSet1f(directionalLight, "intensity", 1.0f);
  ospSet3f(directionalLight, "color", 1.f, 0.72f, 0.075f);
  ospSet3f(directionalLight, "direction", 0.f, -1.f, 0.5f);
  ospCommit(directionalLight);
  lights.push_back(directionalLight);

  auto lightsData = ospNewData(lights.size(), OSP_OBJECT, lights.data());

  for (auto &l : lights)
    ospRelease(l);
  lights.clear();

  ospSetData(renderer, "lights", lightsData);
  ospRelease(lightsData);

  ospCommit(renderer);

  // XXX: Add default camera parameters
  // XXX: Add depth of field, focus length and aperture
  // create a GLFW OSPRay window: this object will create and manage the OSPRay
  // frame buffer and camera directly
  auto glfwOSPRayWindow =
      std::unique_ptr<GLFWOSPRayWindow>(new GLFWOSPRayWindow(
          vec2i{1024, 768}, box3f(vec3f(-12.f), vec3f(12.f)), scene, renderer));

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
