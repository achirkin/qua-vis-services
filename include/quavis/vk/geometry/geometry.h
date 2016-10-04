#ifndef QUAVIS_GEOMETRY_H
#define QUAVIS_GEOMETRY_H

namespace quavis {
  typedef struct vec2 {
    float x;
    float y;
  } vec2;

  typedef struct vec3 {
    float x;
    float y;
    float z;
  } vec3;

  typedef struct mat4 {
    float data[16];
  } mat4;

  struct Vertex {
      vec3 pos;
      vec3 color;

      static VkVertexInputBindingDescription getBindingDescription() {
          VkVertexInputBindingDescription bindingDescription = {};
          bindingDescription.binding = 0;
          bindingDescription.stride = sizeof(Vertex);
          bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
          return bindingDescription;
      }

      static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
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
}

#endif
