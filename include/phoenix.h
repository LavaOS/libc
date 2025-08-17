#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct { uint64_t id; } phx_handle;

typedef phx_handle phx_instance;
typedef phx_handle phx_device;
typedef phx_handle phx_swapchain;
typedef phx_handle phx_image;
typedef phx_handle phx_buffer;
typedef phx_handle phx_pipeline;
typedef phx_handle phx_cmdlist;

typedef struct {
    void* ptr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t format;
} phx_platform_fb;

typedef struct { uint32_t width, height; } phx_swapchain_desc;
typedef struct { uint32_t width, height; } phx_image_desc;
typedef struct { size_t size; } phx_buffer_desc;
typedef struct { float data[16]; } phx_push_constants;

phx_instance phxCreateInstance(void);
phx_device phxCreateDevice(phx_instance i);
phx_swapchain phxCreateSwapchain(phx_device d, const phx_platform_fb* fb, const phx_swapchain_desc* desc);
phx_image phxCreateImage(phx_device d, const phx_image_desc* desc);
phx_buffer phxCreateBuffer(phx_device d, const phx_buffer_desc* desc);
phx_pipeline phxCreatePipelineBasic(phx_device d);

phx_cmdlist phxBeginCommands(phx_device d);
void phxCmdClearColor(phx_cmdlist cl, phx_image img, float r, float g, float b, float a);
void phxCmdBindPipeline(phx_cmdlist cl, phx_pipeline p);
void phxCmdBindVertexBuffer(phx_cmdlist cl, phx_buffer buf);
void phxCmdPushConstants(phx_cmdlist cl, const phx_push_constants* pc);
void phxCmdDraw(phx_cmdlist cl, uint32_t vtx_count, uint32_t first);
void phxEndCommands(phx_cmdlist cl);

void* phxAcquireNextImage(phx_swapchain sc, phx_image* out_img);
void phxSubmit(phx_device d, phx_cmdlist cl);
void phxPresent(phx_swapchain sc);
