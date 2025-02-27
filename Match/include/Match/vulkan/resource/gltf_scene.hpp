#pragma once

#include <Match/vulkan/descriptor_resource/texture.hpp>
#include <Match/vulkan/resource/sampler.hpp>
#include <Match/vulkan/resource/model.hpp>
#include <glm/gtc/quaternion.hpp>
#include <tiny_gltf.h>

namespace Match {
    class GLTFScene;
    class GLTFMesh;
    class DescriptorSet;

    struct GLTFPrimitiveInstanceData {
        uint32_t first_index { 0 };
        uint32_t first_vertex { 0 };
        uint32_t material_index { 0 };
    };

    class GLTFPrimitive final : public RayTracingModel {
        no_copy_move_construction(GLTFPrimitive)
    public:
        MATCH_API GLTFPrimitive(GLTFScene &scene, const tinygltf::Model &gltf_model, const tinygltf::Primitive &gltf_primitive, const std::vector<std::string> &load_attributes);
        MATCH_API ~GLTFPrimitive();
        GLTFPrimitiveInstanceData get_primitive_instance_data() {
            return primitive_instance_data;
        }

        RayTracingModelType get_ray_tracing_model_type() override { return RayTracingModel::RayTracingModelType::eGLTFPrimitive; }
    INNER_VISIBLE:
        GLTFPrimitiveInstanceData primitive_instance_data {};
        uint32_t index_count { 0 };
        uint32_t vertex_count { 0 };
        GLTFScene &scene;

        glm::vec3 pos_min { 0 };
        glm::vec3 pos_max { 0 };
    };

    class GLTFMesh {
        no_copy_move_construction(GLTFMesh)
    public:
        MATCH_API GLTFMesh(GLTFScene &scene, const tinygltf::Model &gltf_model, const tinygltf::Mesh &gltf_mesh, const std::vector<std::string> &load_attributes);
        MATCH_API ~GLTFMesh();
    INNER_VISIBLE:
        std::string name {};
        std::vector<std::shared_ptr<GLTFPrimitive>> primitives {};
    };

    class GLTFNode {
        no_copy_move_construction(GLTFNode)
    public:
        MATCH_API GLTFNode(GLTFScene &scene, const tinygltf::Model &gltf_model, uint32_t gltf_node_index, GLTFNode *parent);
        MATCH_API ~GLTFNode();

        MATCH_API glm::mat4 get_local_matrix();
        MATCH_API glm::mat4 get_world_matrix();
    INNER_VISIBLE:
        std::string name {};
        std::shared_ptr<GLTFMesh> mesh { nullptr };

        GLTFNode *parent {};
        std::vector<std::shared_ptr<GLTFNode>> children {};

        glm::quat rotation { 0, 0, 0, 0 };
        glm::vec3 scale { 1 };
        glm::vec3 translation { 0 };
        glm::mat4 matrix { 1 };
    };

    struct GLTFMaterial {
        glm::vec4 base_color_factor { 1 };
        int base_color_texture { -1 };
        glm::vec3 emissive_factor { 0 };
        int emissive_texture { -1 };
        int normal_texture { -1 };
        float metallic_factor { 1 };
        float roughness_factor { 1 };
        int metallic_roughness_texture { -1 };
    };

    class GLTFScene final : public RayTracingScene {
        no_copy_move_construction(GLTFScene)
    public:
        MATCH_API GLTFScene(const std::string &filename, const std::vector<std::string> &load_attributes);
        MATCH_API ~GLTFScene();
        uint32_t get_textures_count() { return textures.size(); };
        MATCH_API void bind_textures(std::shared_ptr<DescriptorSet> descriptor_set, uint32_t binding);
        std::shared_ptr<Buffer> get_materials_buffer() { return material_buffer; }
        std::shared_ptr<Buffer> get_attribute_buffer(const std::string &attribute_name) { return attribute_buffer.at(attribute_name); }
        MATCH_API void enumerate_primitives(std::function<void(GLTFNode *node, std::shared_ptr<GLTFPrimitive>)> func);

        RayTracingSceneType get_ray_tracing_scene_type() override { return RayTracingSceneType::eGLTFScene; }
    private:
        MATCH_API void load_images(const tinygltf::Model &gltf_model);
        MATCH_API void load_materials(const tinygltf::Model &gltf_model);
    INNER_VISIBLE:
        std::string path;

        std::shared_ptr<Sampler> sampler;
        std::vector<std::shared_ptr<Texture>> textures;
        std::vector<std::shared_ptr<GLTFMesh>> meshes;
        std::vector<std::shared_ptr<GLTFNode>> nodes;
        std::vector<GLTFNode *> all_node_references;

        std::vector<GLTFMaterial> materials;
        std::shared_ptr<Buffer> material_buffer;
        std::map<std::string, std::vector<uint8_t>> attribute_datas;
        std::map<std::string, std::shared_ptr<Buffer>> attribute_buffer;

        std::vector<glm::vec3> positions;
        std::vector<uint32_t> indices;
    INNER_VISIBLE:
        // using by ray_tracing_acceration_structure
        std::unique_ptr<Buffer> vertex_buffer;
        std::unique_ptr<Buffer> index_buffer;
    };
}
