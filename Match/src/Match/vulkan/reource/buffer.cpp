#include <Match/vulkan/resource/buffer.hpp>
#include <Match/core/utils.hpp>
#include "../inner.hpp"

namespace Match {
    Buffer::Buffer(uint64_t size, vk::BufferUsageFlags buffer_usage, VmaMemoryUsage vma_usage, VmaAllocationCreateFlags vma_flags) : size(size), mapped(false), data_ptr(nullptr) {
        vk::BufferCreateInfo buffer_create_info {};
        buffer_create_info.setUsage(buffer_usage)
            .setSize(size);
        VmaAllocationCreateInfo buffer_alloc_info {};
        buffer_alloc_info.flags = vma_flags;
        buffer_alloc_info.usage = vma_usage;
        vmaCreateBuffer(manager->vma_allocator, reinterpret_cast<VkBufferCreateInfo *>(&buffer_create_info), &buffer_alloc_info, reinterpret_cast<VkBuffer *>(&buffer), &buffer_allocation, nullptr);
    }

    Buffer::Buffer(Buffer &&rhs) {
        size = rhs.size;
        mapped = rhs.mapped;
        data_ptr = rhs.data_ptr;
        buffer = rhs.buffer;
        buffer_allocation = rhs.buffer_allocation;
        rhs.size = 0;
        rhs.mapped = false;
        rhs.data_ptr = nullptr;
        rhs.buffer = VK_NULL_HANDLE;
        rhs.buffer_allocation = NULL;
    }

    void *Buffer::map() {
        if (mapped) {
            return data_ptr;
        }
        mapped = true;
        vmaMapMemory(manager->vma_allocator, buffer_allocation, &data_ptr);
        return data_ptr;
    }

    bool Buffer::is_mapped() {
        return mapped;
    }

    void Buffer::unmap() {
        if (!mapped) {
            return;
        }
        mapped = false;
        vmaUnmapMemory(manager->vma_allocator, buffer_allocation);
    }

    Buffer::~Buffer() {
        unmap();
        vmaDestroyBuffer(manager->vma_allocator, buffer, buffer_allocation);
    }

    InFlightBuffer::InFlightBuffer(uint64_t size, vk::BufferUsageFlags buffer_usage, VmaMemoryUsage vma_usage, VmaAllocationCreateFlags vma_flags) {
        in_flight_buffers.reserve(setting.max_in_flight_frame);
        for (uint32_t i = 0; i < setting.max_in_flight_frame; i ++) {
            in_flight_buffers.push_back(std::make_unique<Buffer>(size, buffer_usage, vma_usage, vma_flags));
        }
    }

    void *InFlightBuffer::map() {
        return in_flight_buffers[runtime_setting->current_in_flight]->map();
    }

    void InFlightBuffer::unmap() {
        in_flight_buffers[runtime_setting->current_in_flight]->unmap();
    }

    InFlightBuffer::~InFlightBuffer() {
        in_flight_buffers.clear();
    }

    TwoStageBuffer::TwoStageBuffer(uint64_t size, vk::BufferUsageFlags usage, vk::BufferUsageFlags additional_usage) {
        staging = std::make_unique<Buffer>(size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, 0);
        buffer = std::make_unique<Buffer>(size, usage | vk::BufferUsageFlagBits::eTransferDst | additional_usage, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
    }

    void *TwoStageBuffer::map() {
        return staging->map();
    }

    void TwoStageBuffer::flush() {
        auto command_buffer = manager->command_pool->allocate_single_use();
        vk::BufferCopy copy {};
        copy.setSrcOffset(0)
            .setDstOffset(0)
            .setSize(staging->size);
        command_buffer.copyBuffer(staging->buffer, buffer->buffer, copy);
        manager->command_pool->free_single_use(command_buffer);
    }

    void TwoStageBuffer::unmap() {
        staging->unmap();
    }

    TwoStageBuffer::~TwoStageBuffer() {
        staging.reset();
        buffer.reset();
    }

    VertexBuffer::VertexBuffer(uint32_t vertex_size, uint32_t count, vk::BufferUsageFlags additional_usage) : TwoStageBuffer(vertex_size * count, vk::BufferUsageFlagBits::eVertexBuffer, additional_usage) {}
    IndexBuffer::IndexBuffer(IndexType type, uint32_t count, vk::BufferUsageFlags additional_usage) : type(transform<vk::IndexType>(type)), TwoStageBuffer(transform<uint32_t>(type) * count, vk::BufferUsageFlagBits::eIndexBuffer, additional_usage) {}
}
