// clang-format off
#include <vulkan\vulkan_core.h>
#include <GLFW\glfw3.h>
// clang-format on

#include "window.h"
#include "renderer.h"

#include "input.h"
#include "camera.h"
#include "clock.h"

int
main()
{

  Window window(1280, 920, "Two Triangles");
  Renderer renderer(&window);
  Camera cam(70.f, 1280.f / 920.f, 0.1f, 1000.f);
  cam.SetPosition({ 0.0f, 0.0f, 5.0f });
  Input input = {};
  Clock clock = {};

  while (true) {
    window.Update();
    input.Update(&window);
    renderer.Update();
    clock.Update();
    cam.Update(&input, &clock);

    renderer.drawFrame(cam.GetProjView());
  }

  return 0;
}
