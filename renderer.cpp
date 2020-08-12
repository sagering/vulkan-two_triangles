#include "renderer.h"

#include <algorithm>
#include <array>
#include <iostream>

#include "vk_init.h"
#include "vk_utils.h"

Renderer::Renderer(VulkanWindow* window)
  : VulkanBase(window)
{
  initialize();
  createDescriptorPool();
  createPipeline();
  createBuffersAndSamplers();
  createDescriptorSets();
}

std::string
LoadFile(const char* _filename)
{
  std::string buff;

  FILE* file = 0;
  fopen_s(&file, _filename, "rb");
  if (file) {
    fseek(file, 0, SEEK_END);
    size_t bytes = ftell(file);

    buff.resize(bytes);

    fseek(file, 0, SEEK_SET);
    fread(&(*buff.begin()), 1, bytes, file);
    fclose(file);
    return buff;
  }

  return buff;
}

VkShaderModule
LoadShaderModule(VkDevice device, const char* filename)
{
  std::string buff = LoadFile(filename);
  auto result =
    vkuCreateShaderModule(device, buff.size(), (uint32_t*)buff.data(), nullptr);
  ASSERT_VK_VALID_HANDLE(result);
  return result;
}

void
Renderer::initialize()
{
  // load shader modules
  fragmentShaderModule = LoadShaderModule(device, "simple.frag.spv");
  vertexShaderModule = LoadShaderModule(device, "simple.vert.spv");
}

Renderer::~Renderer()
{
  destroyDescriptorSets();
  destroyBuffersAndSamplers();
  destroyPipeline();
}

void
Renderer::createPipeline()
{
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  pipeline =
    GraphicsPipeline::GetBuilder()
      .SetDevice(device)
      .SetVertexShader(vertexShaderModule)
      .SetFragmentShader(fragmentShaderModule)
      .SetVertexBindings({ Vertex::GetBindingDescription() })
      .SetVertexAttributes(Vertex::GetAttributeDescriptions())
      .SetDescriptorSetLayouts({ { { 0,
                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     1,
                                     VK_SHADER_STAGE_VERTEX_BIT } } })
      .SetViewports({ { 0.0f,
                        0.0f,
                        (float)swapchain->imageExtent.width,
                        (float)swapchain->imageExtent.height,
                        0.0f,
                        1.0f } })
      .SetScissors(
        { { { 0, 0 },
            { swapchain->imageExtent.width, swapchain->imageExtent.height } } })
      .SetColorBlendAttachments({ colorBlendAttachment })
      .SetRenderPass(renderPass)
      //.SetDepthWriteEnable(VK_TRUE)
      //.SetMaxDepthBounds(1.0)
      //.SetMinDepthBounds(0.0)
      .Build();
}

void
Renderer::destroyPipeline()
{}

void
Renderer::createDescriptorPool()
{
  auto poolSize = vkiDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
  auto info = vkiDescriptorPoolCreateInfo(1, 1, &poolSize);
  vkCreateDescriptorPool(device, &info, nullptr, &descriptorPool);
}

void
Renderer::destroyDescriptorPool()
{
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void
Renderer::createDescriptorSets()
{
  auto info = vkiDescriptorSetAllocateInfo(
    descriptorPool, 1, pipeline->descriptorSetLayouts.data());
  vkAllocateDescriptorSets(device, &info, &descriptorSet);

  auto bufferInfo = vkiDescriptorBufferInfo(cameraBuffer, 0, sizeof(glm::mat4));
  auto descriptorWrite =
    vkiWriteDescriptorSet(descriptorSet,
                          0,
                          0,
                          1,
                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                          nullptr,
                          &bufferInfo,
                          nullptr);

  vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void
Renderer::destroyDescriptorSets()
{
  vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
}

void
Renderer::createBuffersAndSamplers()
{
  std::vector<Vertex> vertices = {
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },

    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } }
  };

  vertexCount = vertices.size();
  vertexBufferOffset = 0;

  VkDeviceSize size = vertexCount * sizeof(Vertex);
  vertexBuffer = vkuCreateBuffer(device,
                                 size,
                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 {});
  vertexBufferMemory =
    vkuAllocateBufferMemory(device,
                            physicalDeviceProps.memProps,
                            vertexBuffer,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                            true);
  vkuTransferData(device, vertexBufferMemory, 0, size, vertices.data());

  cameraBuffer = vkuCreateBuffer(device,
                                 sizeof(glm::mat4),
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 {});
  cameraBufferMemory =
    vkuAllocateBufferMemory(device,
                            physicalDeviceProps.memProps,
                            cameraBuffer,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                            true);
}

void
Renderer::destroyBuffersAndSamplers()
{
  vkDestroyBuffer(device, vertexBuffer, nullptr);
  vkFreeMemory(device, vertexBufferMemory, nullptr);
}

void
Renderer::recordCommandBuffer(uint32_t idx)
{
  ASSERT_VK_SUCCESS(
    vkWaitForFences(device, 1, &fences[idx], true, (uint64_t)-1));
  ASSERT_VK_SUCCESS(vkResetFences(device, 1, &fences[idx]));
  ASSERT_VK_SUCCESS(vkResetCommandBuffer(commandBuffers[idx], 0));

  VkCommandBufferBeginInfo beginInfo = vkiCommandBufferBeginInfo(nullptr);
  ASSERT_VK_SUCCESS(vkBeginCommandBuffer(commandBuffers[idx], &beginInfo));

  VkClearValue clearValues[] = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.f, 0 } };
  VkRenderPassBeginInfo renderPassInfo =
    vkiRenderPassBeginInfo(renderPass,
                           framebuffers[idx],
                           { { 0, 0 }, swapchain->imageExtent },
                           2,
                           clearValues);

  vkCmdBeginRenderPass(
    commandBuffers[idx], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(
    commandBuffers[idx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
  vkCmdBindVertexBuffers(
    commandBuffers[idx], 0, 1, &vertexBuffer, &vertexBufferOffset);
  vkCmdBindDescriptorSets(commandBuffers[idx],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline->pipelineLayout,
                          0,
                          1,
                          &descriptorSet,
                          0,
                          nullptr);
  vkCmdDraw(commandBuffers[idx], static_cast<uint32_t>(vertexCount), 1, 0, 0);
  vkCmdEndRenderPass(commandBuffers[idx]);
  ASSERT_VK_SUCCESS(vkEndCommandBuffer(commandBuffers[idx]));
}

void
Renderer::drawFrame(const glm::mat4& vp)
{
  uint32_t nextImageIdx = -1;
  ASSERT_VK_SUCCESS(vkAcquireNextImageKHR(device,
                                          swapchain->handle,
                                          UINT64_MAX,
                                          imageAvailableSemaphore,
                                          VK_NULL_HANDLE,
                                          &nextImageIdx));

  recordCommandBuffer(nextImageIdx);

  vkuTransferData(
    device, cameraBufferMemory, 0, sizeof(glm::mat4), (void*)(&vp));

  VkPipelineStageFlags waitStages[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  VkSubmitInfo submitInfo = vkiSubmitInfo(1,
                                          &imageAvailableSemaphore,
                                          waitStages,
                                          1,
                                          &commandBuffers[nextImageIdx],
                                          1,
                                          &renderFinishedSemaphore);
  ASSERT_VK_SUCCESS(vkQueueSubmit(queue, 1, &submitInfo, fences[nextImageIdx]));

  VkPresentInfoKHR presentInfo = vkiPresentInfoKHR(
    1, &renderFinishedSemaphore, 1, &swapchain->handle, &nextImageIdx, nullptr);
  ASSERT_VK_SUCCESS(vkQueuePresentKHR(queue, &presentInfo));
}

void
Renderer::OnSwapchainReinitialized()
{
  destroyPipeline();
  createPipeline();
}
