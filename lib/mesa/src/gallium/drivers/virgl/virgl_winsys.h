/*
 * Copyright 2014, 2015 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef VIRGL_WINSYS_H
#define VIRGL_WINSYS_H

#include "pipe/p_defines.h"
#include "virgl_hw.h"

struct pipe_box;
struct pipe_fence_handle;
struct winsys_handle;
struct virgl_hw_res;

#define VIRGL_MAX_CMDBUF_DWORDS (64 * 1024)

struct virgl_drm_caps {
   union virgl_caps caps;
};

struct virgl_cmd_buf {
   unsigned cdw;
   uint32_t *buf;
   int in_fence_fd;
   bool needs_out_fence_fd;
};

struct virgl_winsys {
   unsigned pci_id;
   int supports_fences; /* In/Out fences are supported */

   void (*destroy)(struct virgl_winsys *vws);

   int (*transfer_put)(struct virgl_winsys *vws,
                       struct virgl_hw_res *res,
                       const struct pipe_box *box,
                       uint32_t stride, uint32_t layer_stride,
                       uint32_t buf_offset, uint32_t level);

   int (*transfer_get)(struct virgl_winsys *vws,
                       struct virgl_hw_res *res,
                       const struct pipe_box *box,
                       uint32_t stride, uint32_t layer_stride,
                       uint32_t buf_offset, uint32_t level);

   struct virgl_hw_res *(*resource_create)(struct virgl_winsys *vws,
                               enum pipe_texture_target target,
                               uint32_t format, uint32_t bind,
                               uint32_t width, uint32_t height,
                               uint32_t depth, uint32_t array_size,
                               uint32_t last_level, uint32_t nr_samples,
                               uint32_t size);

   void (*resource_unref)(struct virgl_winsys *vws, struct virgl_hw_res *res);

   void *(*resource_map)(struct virgl_winsys *vws, struct virgl_hw_res *res);
   void (*resource_wait)(struct virgl_winsys *vws, struct virgl_hw_res *res);

   struct virgl_hw_res *(*resource_create_from_handle)(struct virgl_winsys *vws,
                                                       struct winsys_handle *whandle);
   boolean (*resource_get_handle)(struct virgl_winsys *vws,
                                  struct virgl_hw_res *res,
                                  uint32_t stride,
                                  struct winsys_handle *whandle);

   struct virgl_cmd_buf *(*cmd_buf_create)(struct virgl_winsys *ws);
   void (*cmd_buf_destroy)(struct virgl_cmd_buf *buf);

   void (*emit_res)(struct virgl_winsys *vws, struct virgl_cmd_buf *buf, struct virgl_hw_res *res, boolean write_buffer);
   int (*submit_cmd)(struct virgl_winsys *vws, struct virgl_cmd_buf *buf,
                     int32_t in_fence_fd, int32_t *out_fence_fd);

   boolean (*res_is_referenced)(struct virgl_winsys *vws,
                                struct virgl_cmd_buf *buf,
                                struct virgl_hw_res *res);

   int (*get_caps)(struct virgl_winsys *vws, struct virgl_drm_caps *caps);

   /* fence */
   struct pipe_fence_handle *(*cs_create_fence)(struct virgl_winsys *vws, int fd);
   bool (*fence_wait)(struct virgl_winsys *vws,
                      struct pipe_fence_handle *fence,
                      uint64_t timeout);

   void (*fence_reference)(struct virgl_winsys *vws,
                           struct pipe_fence_handle **dst,
                           struct pipe_fence_handle *src);

   /* for sw paths */
   void (*flush_frontbuffer)(struct virgl_winsys *vws,
                             struct virgl_hw_res *res,
                             unsigned level, unsigned layer,
                             void *winsys_drawable_handle,
                             struct pipe_box *sub_box);
   void (*fence_server_sync)(struct virgl_winsys *vws,
                             struct virgl_cmd_buf *cbuf,
                             struct pipe_fence_handle *fence);

   int (*fence_get_fd)(struct virgl_winsys *vws,
                       struct pipe_fence_handle *fence);
};

/* this defaults all newer caps,
 * the kernel will overwrite these if newer version is available.
 */
static inline void virgl_ws_fill_new_caps_defaults(struct virgl_drm_caps *caps)
{
   caps->caps.v2.min_aliased_point_size = 1.f;
   caps->caps.v2.max_aliased_point_size = 255.f;
   caps->caps.v2.min_smooth_point_size = 1.f;
   caps->caps.v2.max_smooth_point_size = 190.f;
   caps->caps.v2.min_aliased_line_width = 1.f;
   caps->caps.v2.max_aliased_line_width = 10.f;
   caps->caps.v2.min_smooth_line_width = 0.f;
   caps->caps.v2.max_smooth_line_width = 10.f;
   caps->caps.v2.max_texture_lod_bias = 15.0f;
   caps->caps.v2.max_geom_output_vertices = 256;
   caps->caps.v2.max_geom_total_output_components = 1024;
   caps->caps.v2.max_vertex_outputs = 32;
   caps->caps.v2.max_vertex_attribs = 16;
   caps->caps.v2.max_shader_patch_varyings = 30;
   caps->caps.v2.min_texel_offset = -8;
   caps->caps.v2.max_texel_offset = 7;
   caps->caps.v2.min_texture_gather_offset = -8;
   caps->caps.v2.max_texture_gather_offset = 7;
   caps->caps.v2.texture_buffer_offset_alignment = 0;
   caps->caps.v2.uniform_buffer_offset_alignment = 256;
   caps->caps.v2.shader_buffer_offset_alignment = 32;
   caps->caps.v2.capability_bits = 0;
   caps->caps.v2.max_vertex_attrib_stride = 0;
   caps->caps.v2.max_image_samples = 0;
   caps->caps.v2.max_compute_work_group_invocations = 0;
   caps->caps.v2.max_compute_shared_memory_size = 0;
   caps->caps.v2.host_feature_check_version = 0;
}
#endif
