/*
 * Copyright 2011-2015 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

CCL_NAMESPACE_BEGIN

/* Note on kernel_lamp_emission
 * This is the 3rd kernel in the ray-tracing logic. This is the second of the
 * path-iteration kernels. This kernel takes care of the indirect lamp emission logic.
 * This kernel operates on QUEUE_ACTIVE_AND_REGENERATED_RAYS. It processes rays of state RAY_ACTIVE
 * and RAY_HIT_BACKGROUND.
 * We will empty QUEUE_ACTIVE_AND_REGENERATED_RAYS queue in this kernel.
 * The input/output of the kernel is as follows,
 * Throughput_coop ------------------------------------|--- kernel_lamp_emission --|--- PathRadiance_coop
 * Ray_coop -------------------------------------------|                           |--- Queue_data(QUEUE_ACTIVE_AND_REGENERATED_RAYS)
 * PathState_coop -------------------------------------|                           |--- Queue_index(QUEUE_ACTIVE_AND_REGENERATED_RAYS)
 * kg (globals) ---------------------------------------|                           |
 * Intersection_coop ----------------------------------|                           |
 * ray_state ------------------------------------------|                           |
 * Queue_data (QUEUE_ACTIVE_AND_REGENERATED_RAYS) -----|                           |
 * Queue_index (QUEUE_ACTIVE_AND_REGENERATED_RAYS) ----|                           |
 * queuesize ------------------------------------------|                           |
 * use_queues_flag ------------------------------------|                           |
 * sw -------------------------------------------------|                           |
 * sh -------------------------------------------------|                           |
 */
ccl_device void kernel_lamp_emission(KernelGlobals *kg)
{
	int x = ccl_global_id(0);
	int y = ccl_global_id(1);

	/* We will empty this queue in this kernel. */
	if(ccl_global_id(0) == 0 && ccl_global_id(1) == 0) {
		split_params->queue_index[QUEUE_ACTIVE_AND_REGENERATED_RAYS] = 0;
	}
	/* Fetch use_queues_flag. */
	ccl_local char local_use_queues_flag;
	if(ccl_local_id(0) == 0 && ccl_local_id(1) == 0) {
		local_use_queues_flag = split_params->use_queues_flag[0];
	}
	ccl_barrier(CCL_LOCAL_MEM_FENCE);

	int ray_index;
	if(local_use_queues_flag) {
		int thread_index = ccl_global_id(1) * ccl_global_size(0) + ccl_global_id(0);
		ray_index = get_ray_index(thread_index,
		                          QUEUE_ACTIVE_AND_REGENERATED_RAYS,
		                          split_state->queue_data,
		                          split_params->queue_size,
		                          1);
		if(ray_index == QUEUE_EMPTY_SLOT) {
			return;
		}
	} else {
		if(x < (split_params->w * split_params->parallel_samples) && y < split_params->h) {
			ray_index = x + y * (split_params->w * split_params->parallel_samples);
		} else {
			return;
		}
	}

	if(IS_STATE(split_state->ray_state, ray_index, RAY_ACTIVE) ||
	   IS_STATE(split_state->ray_state, ray_index, RAY_HIT_BACKGROUND))
	{
		PathRadiance *L = &split_state->path_radiance[ray_index];
		ccl_global PathState *state = &split_state->path_state[ray_index];

		float3 throughput = split_state->throughput[ray_index];
		Ray ray = split_state->ray[ray_index];

#ifdef __LAMP_MIS__
		if(kernel_data.integrator.use_lamp_mis && !(state->flag & PATH_RAY_CAMERA)) {
			/* ray starting from previous non-transparent bounce */
			Ray light_ray;

			light_ray.P = ray.P - state->ray_t*ray.D;
			state->ray_t += split_state->isect[ray_index].t;
			light_ray.D = ray.D;
			light_ray.t = state->ray_t;
			light_ray.time = ray.time;
			light_ray.dD = ray.dD;
			light_ray.dP = ray.dP;
			/* intersect with lamp */
			float3 emission;

			if(indirect_lamp_emission(kg, kg->sd_input, state, &light_ray, &emission)) {
				path_radiance_accum_emission(L, throughput, emission, state->bounce);
			}
		}
#endif  /* __LAMP_MIS__ */
	}
}

CCL_NAMESPACE_END

