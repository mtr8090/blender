/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2020 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup gpu
 */

#pragma once

#include "MEM_guardedalloc.h"

#include "gpu_vertex_buffer_private.hh"

namespace blender {
namespace gpu {

class VKVertBuf : public VertBuf {
  friend class VKTexture; /* For buffer texture. */

 public:
  void update_sub(uint start, uint len, void *data) override{};

 protected:
  void acquire_data(void) override{};
  void resize_data(void) override{};
  void release_data(void) override{};
  void upload_data(void) override{};
  void duplicate_data(VertBuf *dst) override{};

  MEM_CXX_CLASS_ALLOC_FUNCS("VKVertBuf");
};

}  // namespace gpu
}  // namespace blender
