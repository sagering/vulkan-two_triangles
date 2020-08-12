#pragma once

#include <glm\glm.hpp>
#include <tuple>

#include "graphics_pipeline.h"
#include "vk_base.h"

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription GetBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::vector<VkVertexInputAttributeDescription>
  GetAttributeDescriptions()
  {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
  }
};

struct Renderer : VulkanBase
{
public:
  Renderer(VulkanWindow* window);
  ~Renderer();

  void drawFrame(const glm::mat4& vp);

private:
  virtual void OnSwapchainReinitialized();

  GraphicsPipeline* pipeline;
  VkShaderModule vertexShaderModule;
  VkShaderModule fragmentShaderModule;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkDeviceSize vertexCount;
  VkDeviceSize vertexBufferOffset;

  VkDescriptorPool descriptorPool;
  VkDescriptorSet descriptorSet;

  VkBuffer cameraBuffer;
  VkDeviceMemory cameraBufferMemory;

  void recordCommandBuffer(uint32_t idx);

private:
  void initialize();
  void createDescriptorPool();
  void destroyDescriptorPool();

  void createPipeline();
  void destroyPipeline();

  void createDescriptorSets();
  void destroyDescriptorSets();

  void createBuffersAndSamplers();
  void destroyBuffersAndSamplers();
};
