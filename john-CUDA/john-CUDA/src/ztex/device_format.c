/*
 * This software is Copyright (c) 2016 Denis Burykin
 * [denis_burykin yahoo com], [denis-burykin2014 yandex ru]
 * and it is hereby released to the general public under the following terms:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../loader.h"
#include "../formats.h"
#include "../memory.h"
#include "../misc.h"
#include "../options.h"

#include "jtr_device.h"
#include "task.h"
#include "jtr_mask.h"
#include "device_bitstream.h"

// If some task is not completed in this many seconds,
// then it counts the device as not functioning one.
#define DEVICE_TASK_TIMEOUT	5

/*
 * keys_buffer. In mask mode, range_info_buffer also used.
 */
char *keys_buffer;
unsigned char *range_info_buffer;

/*
 * Used by get_key() to transfer the output
 */
char *output_key;

/*
 * Task.
 * - includes all the data required to perform computation
 * - is assigned to some device (part of the device) independent from
 * other devices from the point of view from JtR's core (jtr_device)
 * - processed task (status == TASK_COMPLETE) can include the result.
 *
 * Currently there's a global task_list until some further improvement
 * such as tasks are split into batches or alike.
 */
struct task_list *task_list;

/*
 * Saved by device_format_init()
 */
struct fmt_params *jtr_fmt_params;
struct device_bitstream *jtr_bitstream;


void device_format_init(struct fmt_main *fmt_main, struct device_bitstream *bitstream)
{
	jtr_fmt_params = &fmt_main->params;
	jtr_bitstream = bitstream;
	
	// Initialize hardware.
	// Uses globals: jtr_device_list, device_list, jtr_bitstream.
	if (!jtr_device_list_init())
		error();

		
	// Mask issues. 1 mask per JtR run can be specified. Global variables.
	 
	// Mask initialization (mask_int_cand_target). 
	// - Without this, mask is completely unrolled on CPU.
	// - With this value set, some of ranges get unrolled, so remaining ranges
	//   result in approximate this number (can be greater or less than)
	//   of candidates per key.
	// - If the number is large enough, ranges don't get unrolled on CPU.
	// - If mask has more than MASK_FMT_INT_PLHDR ranges, extra ranges
	//   are unrolled regardless of mask_int_cand_target.

	// Mask can create too many candidates. That would result in problems
	// with slow response and not enough keys for distribution among devices.
	// crypt_all() must finish in some reasonable 'response time'
	// such as 0.1-0.2s.
	
	// Reduce mask (request to unroll some ranges on CPU if necessary)
	// by setting mask_int_cand_target.
	//
	mask_int_cand_target = jtr_bitstream->candidates_per_crypt;
	
	// It requires actual mask size (number of candidates in mask)
	// to calculate max_keys_per_crypt.
	// On init(), mask is not ready yet. The following does not work.
	//unsigned int cand_max = mask_estimate_num_cand_max();	
}


void device_format_done()
{
	MEM_FREE(keys_buffer);
	MEM_FREE(range_info_buffer);
}


extern volatile int bench_running;

void device_format_reset()
{
	// Mask data is ready, calculate and set keys_per_crypt
	unsigned int keys_per_crypt = jtr_bitstream->candidates_per_crypt
			/ mask_num_cand();
	if (!keys_per_crypt)
		keys_per_crypt = 1;

	keys_per_crypt *= jtr_device_list_count();
	if (keys_per_crypt > jtr_bitstream->abs_max_keys_per_crypt) {
		keys_per_crypt = jtr_bitstream->abs_max_keys_per_crypt;
		
		if (!bench_running) // self-test or benchmark
			fprintf(stderr, "Warning: Slow communication channel "\
				"to the device. "\
				"Increase mask or expect performance degradation.\n");
	}

	jtr_fmt_params->max_keys_per_crypt = keys_per_crypt;
	jtr_fmt_params->min_keys_per_crypt = keys_per_crypt;

	//fprintf(stderr, "RESET: mask_num_cand():%d keys_per_crypt:%d\n",
	//		mask_num_cand(), jtr_fmt_params->max_keys_per_crypt);


	// (re-)allocate keys_buffer, output_key
	int plaintext_len = jtr_fmt_params->plaintext_length;
	
	MEM_FREE(keys_buffer);
	keys_buffer = mem_alloc(plaintext_len
			* jtr_fmt_params->max_keys_per_crypt);
	
	MEM_FREE(output_key);
	output_key = mem_alloc(plaintext_len + 1);
	output_key[plaintext_len] = 0;

	MEM_FREE(range_info_buffer);
	if (!mask_is_inactive())
		range_info_buffer = mem_alloc(MASK_FMT_INT_PLHDR
				* jtr_fmt_params->max_keys_per_crypt);
}


void device_format_set_salt(void *salt)
{
}


void device_format_set_key(char *key, int index)
{
	// Copy key into buffer.
	memcpy(keys_buffer + index * jtr_fmt_params->plaintext_length,
			key, jtr_fmt_params->plaintext_length);

	//mask_print();
	//fprintf(stderr, "set_key:%s\n", key);

	// Copy mask data for template key into range_info_buffer.
	if (!mask_is_inactive())
		mask_set_range_info(range_info_buffer + index * MASK_FMT_INT_PLHDR);
}


int device_format_crypt_all(int *pcount, struct db_salt *salt)
{
	// * create tasks from keys_buffer and db_salt, 1 task for each jtr_device
	// * equally distribute load among tasks assuming all devices are equal
	// * assign tasks to jtr_devices
	// * global jtr_device_list used
	//
	if (task_list)
		task_list_delete(task_list);
	task_list = task_list_create(*pcount, keys_buffer,
			mask_is_inactive() ? NULL : range_info_buffer, salt);

	// Send data to devices, continue communication until result is received
	int rw_result;
	for (;;) {
	
		// If some devices were stopped then some tasks are unassigned.
		tasks_assign(task_list, jtr_device_list);

		// Perform r/w operations. Stop erroneous devices.
		rw_result = jtr_device_list_rw(task_list);
		
		// No operational devices remain - comment out next line
		// to wait until the device is up
		if (rw_result < 0)
			break;
		
		// Some tasks could be unable to complete for too long.
		struct timeval tv;
		gettimeofday(&tv, NULL);
		for(;;) {
			struct task *task = task_find_by_mtime(task_list,
					tv.tv_sec - DEVICE_TASK_TIMEOUT);
			if (!task)
				break;
			// Underlying physical device "silently" stopped operation.
			device_stop(task->jtr_device->device, task_list, "Timeout.");
		}

		// Process input packets, store results in task_result
		jtr_device_list_process_inpkt(task_list);
		
		// Computation done.
		if (task_list_all_completed(task_list))
			break;

		// There was no data transfer on devices.
		// Don't use 100% CPU in a loop.
		if (!rw_result)
			usleep(1000);
		
	}

	if (rw_result < 0) {
		fprintf(stderr, "No ZTEX devices available, exiting\n");
		error();
	}
	
	// Number of devices can change at runtime.
	// Dynamic adjustment of max_keys_per_crypt could be a good idea.

	*pcount *= mask_num_cand();

	return task_list_result_count(task_list);
}


int device_format_cmp_all(void *binary, int count)
{
	return !!count;
}


int device_format_cmp_one(void *binary, int index)
{
	struct task_result *result = task_result_by_index(task_list, index);
	if (!result) {
		fprintf(stderr,"device_format_cmp_one(%d): no result\n", index);
		error();
	}
	if (!result->binary) {
		fprintf(stderr,"device_format_cmp_one(%d): no binary\n", index);
		error();
	}

	return !memcmp(result->binary, binary, jtr_fmt_params->binary_size);
}


int device_format_cmp_exact(char *source, int index)
{
	// TODO
	return 1;
}


struct db_password *device_format_get_password(int index)
{
	struct task_result *result = task_result_by_index(task_list, index);
	if (!result) {
		fprintf(stderr, "get_password(%d): no task_result\n", index);
		return NULL;
	}
	if (!result->pw) {
		fprintf(stderr, "get_password(%d): no result->pw\n", index);
		return NULL;
	}
	return result->pw;
}


char *device_format_get_key(int index)
{
	// It happens status reporting is requested and there's 
	// a task_result at same index.
	if (task_list) {
		struct task_result *result = task_result_by_index(task_list, index);
		if (result)
			return result->key;
	}

	// There must be status reporting or self-test
	int plaintext_len = jtr_fmt_params->plaintext_length;

	if (mask_num_cand() > 1) {
		int key_num = index / mask_num_cand();
		int gen_id = index % mask_num_cand();
		memcpy(output_key, keys_buffer
				+ key_num * plaintext_len, plaintext_len);
		mask_reconstruct_plaintext(output_key, range_info_buffer
				+ key_num * MASK_FMT_INT_PLHDR, gen_id);

	} else {
		if (index < jtr_fmt_params->max_keys_per_crypt)
			memcpy(output_key, keys_buffer
					+ index * plaintext_len, plaintext_len);
		else
			return "-----";
	}
	
	return output_key;
}

