#pragma once

#include <Match/vulkan/commons.hpp>
#include <Match/vulkan/resource/sampler.hpp>
#include <Match/vulkan/descriptor_resource/uniform.hpp>
#include <Match/vulkan/descriptor_resource/texture.hpp>
#include <Match/vulkan/descriptor_resource/storage_buffer.hpp>
#include <Match/vulkan/descriptor_resource/storage_image.hpp>
#include <Match/vulkan/resource/ray_tracing_instance_collect.hpp>
#include <optional>

namespace Match {
    struct DescriptorInfo {
        DescriptorInfo(ShaderStages stages, uint32_t binding, DescriptorType type, uint32_t count, const vk::Sampler *immutable_samplers) : stages(stages), binding(binding), type(type), count(count), immutable_samplers(immutable_samplers) {}
        DescriptorInfo(ShaderStages stages, uint32_t binding, DescriptorType type, uint32_t count) : stages(stages), binding(binding), type(type), count(count), immutable_samplers(nullptr) {}
        DescriptorInfo(ShaderStages stages, uint32_t binding, DescriptorType type, const vk::Sampler *immutable_samplers) : stages(stages), binding(binding), type(type), count(1), immutable_samplers(immutable_samplers) {}
        DescriptorInfo(ShaderStages stages, uint32_t binding, DescriptorType type) : stages(stages), binding(binding), type(type), count(1), immutable_samplers(nullptr) {}
        ShaderStages stages;
        uint32_t binding;
        DescriptorType type;
        uint32_t count;
        const vk::Sampler *immutable_samplers;
    };

    class Renderer;

    class DescriptorSet {
        no_copy_move_construction(DescriptorSet)
    public:
        MATCH_API DescriptorSet(std::optional<std::weak_ptr<Renderer>> renderer);
        MATCH_API DescriptorSet &add_descriptors(const std::vector<DescriptorInfo> &descriptor_infos);
        MATCH_API DescriptorSet &allocate();
        MATCH_API DescriptorSet &free();
        MATCH_API DescriptorSet &bind_uniforms(uint32_t binding, const std::vector<std::shared_ptr<UniformBuffer>> &uniform_buffers);
        MATCH_API DescriptorSet &bind_uniform(uint32_t binding, std::shared_ptr<UniformBuffer> uniform_buffer);
        MATCH_API DescriptorSet &bind_textures(uint32_t binding, const std::vector<std::pair<std::shared_ptr<Texture>, std::shared_ptr<Sampler>>> &textures_samplers);
        MATCH_API DescriptorSet &bind_texture(uint32_t binding, std::shared_ptr<Texture> texture, std::shared_ptr<Sampler> sampler);
        MATCH_API DescriptorSet &bind_input_attachments(uint32_t binding, const std::vector<std::pair<std::string, std::shared_ptr<Sampler>>> &attachment_names_samplers);
        MATCH_API DescriptorSet &bind_input_attachment(uint32_t binding, const std::string &attachment_name, std::shared_ptr<Sampler> sampler);
        MATCH_API DescriptorSet &bind_storage_buffers(uint32_t binding, const std::vector<std::shared_ptr<StorageBuffer>> &storage_buffers);
        MATCH_API DescriptorSet &bind_storage_buffer(uint32_t binding, std::shared_ptr<StorageBuffer> storage_buffer);
        MATCH_API DescriptorSet &bind_storage_images(uint32_t binding, const std::vector<std::shared_ptr<StorageImage>> &storage_images);
        MATCH_API DescriptorSet &bind_storage_image(uint32_t binding, std::shared_ptr<StorageImage> storage_image);
        MATCH_API DescriptorSet &bind_ray_tracing_instance_collects(uint32_t binding, const std::vector<std::shared_ptr<RayTracingInstanceCollect>> &collects);
        MATCH_API DescriptorSet &bind_ray_tracing_instance_collect(uint32_t binding, std::shared_ptr<RayTracingInstanceCollect> collect);
        MATCH_API ~DescriptorSet();
        bool is_allocated() const { return allocated; }
    private:
        MATCH_API vk::DescriptorSetLayoutBinding &get_layout_binding(uint32_t binding);
        MATCH_API void update_input_attachments();
    INNER_VISIBLE:
        bool allocated;
        std::optional<std::weak_ptr<Renderer>> renderer;
        std::map<uint32_t, std::vector<std::pair<std::string, std::shared_ptr<Sampler>>>> input_attachments_temp;
        std::optional<uint32_t> callback_id;
        std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
        vk::DescriptorSetLayout descriptor_layout;
        std::vector<vk::DescriptorSet> descriptor_sets;
    };
}
