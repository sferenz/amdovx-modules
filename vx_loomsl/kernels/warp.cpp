/*
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "warp.h"

#define WRITE_LUMA_AS_A 1

//! \brief The input validator callback.
static vx_status VX_CALLBACK warp_input_validator(vx_node node, vx_uint32 index)
{
	vx_status status = VX_ERROR_INVALID_PARAMETERS;
	// get reference for parameter at specified index
	vx_reference ref = avxGetNodeParamRef(node, index);
	ERROR_CHECK_OBJECT(ref);
	// validate each parameter
	if (index == 0)
	{ // object of SCALAR type
		vx_enum itemtype = VX_TYPE_INVALID;
		ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)ref, VX_SCALAR_ATTRIBUTE_TYPE, &itemtype, sizeof(itemtype)));
		ERROR_CHECK_STATUS(vxReleaseScalar((vx_scalar *)&ref));
		if (itemtype == VX_TYPE_ENUM) {
			status = VX_SUCCESS;
		}
		else {
			status = VX_ERROR_INVALID_TYPE;
			vxAddLogEntry((vx_reference)node, status, "ERROR: warp num_cameras scalar type should be an ENUM\n");
		}
	}
	if (index == 1)
	{ // object of SCALAR type
		vx_enum itemtype = VX_TYPE_INVALID;
		ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)ref, VX_SCALAR_ATTRIBUTE_TYPE, &itemtype, sizeof(itemtype)));
		ERROR_CHECK_STATUS(vxReleaseScalar((vx_scalar *)&ref));
		if (itemtype == VX_TYPE_UINT32) {
			status = VX_SUCCESS;
		}
		else {
			status = VX_ERROR_INVALID_TYPE;
			vxAddLogEntry((vx_reference)node, status, "ERROR: warp num_cameras scalar type should be a UINT32\n");
		}
	}
	else if (index == 2)
	{ // array object of StitchValidPixelEntry type
		vx_size itemsize = 0;
		ERROR_CHECK_STATUS(vxQueryArray((vx_array)ref, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemsize, sizeof(itemsize)));
		if (itemsize == sizeof(StitchValidPixelEntry)) {
			status = VX_SUCCESS;
		}
		else {
			status = VX_ERROR_INVALID_DIMENSION;
			vxAddLogEntry((vx_reference)node, status, "ERROR: warp array element (StitchWarpRemapEntry) size should be 32 bytes\n");
		}
		ERROR_CHECK_STATUS(vxReleaseArray((vx_array *)&ref));
		
	}
	else if (index == 3)
	{ // array object of StitchWarpRemapEntry type
		vx_size itemsize = 0;
		ERROR_CHECK_STATUS(vxQueryArray((vx_array)ref, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemsize, sizeof(itemsize)));
		if (itemsize == sizeof(StitchWarpRemapEntry)) {
			status = VX_SUCCESS;
		}
		else {
			status = VX_ERROR_INVALID_DIMENSION;
			vxAddLogEntry((vx_reference)node, status, "ERROR: warp array element (StitchWarpRemapEntry) size should be 32 bytes\n");
		}
		ERROR_CHECK_STATUS(vxReleaseArray((vx_array *)&ref));

	}
	else if (index == 4)
	{ // image of format RGB or RGBX
		// check input image format and dimensions
		vx_uint32 input_width = 0, input_height = 0;
		vx_df_image input_format = VX_DF_IMAGE_VIRT;
		ERROR_CHECK_STATUS(vxQueryImage((vx_image)ref, VX_IMAGE_ATTRIBUTE_WIDTH, &input_width, sizeof(input_width)));
		ERROR_CHECK_STATUS(vxQueryImage((vx_image)ref, VX_IMAGE_ATTRIBUTE_HEIGHT, &input_height, sizeof(input_height)));
		ERROR_CHECK_STATUS(vxQueryImage((vx_image)ref, VX_IMAGE_ATTRIBUTE_FORMAT, &input_format, sizeof(input_format)));
		ERROR_CHECK_STATUS(vxReleaseImage((vx_image *)&ref));
		if (input_format != VX_DF_IMAGE_RGB && input_format != VX_DF_IMAGE_RGBX && input_format != VX_DF_IMAGE_RGB4_AMD && input_format != VX_DF_IMAGE_RGB6_AMD) {
			status = VX_ERROR_INVALID_TYPE;
			vxAddLogEntry((vx_reference)node, status, "ERROR: warp doesn't support input image format: %4.4s\n", &input_format);
		}
		else {
			status = VX_SUCCESS;
		}
	}
	else if (index == 7)
	{ // object of SCALAR type (UINT32) for num_camera_columns
		status = VX_SUCCESS;
		if (ref) {
			vx_enum itemtype = VX_TYPE_INVALID;
			ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)ref, VX_SCALAR_ATTRIBUTE_TYPE, &itemtype, sizeof(itemtype)));
			ERROR_CHECK_STATUS(vxReleaseScalar((vx_scalar *)&ref));
			if (itemtype != VX_TYPE_UINT32) {
				status = VX_ERROR_INVALID_TYPE;
				vxAddLogEntry((vx_reference)node, status, "ERROR: warp num_camera_columns scalar type should be a UINT32\n");
			}
		}
	}
	else if (index == 8)
	{ // object of SCALAR type (UINT8) for alpha_value
		status = VX_SUCCESS;
		if (ref) {
			vx_enum itemtype = VX_TYPE_INVALID;
			ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)ref, VX_SCALAR_ATTRIBUTE_TYPE, &itemtype, sizeof(itemtype)));
			ERROR_CHECK_STATUS(vxReleaseScalar((vx_scalar *)&ref));
			if (itemtype != VX_TYPE_UINT8) {
				status = VX_ERROR_INVALID_TYPE;
				vxAddLogEntry((vx_reference)node, status, "ERROR: warp alpha_value scalar type should be a UINT8\n");
			}
			ref = avxGetNodeParamRef(node, 4);
			vx_df_image input_format = VX_DF_IMAGE_VIRT;
			ERROR_CHECK_STATUS(vxQueryImage((vx_image)ref, VX_IMAGE_ATTRIBUTE_FORMAT, &input_format, sizeof(input_format)));
			ERROR_CHECK_STATUS(vxReleaseImage((vx_image *)&ref));
			if (input_format != VX_DF_IMAGE_RGB) {
				status = VX_ERROR_INVALID_PARAMETERS;
				vxAddLogEntry((vx_reference)node, status, "ERROR: warp doesn't support external alpha_value for non RGB input image format\n");
			}
		}
	}
	else if (index == 9)
	{ // object of SCALAR type (UINT8) for flags
		status = VX_SUCCESS;
		if (ref) {
			vx_enum itemtype = VX_TYPE_INVALID;
			ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)ref, VX_SCALAR_ATTRIBUTE_TYPE, &itemtype, sizeof(itemtype)));
			ERROR_CHECK_STATUS(vxReleaseScalar((vx_scalar *)&ref));
			if (itemtype != VX_TYPE_UINT8) {
				status = VX_ERROR_INVALID_TYPE;
				vxAddLogEntry((vx_reference)node, status, "ERROR: warp flags scalar type should be a UINT8\n");
			}
		}
	}
	return status;
}

//! \brief The output validator callback.
static vx_status VX_CALLBACK warp_output_validator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
	vx_status status = VX_ERROR_INVALID_PARAMETERS;
	if (index == 5)
	{ // image of format RGB or RGBX
		vx_uint32 output_width = 0, output_height = 0;
		vx_df_image output_format = VX_DF_IMAGE_VIRT, input_format = VX_DF_IMAGE_VIRT;
		// get image configuration
		vx_image image = (vx_image)avxGetNodeParamRef(node, 5);
		ERROR_CHECK_OBJECT(image);
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &input_format, sizeof(output_format)));
		ERROR_CHECK_STATUS(vxReleaseImage(&image));
		image = (vx_image)avxGetNodeParamRef(node, index);
		ERROR_CHECK_OBJECT(image);
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &output_width, sizeof(output_width)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_height, sizeof(output_height)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &output_format, sizeof(output_format)));
		ERROR_CHECK_STATUS(vxReleaseImage(&image));
		if ((input_format == VX_DF_IMAGE_RGB || input_format == VX_DF_IMAGE_RGBX) && output_format != VX_DF_IMAGE_RGB && output_format != VX_DF_IMAGE_RGBX){
			// pick RGBX as default
			output_format = VX_DF_IMAGE_RGBX;
		}
		else if ((input_format == VX_DF_IMAGE_RGB4_AMD || input_format == VX_DF_IMAGE_RGB6_AMD) && output_format != VX_DF_IMAGE_RGB4_AMD && output_format != VX_DF_IMAGE_RGB6_AMD){
			// pick RGBX as default
			output_format = VX_DF_IMAGE_RGB6_AMD;
		}
		// set output image meta data
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &output_width, sizeof(output_width)));
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_height, sizeof(output_height)));
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &output_format, sizeof(output_format)));
		status = VX_SUCCESS;
	}
	else if (index == 6)
	{ // optional image of format U8
		vx_uint32 output_width = 0, output_height = 0, output_u8_width = 0, output_u8_height = 0;
		vx_df_image output_u8_format = VX_DF_IMAGE_VIRT, output_format = VX_DF_IMAGE_VIRT;
		// get image configuration
		vx_image image = (vx_image)avxGetNodeParamRef(node, 5);
		ERROR_CHECK_OBJECT(image);
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &output_width, sizeof(output_width)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_height, sizeof(output_height)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &output_format, sizeof(output_format)));
		ERROR_CHECK_STATUS(vxReleaseImage(&image));
		image = (vx_image)avxGetNodeParamRef(node, index);
		ERROR_CHECK_OBJECT(image);
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &output_u8_format, sizeof(output_u8_format)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &output_u8_width, sizeof(output_u8_width)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_u8_height, sizeof(output_u8_height)));
		ERROR_CHECK_STATUS(vxReleaseImage(&image));
		if ((output_format == VX_DF_IMAGE_RGB || output_format == VX_DF_IMAGE_RGBX) && output_u8_format != VX_DF_IMAGE_U8) {
			// pick U8 as default
			output_u8_format = VX_DF_IMAGE_U8;
		}
		else if ((output_format == VX_DF_IMAGE_RGB4_AMD || output_format == VX_DF_IMAGE_RGB6_AMD) && output_u8_format != VX_DF_IMAGE_U8 && output_u8_format != VX_DF_IMAGE_S16) {
			// pick U16 as default
			output_u8_format = VX_DF_IMAGE_S16;
		}
		if (output_u8_width != output_width || output_u8_height != output_height)
		{
			output_u8_width = output_width;
			output_u8_height = output_height;
		}
		// set output image meta data
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &output_u8_width, sizeof(output_u8_width)));
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_u8_height, sizeof(output_u8_height)));
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &output_u8_format, sizeof(output_u8_format)));
		status = VX_SUCCESS;
	}
	else if (index == 10)
	{ // optional image of format S16 for exposure comp
		vx_uint32 output_width = 0, output_height = 0, output_S16_width = 0, output_S16_height = 0;
		vx_df_image output_S16_format = VX_DF_IMAGE_VIRT, output_format = VX_DF_IMAGE_VIRT;
		// get image configuration
		vx_image image = (vx_image)avxGetNodeParamRef(node, 5);
		ERROR_CHECK_OBJECT(image);
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &output_width, sizeof(output_width)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_height, sizeof(output_height)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &output_format, sizeof(output_format)));
		ERROR_CHECK_STATUS(vxReleaseImage(&image));
		image = (vx_image)avxGetNodeParamRef(node, index);
		ERROR_CHECK_OBJECT(image);
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &output_S16_format, sizeof(output_S16_format)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &output_S16_width, sizeof(output_S16_width)));
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_S16_height, sizeof(output_S16_height)));
		ERROR_CHECK_STATUS(vxReleaseImage(&image));

		status = VX_SUCCESS;
		if (output_format != VX_DF_IMAGE_RGB4_AMD && output_format != VX_DF_IMAGE_RGB6_AMD){ // Check for 16 bit mode
			vxAddLogEntry((vx_reference)node, status, "ERROR: warp S16 output for exposure compensation is only avaible in 16 bit mode\n");
			status = VX_ERROR_INVALID_TYPE;
		}
		if (output_S16_format != VX_DF_IMAGE_S16){ // Pick S16 output format as default
			output_S16_format = VX_DF_IMAGE_S16;
		}
		if (output_S16_width != output_width || output_S16_height != output_height)
		{
			output_S16_width = output_width;
			output_S16_height = output_height;
		}
		// set output image meta data
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &output_S16_width, sizeof(output_S16_width)));
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_S16_height, sizeof(output_S16_height)));
		ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &output_S16_format, sizeof(output_S16_format)));
	}
	return status;
}

//! \brief The kernel target support callback.
static vx_status VX_CALLBACK warp_query_target_support(vx_graph graph, vx_node node,
	vx_bool use_opencl_1_2,              // [input]  false: OpenCL driver is 2.0+; true: OpenCL driver is 1.2
	vx_uint32& supported_target_affinity // [output] must be set to AGO_TARGET_AFFINITY_CPU or AGO_TARGET_AFFINITY_GPU or (AGO_TARGET_AFFINITY_CPU | AGO_TARGET_AFFINITY_GPU)
	)
{
	supported_target_affinity = AGO_TARGET_AFFINITY_GPU;
	return VX_SUCCESS;
}

//! \brief The OpenCL global work updater callback.
static vx_status VX_CALLBACK warp_opencl_global_work_update(
	vx_node node,                                  // [input] node
	const vx_reference parameters[],               // [input] parameters
	vx_uint32 num,                                 // [input] number of parameters
	vx_uint32 opencl_work_dim,                     // [input] work_dim for clEnqueueNDRangeKernel()
	vx_size opencl_global_work[],                  // [output] global_work[] for clEnqueueNDRangeKernel()
	const vx_size opencl_local_work[]              // [input] local_work[] for clEnqueueNDRangeKernel()
	)
{
	// Get the number of elements in the stitchWarpRemapEntry array
	vx_size arr_numitems = 0;
	vx_array arr = (vx_array)avxGetNodeParamRef(node, 2);				// input array
	ERROR_CHECK_OBJECT(arr);
	ERROR_CHECK_STATUS(vxQueryArray(arr, VX_ARRAY_ATTRIBUTE_NUMITEMS, &arr_numitems, sizeof(arr_numitems)));
	ERROR_CHECK_STATUS(vxReleaseArray(&arr));

	// Check for interpolation method
	vx_uint8 flags = 0;
	vx_scalar s_flags = (vx_scalar)parameters[9];
	if (s_flags) {
		ERROR_CHECK_STATUS(vxReadScalarValue(s_flags, &flags));
	}
	bool useBilinearInterpolation = (flags & 1) ? false : true;

	// Get input format
	vx_df_image input_format = VX_DF_IMAGE_VIRT;
	vx_image image = (vx_image)avxGetNodeParamRef(node, 4);				// input image
	ERROR_CHECK_OBJECT(image);
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &input_format, sizeof(input_format)));
	ERROR_CHECK_STATUS(vxReleaseImage(&image));

	opencl_global_work[0] = ((arr_numitems << 1) + opencl_local_work[0] - 1) & ~(opencl_local_work[0] - 1);

	if ((input_format == VX_DF_IMAGE_RGB4_AMD || input_format == VX_DF_IMAGE_RGB6_AMD) && !useBilinearInterpolation)
	{
		opencl_global_work[0] = 2 * opencl_global_work[0];
	}

	return VX_SUCCESS;
}

//! \brief The OpenCL code generator callback.
static vx_status VX_CALLBACK warp_opencl_codegen(
	vx_node node,                                  // [input] node
	const vx_reference parameters[],               // [input] parameters
	vx_uint32 num,                                 // [input] number of parameters
	bool opencl_load_function,                     // [input]  false: normal OpenCL kernel; true: reserved
	char opencl_kernel_function_name[64],          // [output] kernel_name for clCreateKernel()
	std::string& opencl_kernel_code,               // [output] string for clCreateProgramWithSource()
	std::string& opencl_build_options,             // [output] options for clBuildProgram()
	vx_uint32& opencl_work_dim,                    // [output] work_dim for clEnqueueNDRangeKernel()
	vx_size opencl_global_work[],                  // [output] global_work[] for clEnqueueNDRangeKernel()
	vx_size opencl_local_work[],                   // [output] local_work[] for clEnqueueNDRangeKernel()
	vx_uint32& opencl_local_buffer_usage_mask,     // [output] reserved: must be ZERO
	vx_uint32& opencl_local_buffer_size_in_bytes   // [output] reserved: must be ZERO
	)
{
	// get input and output image configurations
	vx_uint32 num_cameras = 0, num_camera_columns = 1;
	vx_uint8 flags = 0;
	vx_size arr_capacity = 0;
	vx_enum grayscale_compute_method;
	vx_df_image input_format = VX_DF_IMAGE_VIRT, output_format = VX_DF_IMAGE_VIRT, output_U_format = VX_DF_IMAGE_VIRT;
	bool bWriteU8Image = false;
	bool bWriteExpCompImage = false;
	vx_scalar scalar = (vx_scalar)avxGetNodeParamRef(node, 0);			// input scalar - grayscale compute method
	ERROR_CHECK_OBJECT(scalar);
	ERROR_CHECK_STATUS(vxReadScalarValue(scalar, &grayscale_compute_method));
	ERROR_CHECK_STATUS(vxReleaseScalar(&scalar));
	scalar = (vx_scalar)avxGetNodeParamRef(node, 1);			// input scalar - num cameras
	ERROR_CHECK_OBJECT(scalar);
	ERROR_CHECK_STATUS(vxReadScalarValue(scalar, &num_cameras));
	ERROR_CHECK_STATUS(vxReleaseScalar(&scalar));
	vx_array arr = (vx_array)avxGetNodeParamRef(node, 2);				// input array
	ERROR_CHECK_OBJECT(arr);
	ERROR_CHECK_STATUS(vxQueryArray(arr, VX_ARRAY_ATTRIBUTE_CAPACITY, &arr_capacity, sizeof(arr_capacity)));
	ERROR_CHECK_STATUS(vxReleaseArray(&arr));
	vx_image image = (vx_image)avxGetNodeParamRef(node, 4);				// input image
	vx_uint32 input_height = 0, output_height = 0;
	ERROR_CHECK_OBJECT(image);
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &input_format, sizeof(input_format)));
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &input_height, sizeof(input_height)));
	ERROR_CHECK_STATUS(vxReleaseImage(&image));
	image = (vx_image)avxGetNodeParamRef(node, 5);						// output image
	ERROR_CHECK_OBJECT(image);
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &output_format, sizeof(output_format)));
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &output_height, sizeof(output_height)));
	ERROR_CHECK_STATUS(vxReleaseImage(&image));

	// Check if output U8 image is specified
	image = (vx_image)avxGetNodeParamRef(node, 6);
	if (image != nullptr) {
		bWriteU8Image = true;
		ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &output_U_format, sizeof(output_format)));
		ERROR_CHECK_STATUS(vxReleaseImage(&image));
	}
	// Check if output S16 image for exposure compensation is specified
	image = (vx_image)avxGetNodeParamRef(node, 10);
	if (image != nullptr) {
		bWriteExpCompImage = true;
		ERROR_CHECK_STATUS(vxReleaseImage(&image));
	}

	vx_scalar s_num_camera_columns = (vx_scalar)parameters[7];
	if (s_num_camera_columns) {
		// read num_camera_columns
		ERROR_CHECK_STATUS(vxReadScalarValue(s_num_camera_columns, &num_camera_columns));
	}
	vx_scalar s_alpha_value = (vx_scalar)parameters[8];

	// Check for interpolation method
	vx_scalar s_flags = (vx_scalar)parameters[9];
	if (s_flags) {
		ERROR_CHECK_STATUS(vxReadScalarValue(s_flags, &flags));
	}
	bool useBilinearInterpolation = (flags & 1) ? false : true;

	// Calculate number of pixels to process
	bool pixels4 = true;
	if ((input_format == VX_DF_IMAGE_RGB4_AMD || input_format == VX_DF_IMAGE_RGB6_AMD) && !useBilinearInterpolation)
	{
		pixels4 = false;
	}

	// set kernel configuration
	vx_uint32 work_items = (vx_uint32)arr_capacity << 1;
	strcpy(opencl_kernel_function_name, "warp");
	opencl_work_dim = 1;
	opencl_local_work[0] = 64;
	opencl_global_work[0] = (work_items + opencl_local_work[0] - 1) & ~(opencl_local_work[0] - 1);
	if (!pixels4)
	{
		opencl_global_work[0] = 2*opencl_global_work[0];
	}
	
	vx_uint32 ip_image_height_offs = (vx_uint32)(input_height / num_cameras);
	vx_uint32 op_image_height_offs = (vx_uint32)(output_height / num_cameras);
	// Setting variables required by the interface
	opencl_local_buffer_usage_mask = 0;
	opencl_local_buffer_size_in_bytes = 0;

	// kernel header and reading
	// TBD: remove the ifdefs once we reach a conclusion on which is a better approach for all hardwares
#if 0		// DBG for Vega: This approach does not read when sx and sy are invalid
	char item[8192];
	sprintf(item,
		"#pragma OPENCL EXTENSION cl_amd_media_ops : enable\n"
		"#pragma OPENCL EXTENSION cl_amd_media_ops2 : enable\n"
		"__kernel __attribute__((reqd_work_group_size(%d, 1, 1)))\n" // opencl_local_work[0]
		"void %s(uint grayscale_compute_method,\n" // opencl_kernel_function_name
		"        uint num_cameras,\n"
		"        __global char * valid_pix_buf, uint valid_pix_buf_offset, uint valid_pix_num_items,\n"
		"        __global char * warp_remap_buf, uint warp_remap_buf_offset, uint warp_remap_num_items,\n"
		"        uint ip_width, uint ip_height, __global uchar * ip_buf, uint ip_stride, uint ip_offset,\n"
		"        uint op_width, uint op_height, __global uchar * op_buf, uint op_stride, uint op_offset"
		, (int)opencl_local_work[0], opencl_kernel_function_name);
	opencl_kernel_code = item;
	if (bWriteU8Image) {
		opencl_kernel_code +=
			",\n"
			"        uint op_u8_width, uint op_u8_height, __global uchar * op_u8_buf, uint op_u8_stride, uint op_u8_offset";
	}
	if (s_num_camera_columns) {
		opencl_kernel_code +=
			",\n"
			"        uint num_camera_columns";
	}
	if (s_alpha_value) {
		opencl_kernel_code +=
			",\n"
			"        uint alpha";
	}
	sprintf(item,
		")\n"
		"{\n"
		"  int gid = get_global_id(0);\n"
		"  float4 f, mf; uint sx, sy, offset; uint4 outpix;\n"
		"  uint QF = 3;\n"
		"  uint QFB = (1 << QF) - 1; float QFM = 1.0f / (1 << QF);\n"
		"  uint ip_image_height_offset = %d;\n" // ip_image_height_offs
		"  uint op_image_height_offset = %d;\n" // op_image_height_offs
		, ip_image_height_offs, op_image_height_offs);
	opencl_kernel_code += item;
	if (bWriteU8Image)
	{
		sprintf(item,
			"  uint op_u8_image_height_offset = %d;\n" // op_image_height_offs
			, op_image_height_offs);
		opencl_kernel_code += item;
	}
	opencl_kernel_code +=
		"  warp_remap_buf += warp_remap_buf_offset + (gid << 4);\n"
		"  valid_pix_buf += valid_pix_buf_offset + ((gid >> 1) << 2);\n"
		"  if (((gid >> 1) < valid_pix_num_items)) {\n"
		"    uint pixelEntry = *(__global uint*) valid_pix_buf;\n"
		"    if(pixelEntry == 0xffffffff) return;\n"
		"    uint4 map = *(__global uint4 *) warp_remap_buf;\n"
		"    uint camera_id = pixelEntry & 0x1f; uint op_x = (pixelEntry >> 8) & 0x7ff; uint op_y = (pixelEntry >> 19) & 0x1fff;\n";
	if (num_camera_columns == 1)
		opencl_kernel_code += "    ip_buf += ip_offset + (camera_id * ip_image_height_offset * ip_stride);\n";
	else {
		int shiftAmount = 0;
		for (vx_uint32 i = 1; i < 6; i++) {
			if (num_camera_columns == (1u << i)) {
				shiftAmount = i;
				break;
			}
		}
		if (shiftAmount > 0)
			sprintf(item, "    ip_buf += ip_offset + ((camera_id >> %d) * ip_image_height_offset * ip_stride);\n", shiftAmount);
		else
			sprintf(item, "    ip_buf += ip_offset + ((camera_id / %d) * ip_image_height_offset * ip_stride);\n", num_camera_columns);
		opencl_kernel_code += item;
	}
	if (bWriteU8Image)
	{
		opencl_kernel_code += "    float4 Yval;\n";
#if WRITE_LUMA_AS_A
		opencl_kernel_code += "    float3 RGBToY = (float3)(0.2126f, 0.7152f, 0.0722f);\n";
#endif
	}
	if (input_format == VX_DF_IMAGE_RGB)
	{
		opencl_kernel_code +=
			"    uint3 px0, px1;\n"
			"    __global uchar * pt;\n";
		if (output_format == VX_DF_IMAGE_RGBX)
		{
			opencl_kernel_code +=
				"    uint invalidPix = amd_pack((float4)(0.0f, 0.0f, 0.0f, 128.0f));\n"
				"    // pixel[0]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff;\n"
				"    if(!(sx == 0xffff && sy == 0xffff)) { offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; }\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
			if (s_alpha_value)
			{
				opencl_kernel_code += "    f.s3 = (float) alpha;\n";
			}
			else
			{
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
						"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
				}
			}
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, sx == 0xffff && sy == 0xffff);\n";
#else
				opencl_kernel_code += "    Yval.s0 = select(f.s3, 0.0f, sx == 0xffff && sy == 0xffff);\n";
#endif
			}
			opencl_kernel_code +=
				"    outpix.s0 = select(amd_pack(f), invalidPix, sx == 0xffff && sy == 0xffff);\n"
				"    // pixel[1]\n"
				"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff;\n"
				"    if(!(sx == 0xffff && sy == 0xffff)) { offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; }\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
			if (s_alpha_value)
			{
				opencl_kernel_code += "    f.s3 = (float) alpha;\n";
			}
			else
			{
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
						"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
				}
			}
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, sx == 0xffff && sy == 0xffff);\n";
#else
				opencl_kernel_code += "    Yval.s1 = select(f.s3, 0.0f, sx == 0xffff && sy == 0xffff);\n";
#endif
			}
			opencl_kernel_code +=
				"    outpix.s1 = select(amd_pack(f), invalidPix, sx == 0xffff && sy == 0xffff);\n\n"
				"    // pixel[2]\n"
				"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff;\n"
				"    if(!(sx == 0xffff && sy == 0xffff)) { offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; }\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
			if (s_alpha_value)
			{
				opencl_kernel_code += "    f.s3 = (float) alpha;\n";
			}
			else
			{
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
						"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
				}
			}
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, sx == 0xffff && sy == 0xffff);\n";
#else
				opencl_kernel_code += "    Yval.s2 = select(f.s3, 0.0f, sx == 0xffff && sy == 0xffff);\n";
#endif
			}
			opencl_kernel_code +=
				"    outpix.s2 = select(amd_pack(f), invalidPix, sx == 0xffff && sy == 0xffff);\n\n"
				"    // pixel[3]\n"
				"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff;\n"
				"    if(!(sx == 0xffff && sy == 0xffff)) { offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; }\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
			if (s_alpha_value)
			{
				opencl_kernel_code += "    f.s3 = (float) alpha;\n";
			}
			else
			{
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
						"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
				}
			}
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, sx == 0xffff && sy == 0xffff);\n";
#else
				opencl_kernel_code += "    Yval.s3 = select(f.s3, 0.0f, sx == 0xffff && sy == 0xffff);\n";
#endif
			}
			opencl_kernel_code +=
				"    outpix.s3 = select(amd_pack(f), invalidPix, sx == 0xffff && sy == 0xffff);\n\n";
		}
		else // RGB
		{
			opencl_kernel_code +=
				"    float mulFactor;\n"
				"    // pixel[0]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s012 *= mulFactor;\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "   Yval.s0 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    Yval.s0 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
						"    Yval.s0 = sqrt(Yval.s0 * 0.3333333333f);\n";
				}
#endif
			}
			opencl_kernel_code +=
				"    // pixel[1]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s3 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s3 *= mulFactor;\n"
				"    outpix.s0 = amd_pack(f);\n"
				"    f.s0 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s01 *= mulFactor;\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s0 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "   Yval.s1 = (f.s3 + f.s0 + f.s1) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    Yval.s1 = mad(f.s3, f.s3, mad(f.s0, f.s0, f.s1 * f.s1));\n"
						"    Yval.s1 = sqrt(Yval.s1 * 0.3333333333f);\n";
				}
#endif
			}
			opencl_kernel_code +=
				"    // pixel[2]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s2 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s23 *= mulFactor;\n"
				"    outpix.s1 = amd_pack(f);\n"
				"    f.s0 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s0 *= mulFactor;\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s0 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "   Yval.s2 = (f.s2 + f.s3 + f.s0) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    Yval.s2 = mad(f.s2, f.s2, mad(f.s3, f.s3, f.s0 * f.s0));\n"
						"    Yval.s2 = sqrt(Yval.s2 * 0.3333333333f);\n";
				}
#endif
			}
			opencl_kernel_code +=
				"    // pixel[3]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride)); px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s1 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s123 *= mulFactor;\n"
				"    outpix.s2 = amd_pack(f);\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s0 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
				if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
					opencl_kernel_code += "   Yval.s3 = (f.s1 + f.s2 + f.s3) * 0.3333333333f;\n";
				else
				{
					opencl_kernel_code +=
						"    Yval.s3 = mad(f.s1, f.s1, mad(f.s2, f.s2, f.s3 * f.s3));\n"
						"    Yval.s3 = sqrt(Yval.s3 * 0.3333333333f);\n";
				}
#endif
			}
		}
	}
	else  // input_format RGBX
	{
		opencl_kernel_code +=
			"    uint2 px0, px1;\n"
			"    __global uchar * pt;\n";
		if (output_format == VX_DF_IMAGE_RGBX)
		{
			opencl_kernel_code +=
				"    uint invalidPix = amd_pack((float4)(0.0f, 0.0f, 0.0f, 128.0f));\n"
				"    bool isSrcInvalid;"
				"    // pixel[0]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
				"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
				"    outpix.s0 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
				opencl_kernel_code += "    Yval.s0 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
			}
			opencl_kernel_code +=
				"    // pixel[1]\n"
				"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
				"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
				"    outpix.s1 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
				opencl_kernel_code += "    Yval.s1 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
			}
			opencl_kernel_code +=
				"    // pixel[2]\n"
				"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
				"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
				"    outpix.s2 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, sx == 0xffff && sy == 0xffff);\n";
#else
				opencl_kernel_code += "    Yval.s2 = select(f.s3, 0.0f, sx == 0xffff && sy == 0xffff);\n";
#endif
			}
			opencl_kernel_code +=
				"    // pixel[3]\n"
				"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
				"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
				"    outpix.s3 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
				opencl_kernel_code += "    Yval.s3 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
			}
		}
		else // RGB
		{
			opencl_kernel_code +=
				"    float mulFactor;\n"
				"    // pixel[0]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s012 *= mulFactor;\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
				opencl_kernel_code += "    Yval.s0 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
			}
			opencl_kernel_code +=
				"    // pixel[1]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s3 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 *= mulFactor;\n"
				"    outpix.s0 = amd_pack(f);\n"
				"    f.s0 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s1 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s01 *= mulFactor;\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
				opencl_kernel_code += "    Yval.s1 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
			}
			opencl_kernel_code +=
				"    // pixel[2]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s2 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s23 *= mulFactor;\n"
				"    outpix.s1 = amd_pack(f);\n"
				"    f.s0 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s0 *= mulFactor;\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s2 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
				opencl_kernel_code += "    Yval.s2 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
			}
			opencl_kernel_code +=
				"    // pixel[3]\n"
				"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
				"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
				"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
				"    f.s1 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s2 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s3 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
				"    f.s123 *= mulFactor;\n"
				"    outpix.s2 = amd_pack(f);\n";
			if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
				opencl_kernel_code += "    Yval.s3 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
				opencl_kernel_code += "    Yval.s3 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
			}
		}
	}
	if (output_format == VX_DF_IMAGE_RGBX)
	{
		opencl_kernel_code +=
			"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x << 5) + ((gid & 1) << 4);\n"
			"    *(__global uint4 *) op_buf = outpix;\n";
	}
	else
	{
		opencl_kernel_code +=
			"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x * 24) + ((gid & 1) * 12);\n"
			"    *(__global uint3 *) (op_buf +  0) = outpix.s012;\n";
	}
	if (bWriteU8Image)
	{
		opencl_kernel_code +=
			"    op_u8_buf += op_u8_offset + ((camera_id * op_u8_image_height_offset + op_y) * op_u8_stride) + (op_x << 3) + ((gid & 1) << 2);\n"
			"    *(__global uint *) op_u8_buf = amd_pack(Yval.s0123);\n";
	}
	opencl_kernel_code +=
		"  }\n"
		"}\n";
#else	// DBG for Vega: This approach reads from sx=sy=0 when sx and sy are invalid

	char item[8192];
	opencl_kernel_code =
		"#pragma OPENCL EXTENSION cl_amd_media_ops : enable\n"
		"#pragma OPENCL EXTENSION cl_amd_media_ops2 : enable\n\n";
	if (!useBilinearInterpolation){
		opencl_kernel_code += Create_compute_bicubic_coeffs();
		if (input_format == VX_DF_IMAGE_RGB){
			opencl_kernel_code += Create_interpolate_cubic_rgb();
		}
		else if (input_format == VX_DF_IMAGE_RGBX){
			opencl_kernel_code += Create_amd_unpack();
			opencl_kernel_code += Create_interpolate_cubic_rgbx();
		}
		else if (input_format == VX_DF_IMAGE_RGB4_AMD){
			opencl_kernel_code += Create_interpolate_cubic_rgb4();
		}
		else if (input_format == VX_DF_IMAGE_RGB6_AMD){
			opencl_kernel_code += Create_amd_unpack15();
			opencl_kernel_code += Create_interpolate_cubic_rgb6();
		}
	}
	sprintf(item,
		"__kernel __attribute__((reqd_work_group_size(%d, 1, 1)))\n" // opencl_local_work[0]
		"void %s(uint grayscale_compute_method,\n" // opencl_kernel_function_name
		"        uint num_cameras,\n"
		"        __global char * valid_pix_buf, uint valid_pix_buf_offset, uint valid_pix_num_items,\n"
		"        __global char * warp_remap_buf, uint warp_remap_buf_offset, uint warp_remap_num_items,\n"
		"        uint ip_width, uint ip_height, __global uchar * ip_buf, uint ip_stride, uint ip_offset,\n"
		"        uint op_width, uint op_height, __global uchar * op_buf, uint op_stride, uint op_offset"
		, (int)opencl_local_work[0], opencl_kernel_function_name);
	opencl_kernel_code += item;
	if (bWriteU8Image) {
		opencl_kernel_code +=
			",\n"
			"        uint op_u8_width, uint op_u8_height, __global uchar * op_u8_buf, uint op_u8_stride, uint op_u8_offset";
	}
	if (s_num_camera_columns) {
		opencl_kernel_code +=
			",\n"
			"        uint num_camera_columns";
	}
	if (s_alpha_value) {
		opencl_kernel_code +=
			",\n"
			"        uint alpha";
	}
	if (s_flags) {
		opencl_kernel_code +=
			",\n"
			"        uint flags";
	}
	if (bWriteExpCompImage){
		opencl_kernel_code +=
			",\n"
			"        uint op_s16_width, uint op_s16_height, __global uchar * op_s16_buf, uint op_s16_stride, uint op_s16_offset";
	}
	sprintf(item,
		")\n"
		"{\n"
		"  int gid = get_global_id(0);\n"
		"  float4 f, mf; uint sx, sy, offset; uint4 outpix; float y; uint2 row_offset;\n"
		"  uint QF = 3;\n"
		"  uint QFB = (1 << QF) - 1; float QFM = 1.0f / (1 << QF);\n"
		"  uint ip_image_height_offset = %d;\n" // ip_image_height_offs
		"  uint op_image_height_offset = %d;\n" // op_image_height_offs
		, ip_image_height_offs, op_image_height_offs);
	opencl_kernel_code += item;
	if (bWriteU8Image)
	{
		sprintf(item,
			"  uint op_u8_image_height_offset = %d;\n" // op_image_height_offs
			, op_image_height_offs);
		opencl_kernel_code += item;
	}
	if (bWriteExpCompImage)
	{
		sprintf(item,
			"  uint op_s16_image_height_offset = %d;\n" // op_image_height_offs
			, op_image_height_offs);
		opencl_kernel_code += item;
	}
	opencl_kernel_code += pixels4 ? get_warp_valid_info_4pixels() : get_warp_valid_info_2pixels();
	if (num_camera_columns == 1)
		opencl_kernel_code += "    ip_buf += ip_offset + (camera_id * ip_image_height_offset * ip_stride);\n";
	else {
		int shiftAmount = 0;
		for (vx_uint32 i = 1; i < 6; i++) {
			if (num_camera_columns == (1u << i)) {
				shiftAmount = i;
				break;
			}
		}
		if (shiftAmount > 0)
			sprintf(item, "    ip_buf += ip_offset + ((camera_id >> %d) * ip_image_height_offset * ip_stride);\n", shiftAmount);
		else
			sprintf(item, "    ip_buf += ip_offset + ((camera_id / %d) * ip_image_height_offset * ip_stride);\n", num_camera_columns);
		opencl_kernel_code += item;
	}
	if (bWriteU8Image || bWriteExpCompImage)
	{
		opencl_kernel_code += pixels4 ? "    float4 Yval;\n" : "    float2 Yval;\n";
#if WRITE_LUMA_AS_A
		opencl_kernel_code += "    float3 RGBToY = (float3)(0.2126f, 0.7152f, 0.0722f);\n";
#endif
	}
	if (input_format == VX_DF_IMAGE_RGB && output_format == VX_DF_IMAGE_RGB){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgb2_bilinear_rgb2(s_alpha_value, grayscale_compute_method, bWriteU8Image) : warp_rgb2_bicubic_rgb2(s_alpha_value, grayscale_compute_method, bWriteU8Image);
		opencl_kernel_code += write_4pixels_to_RGB();
	}
	else if (input_format == VX_DF_IMAGE_RGB && output_format == VX_DF_IMAGE_RGBX){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgb2_bilinear_rgbx(s_alpha_value, grayscale_compute_method, bWriteU8Image) : warp_rgb2_bicubic_rgbx(s_alpha_value, grayscale_compute_method, bWriteU8Image);
		opencl_kernel_code += write_4pixels_to_RGBX();
	}
	else if (input_format == VX_DF_IMAGE_RGBX && output_format == VX_DF_IMAGE_RGB){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgbx_bilinear_rgb2(bWriteU8Image) : warp_rgbx_bicubic_rgb2(bWriteU8Image);
		opencl_kernel_code += write_4pixels_to_RGB();
	}
	else if (input_format == VX_DF_IMAGE_RGBX && output_format == VX_DF_IMAGE_RGBX){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgbx_bilinear_rgbx(bWriteU8Image) : warp_rgbx_bicubic_rgbx(bWriteU8Image);
		opencl_kernel_code += write_4pixels_to_RGBX();
	}
	else if (input_format == VX_DF_IMAGE_RGB4_AMD && output_format == VX_DF_IMAGE_RGB4_AMD){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgb4_bilinear_rgb4(s_alpha_value, grayscale_compute_method, bWriteU8Image) : warp_rgb4_bicubic_rgb4(s_alpha_value, grayscale_compute_method, bWriteU8Image);
		opencl_kernel_code += pixels4 ? write_4pixels_to_RGB4() : write_2pixels_to_RGB4();
	}
	else if (input_format == VX_DF_IMAGE_RGB4_AMD && output_format == VX_DF_IMAGE_RGB6_AMD){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgb4_bilinear_rgb6(s_alpha_value, grayscale_compute_method, bWriteU8Image) : warp_rgb4_bicubic_rgb6(s_alpha_value, grayscale_compute_method, bWriteU8Image);
		opencl_kernel_code += pixels4 ? write_4pixels_to_RGB6() : write_2pixels_to_RGB6();
	}
	else if (input_format == VX_DF_IMAGE_RGB6_AMD && output_format == VX_DF_IMAGE_RGB4_AMD){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgb6_bilinear_rgb4(bWriteU8Image) : warp_rgb6_bicubic_rgb4(bWriteU8Image);
		opencl_kernel_code += pixels4 ? write_4pixels_to_RGB4() : write_2pixels_to_RGB4();
	}
	else if (input_format == VX_DF_IMAGE_RGB6_AMD && output_format == VX_DF_IMAGE_RGB6_AMD){
		opencl_kernel_code += useBilinearInterpolation ? warp_rgb6_bilinear_rgb6(bWriteU8Image) : warp_rgb6_bicubic_rgb6(bWriteU8Image);
		opencl_kernel_code += pixels4 ? write_4pixels_to_RGB6() : write_2pixels_to_RGB6();
	}
	if (bWriteExpCompImage){
		opencl_kernel_code += write_exp_comp_img(pixels4);
	}
	if (bWriteU8Image)
	{
		if (output_U_format == VX_DF_IMAGE_U8)
		{
			opencl_kernel_code += (input_format == VX_DF_IMAGE_RGB4_AMD || input_format == VX_DF_IMAGE_RGB6_AMD) ? write_luma_U8_16bit_input(pixels4) : write_luma_U8_8bit_input();
		}
		else
		{
			opencl_kernel_code += write_luma_S16_16bit_input(pixels4);
		}
	}
	opencl_kernel_code +=
		"  }\n"
		"}\n";
#endif
	if (s_num_camera_columns)	ERROR_CHECK_STATUS(vxReleaseScalar(&s_num_camera_columns));
	if (s_alpha_value)			ERROR_CHECK_STATUS(vxReleaseScalar(&s_alpha_value));
	if (s_flags)				ERROR_CHECK_STATUS(vxReleaseScalar(&s_flags));
	
	return VX_SUCCESS;
}

//! \brief The kernel execution.
static vx_status VX_CALLBACK warp_kernel(vx_node node, const vx_reference * parameters, vx_uint32 num)
{
	return VX_ERROR_NOT_SUPPORTED;
}

//! \brief The kernel publisher.
vx_status warp_publish(vx_context context)
{
	// add kernel to the context with callbacks
	vx_kernel kernel = vxAddKernel(context, "com.amd.loomsl.warp",
		AMDOVX_KERNEL_STITCHING_WARP,
		warp_kernel,
		10,
		warp_input_validator,
		warp_output_validator,
		nullptr,
		nullptr);
	ERROR_CHECK_OBJECT(kernel);
	amd_kernel_query_target_support_f query_target_support_f = warp_query_target_support;
	amd_kernel_opencl_codegen_callback_f opencl_codegen_callback_f = warp_opencl_codegen;
	amd_kernel_opencl_global_work_update_callback_f opencl_global_work_update_callback_f = warp_opencl_global_work_update;
	ERROR_CHECK_STATUS(vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_AMD_QUERY_TARGET_SUPPORT, &query_target_support_f, sizeof(query_target_support_f)));
	ERROR_CHECK_STATUS(vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_AMD_OPENCL_CODEGEN_CALLBACK, &opencl_codegen_callback_f, sizeof(opencl_codegen_callback_f)));
	ERROR_CHECK_STATUS(vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_AMD_OPENCL_GLOBAL_WORK_UPDATE_CALLBACK, &opencl_global_work_update_callback_f, sizeof(opencl_global_work_update_callback_f)));

	// set kernel parameters
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 0, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 1, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 2, VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 3, VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 4, VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 5, VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 6, VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 7, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 8, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 9, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL));
	ERROR_CHECK_STATUS(vxAddParameterToKernel(kernel, 10, VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL));

	// finalize and release kernel object
	ERROR_CHECK_STATUS(vxFinalizeKernel(kernel));
	ERROR_CHECK_STATUS(vxReleaseKernel(&kernel));

	return VX_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// Calculate buffer sizes and generate data in buffers for warp
//   CalculateLargestWarpBufferSizes  - useful when reinitialize is enabled
//   CalculateSmallestWarpBufferSizes - useful when reinitialize is disabled
//   GenerateWarpBuffers              - generate tables

vx_status CalculateLargestWarpBufferSizes(
	vx_uint32 numCamera,                  // [in] number of cameras
	vx_uint32 eqrWidth,                   // [in] output equirectangular image width
	vx_uint32 eqrHeight,                  // [in] output equirectangular image height
	vx_size * warpMapEntryCount           // [out] number of entries needed by warp map table
	)
{
	// one entry for every eight consecutive pixels one in every camera
	*warpMapEntryCount = ((eqrWidth + 7) >> 3) * eqrHeight * numCamera;
	return VX_SUCCESS;
}

vx_status CalculateSmallestWarpBufferSizes(
	vx_uint32 numCamera,                         // [in] number of cameras
	vx_uint32 eqrWidth,                          // [in] output equirectangular image width
	vx_uint32 eqrHeight,                         // [in] output equirectangular image height
	const vx_uint32 * validPixelCamMap,          // [in] valid pixel camera index map: size: [eqrWidth * eqrHeight]
	const vx_uint32 * paddedPixelCamMap,         // [in] padded pixel camera index map: size: [eqrWidth * eqrHeight]
	vx_size * warpMapEntryCount                  // [out] number of entries needed by warp map table
	)
{
	vx_uint32  entryCount = 0;
	for (vx_uint32 y_eqr = 0, pixelPosition = 0; y_eqr < eqrHeight; y_eqr++)
	{
		for (vx_uint32 x_eqr = 0; x_eqr < eqrWidth; x_eqr += 8, pixelPosition += 8)
		{
			// get camera use mask for consecutive 8 pixels from current pixel position
			vx_uint32 validMaskFor8Pixels =
				validPixelCamMap[pixelPosition + 0] | validPixelCamMap[pixelPosition + 1] |
				validPixelCamMap[pixelPosition + 2] | validPixelCamMap[pixelPosition + 3] |
				validPixelCamMap[pixelPosition + 4] | validPixelCamMap[pixelPosition + 5] |
				validPixelCamMap[pixelPosition + 6] | validPixelCamMap[pixelPosition + 7];
			if (paddedPixelCamMap) {
				validMaskFor8Pixels |=
					paddedPixelCamMap[pixelPosition + 0] | paddedPixelCamMap[pixelPosition + 1] |
					paddedPixelCamMap[pixelPosition + 2] | paddedPixelCamMap[pixelPosition + 3] |
					paddedPixelCamMap[pixelPosition + 4] | paddedPixelCamMap[pixelPosition + 5] |
					paddedPixelCamMap[pixelPosition + 6] | paddedPixelCamMap[pixelPosition + 7];
			}
			// each bit in validMaskFor8Pixels indicates that a warpMapEntry is needed for that camera
			entryCount += GetOneBitCount(validMaskFor8Pixels);
		}
	}

	entryCount = (entryCount + 63) & ~63;		// Make entry count next highest multiple of 64
	*warpMapEntryCount = entryCount;

	return VX_SUCCESS;
}

vx_status GenerateWarpBuffers(
	vx_uint32 numCamera,                         // [in] number of cameras
	vx_uint32 eqrWidth,                          // [in] output equirectangular image width
	vx_uint32 eqrHeight,                         // [in] output equirectangular image height
	const vx_uint32 * validPixelCamMap,          // [in] valid pixel camera index map: size: [eqrWidth * eqrHeight]
	const vx_uint32 * paddedPixelCamMap,         // [in] padded pixel camera index map: size: [eqrWidth * eqrHeight]
	const StitchCoord2dFloat * camSrcMap,        // [in] camera coordinate mapping: size: [numCamera * eqrWidth * eqrHeight] (optional)
	vx_uint32 numCameraColumns,                  // [in] number of camera columns
	vx_uint32 camWidth,                          // [in] input camera image width
	vx_size   mapTableSize,                      // [in] size of warp/valid map table, in terms of number of entries
	StitchValidPixelEntry * validMap,            // [in] valid map table
	StitchWarpRemapEntry * warpMap,              // [in] warp map table
	vx_size * mapEntryCount                      // [out] number of entries added to warp/valid map table
	)
{
	vx_uint32  entryCount = 0;
	for (vx_uint32 camId = 0; camId < numCamera; camId++)
	{
		float xSrcOffset = (float)((camId % numCameraColumns) * camWidth) * 8.0f;
		vx_uint32 camMapBit = 1 << camId;
		const StitchCoord2dFloat * camSrcMapCurrent = camSrcMap + camId * eqrWidth * eqrHeight;
		for (vx_uint32 y_eqr = 0, pixelPosition = 0; y_eqr < eqrHeight; y_eqr++)
		{
			for (vx_uint32 x_eqr = 0; x_eqr < eqrWidth; x_eqr += 8, pixelPosition += 8)
			{
				// get camera use mask for consecutive 8 pixels from current pixel position
				vx_uint32 validMask[8];
				vx_uint32 validMaskFor8Pixels = 0;
				if (paddedPixelCamMap) {
					for (vx_uint32 i = 0; i < 8; i++) {
						validMask[i] = validPixelCamMap[pixelPosition + i] | paddedPixelCamMap[pixelPosition + i];
						validMaskFor8Pixels |= validMask[i];
					}
				}
				else {
					for (vx_uint32 i = 0; i < 8; i++) {
						validMask[i] = validPixelCamMap[pixelPosition + i];
						validMaskFor8Pixels |= validMask[i];
					}
				}
				if (validMaskFor8Pixels & camMapBit)
				{
					if (entryCount < mapTableSize)
					{
						// get mask to check if all pixels are valid and set validMap entry
						vx_uint32 allValidMaskFor8Pixels = validMask[0];
						for (vx_uint32 i = 1; i < 8; i++) {
							allValidMaskFor8Pixels &= validMask[i];
						}
						StitchValidPixelEntry validEntry = { 0 };
						validEntry.camId = camId;
						validEntry.allValid = (allValidMaskFor8Pixels & camMapBit) ? 1 : 0;
						validEntry.dstX = x_eqr >> 3;
						validEntry.dstY = y_eqr;
						validMap[entryCount] = validEntry;
						// set warpMap entry: NOTE: assumes that current structure of StitchWarpRemapEntry to be consetive (x,y) value pairs
						const StitchCoord2dFloat * srcEntry = &camSrcMapCurrent[pixelPosition];
						vx_uint16 * warpEntry = (vx_uint16 *)&warpMap[entryCount];
						for (vx_uint32 i = 0; i < 8; i++, warpEntry += 2, srcEntry++) {
							warpEntry[0] = !(validMask[i] & camMapBit) ? (vx_uint16)0xffff : (vx_uint16)(srcEntry->x * 8.0f + 0.5f + xSrcOffset);
							warpEntry[1] = !(validMask[i] & camMapBit) ? (vx_uint16)0xffff : (vx_uint16)(srcEntry->y * 8.0f + 0.5f);
						}
					}
					entryCount++;
				}
			}
		}
	}

	while (entryCount < mapTableSize) {
		vx_uint32 * validEntry = (vx_uint32 *) &validMap[entryCount];
		*validEntry = 0xFFFFFFFF;

		vx_uint16 * warpEntry = (vx_uint16 *)&warpMap[entryCount];
		for (int i = 0; i < 16; i++)	
			warpEntry[i] = (vx_uint16)0xffff;

		entryCount++;
	}
	*mapEntryCount = entryCount;

	// check for buffer overflow error condition
	if (entryCount > mapTableSize) {
		return VX_ERROR_NOT_SUFFICIENT;
	}

	return VX_SUCCESS;


}

// Get Warp and Valid information ------------------------------------------------------------------------------------------------------------
std::string get_warp_valid_info_4pixels(){
	std::string output =
		"  warp_remap_buf += warp_remap_buf_offset + (gid << 4);\n"
		"  valid_pix_buf += valid_pix_buf_offset + ((gid >> 1) << 2);\n"
		"  if (((gid >> 1) < valid_pix_num_items)) {\n"
		"    uint pixelEntry = *(__global uint*) valid_pix_buf;\n"
		"    if(pixelEntry == 0xffffffff) return;\n"
		"    uint4 map = *(__global uint4 *) warp_remap_buf;\n"
		"    uint camera_id = pixelEntry & 0x1f; uint op_x = (pixelEntry >> 8) & 0x7ff; uint op_y = (pixelEntry >> 19) & 0x1fff;\n";
	return output;
}
std::string get_warp_valid_info_2pixels(){
	std::string output =
		"  warp_remap_buf += warp_remap_buf_offset + ((gid >> 1) << 4);\n"
		"  valid_pix_buf += valid_pix_buf_offset + ((gid >> 2) << 2);\n"
		"  if (((gid >> 2) < valid_pix_num_items)) {\n"
		"    uint pixelEntry = *(__global uint*) valid_pix_buf;\n"
		"    if(pixelEntry == 0xffffffff) return;\n"
		"    uint4 map = *(__global uint4 *) warp_remap_buf;\n"
		"    uint camera_id = pixelEntry & 0x1f; uint op_x = (pixelEntry >> 8) & 0x7ff; uint op_y = (pixelEntry >> 19) & 0x1fff;\n";
	return output;
}


// Do reading and wraping bilinear ------------------------------------------------------------------------------------------------------------
std::string warp_rgb2_bilinear_rgbx(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint3 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
		"    uint invalidPix = amd_pack((float4)(0.0f, 0.0f, 0.0f, 128.0f));\n"
		"    bool isSrcInvalid;"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s0 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s0 = select(amd_pack(f), invalidPix, isSrcInvalid);\n"
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s1 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s1 = select(amd_pack(f), invalidPix, isSrcInvalid);\n\n"
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s2 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s2 = select(amd_pack(f), invalidPix, isSrcInvalid);\n\n"
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s3 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s3 = select(amd_pack(f), invalidPix, isSrcInvalid);\n\n";

	return output;
}
std::string warp_rgb2_bilinear_rgb2(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint3 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
		"    float mulFactor;\n"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s012 *= mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s0 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s0 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    Yval.s0 = sqrt(Yval.s0 * 0.3333333333f);\n";
		}
#endif
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s3 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s3 *= mulFactor;\n"
		"    outpix.s0 = amd_pack(f);\n"
		"    f.s0 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s01 *= mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s1 = (f.s3 + f.s0 + f.s1) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s1 = mad(f.s3, f.s3, mad(f.s0, f.s0, f.s1 * f.s1));\n"
				"    Yval.s1 = sqrt(Yval.s1 * 0.3333333333f);\n";
		}
#endif
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s2 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s3 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s23 *= mulFactor;\n"
		"    outpix.s1 = amd_pack(f);\n"
		"    f.s0 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s0 *= mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s2 = (f.s2 + f.s3 + f.s0) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s2 = mad(f.s2, f.s2, mad(f.s3, f.s3, f.s0 * f.s0));\n"
				"    Yval.s2 = sqrt(Yval.s2 * 0.3333333333f);\n";
		}
#endif
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; mulFactor = 0.0f; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 3; pt = ip_buf + (offset & ~3); px0 = vload3(0, (__global uint *)pt); px1 = vload3(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset);\n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s1 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack3(px0.s0) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack3(px1.s0) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s3 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s123 *= mulFactor;\n"
		"    outpix.s2 = amd_pack(f);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s3 = (f.s1 + f.s2 + f.s3) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s3 = mad(f.s1, f.s1, mad(f.s2, f.s2, f.s3 * f.s3));\n"
				"    Yval.s3 = sqrt(Yval.s3 * 0.3333333333f);\n";
		}
#endif
	}
	return output;
}
std::string warp_rgbx_bilinear_rgbx(bool bWriteU8Image){
	std::string output =
		"    uint2 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
			"    uint invalidPix = amd_pack((float4)(0.0f, 0.0f, 0.0f, 128.0f));\n"
			"    bool isSrcInvalid;"
			"    // pixel[0]\n"
			"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
			"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride));\n"
			"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
			"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
			"    outpix.s0 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s0 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		output +=
			"    // pixel[1]\n"
			"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
			"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride));\n"
			"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
			"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
			"    outpix.s1 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s1 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		output +=
			"    // pixel[2]\n"
			"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
			"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride));\n"
			"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
			"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
			"    outpix.s2 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s2 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		output +=
			"    // pixel[3]\n"
			"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
			"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride));\n"
			"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
			"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
			"    f.s3 = (amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1;\n"
			"    outpix.s3 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s3 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		return output;
}
std::string warp_rgbx_bilinear_rgb2(bool bWriteU8Image){
	std::string output =
		"    uint2 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
		"    float mulFactor;\n"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s012 *= mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		output += "    Yval.s0 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s3 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s3 *= mulFactor;\n"
		"    outpix.s0 = amd_pack(f);\n"
		"    f.s0 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s1 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s01 *= mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		output += "    Yval.s1 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s2 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s3 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s23 *= mulFactor;\n"
		"    outpix.s1 = amd_pack(f);\n"
		"    f.s0 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s0 *= mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
		output += "    Yval.s2 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 4; pt = ip_buf + offset; px0 = vload2(0, (__global uint *)pt); px1 = vload2(0, (__global uint *)(pt + ip_stride)); mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s1 = (amd_unpack0(px0.s0) * mf.s2 + amd_unpack0(px0.s1) * mf.s0) * mf.s3 + (amd_unpack0(px1.s0) * mf.s2 + amd_unpack0(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s2 = (amd_unpack1(px0.s0) * mf.s2 + amd_unpack1(px0.s1) * mf.s0) * mf.s3 + (amd_unpack1(px1.s0) * mf.s2 + amd_unpack1(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s3 = (amd_unpack2(px0.s0) * mf.s2 + amd_unpack2(px0.s1) * mf.s0) * mf.s3 + (amd_unpack2(px1.s0) * mf.s2 + amd_unpack2(px1.s1) * mf.s0) * mf.s1;\n"
		"    f.s123 *= mulFactor;\n"
		"    outpix.s2 = amd_pack(f);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
		output += "    Yval.s3 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
		return output;
}
std::string warp_rgb4_bilinear_rgb6(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint4 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    uint invalidPix0 = (((uint)0) << 16) + 0;\n"
		"    uint invalidPix1 = (((uint)16384) << 16) + 0;\n"
		"    bool isSrcInvalid;"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"	 px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s0 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s0 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s1 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n\n"
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"	 px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s1 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s2 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s3 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n\n"
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"	 px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s2 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix1.s0 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix1.s1 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n\n"
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"	 px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s3 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix1.s2 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix1.s3 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n\n";
		return output;
}
std::string warp_rgb4_bilinear_rgb4(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint4 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    bool isSrcInvalid;\n"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; isSrcInvalid = true; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"    px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"    mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid) {f.s012 = (float3)32768;}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid){\n"
			"       Yval.s0 = -10.0f;\n"
			"    }else{\n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s0 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s0 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    Yval.s0 = sqrt(Yval.s0 * 0.3333333333f);\n";
		}
#endif
		output +=
			"    }\n";
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; isSrcInvalid = true; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"    px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s3 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid) {f.s3 = 32768;}\n"
		"    outpix.s0 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix.s1 = (((uint)f.s3) << 16) + (uint)f.s2;\n"
		"    f.s0 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid) {f.s01 = (float2)32768;}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid){\n"
			"       Yval.s1 = -10.0f;\n"
			"    }else{\n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s1 = (f.s3 + f.s0 + f.s1) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s1 = mad(f.s3, f.s3, mad(f.s0, f.s0, f.s1 * f.s1));\n"
				"    Yval.s1 = sqrt(Yval.s1 * 0.3333333333f);\n";
		}
#endif
		output +=
			"    }\n";
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; isSrcInvalid = true; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"    px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s2 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid) {f.s23 = (float2)32768;}\n"
		"    outpix.s2 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix.s3 = (((uint)f.s3) << 16) + (uint)f.s2;\n"
		"    f.s0 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid) {f.s0 = 32768;}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid){\n"
			"       Yval.s2 = -10.0f;\n"
			"    }else{\n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s2 = (f.s2 + f.s3 + f.s0) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s2 = mad(f.s2, f.s2, mad(f.s3, f.s3, f.s0 * f.s0));\n"
				"    Yval.s2 = sqrt(Yval.s2 * 0.3333333333f);\n";
		}
#endif
		output +=
			"    }\n";
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 0; sy = 0; isSrcInvalid = true; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 6; pt = ip_buf + (offset & ~3); px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"    px0.s0 = amd_bytealign(px0.s1, px0.s0, offset); px0.s1 = amd_bytealign(px0.s2, px0.s1, offset); px0.s2 = amd_bytealign(px0.s3, px0.s2, offset);\n"
		"    px1.s0 = amd_bytealign(px1.s1, px1.s0, offset); px1.s1 = amd_bytealign(px1.s2, px1.s1, offset); px1.s2 = amd_bytealign(px1.s3, px1.s2, offset); \n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1;\n"
		"    f.s1 = ((px0.s0 & 0x7fff) * mf.s2 + ((px0.s1 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + ((px1.s1 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = ((px0.s1 & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid) {f.s123 = (float3)32768;}\n"
		"    outpix1.s0 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix1.s1 = (((uint)f.s3) << 16) + (uint)f.s2;\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid){\n"
			"       Yval.s3 = -10.0f;\n"
			"    }else{\n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s3 = (f.s1 + f.s2 + f.s3) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s3 = mad(f.s1, f.s1, mad(f.s2, f.s2, f.s3 * f.s3));\n"
				"    Yval.s3 = sqrt(Yval.s3 * 0.3333333333f);\n";
		}
#endif
		output +=
			"    }\n";
	}
		return output;
}
std::string warp_rgb6_bilinear_rgb6(bool bWriteU8Image){
	std::string output =
		"    uint4 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    uint invalidPix0 = (((uint)0) << 16) + 0;\n"
		"    uint invalidPix1 = (((uint)16384) << 16) + 0;\n"
		"    bool isSrcInvalid;"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = (((px0.s1 >> 16) & 0x7fff) * mf.s2 + ((px0.s3 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s1 >> 16) & 0x7fff) * mf.s2 + ((px1.s3 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    outpix.s0 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s1 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s0 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = (((px0.s1 >> 16) & 0x7fff) * mf.s2 + ((px0.s3 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s1 >> 16) & 0x7fff) * mf.s2 + ((px1.s3 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    outpix.s2 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s3 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s1 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = (((px0.s1 >> 16) & 0x7fff) * mf.s2 + ((px0.s3 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s1 >> 16) & 0x7fff) * mf.s2 + ((px1.s3 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    outpix1.s0 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix1.s1 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s2 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = (((px0.s1 >> 16) & 0x7fff) * mf.s2 + ((px0.s3 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s1 >> 16) & 0x7fff) * mf.s2 + ((px1.s3 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    outpix1.s2 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix1.s3 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s3 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
		return output;
}
std::string warp_rgb6_bilinear_rgb4(bool bWriteU8Image){
	std::string output =
		"    uint4 px0, px1;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    bool isSrcInvalid;\n"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s0 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid){f.s012=(float3)32768;}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s0 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		output += "    Yval.s0 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
		output +=
			"    }\n";
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s3 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid){f.s3=32768;}\n"
		"    outpix.s0 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix.s1 = (((uint)f.s3) << 16) + (uint)f.s2;\n"
		"    f.s0 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s1 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid){f.s01=(float2)32768;}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s1 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		output += "    Yval.s1 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
		output +=
			"    }\n";
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s2 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid){f.s23=(float2)32768;}\n;\n"
		"    outpix.s2 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix.s3 = (((uint)f.s3) << 16) + (uint)f.s2;\n"
		"    f.s0 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid){f.s0=(float)32768;}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s2 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
		output += "    Yval.s2 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
		output +=
			"    }\n";
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 0; sy = 0; }\n"
		"    offset = (sy >> QF) * ip_stride + (sx >> QF) * 8; pt = ip_buf + offset; px0 = vload4(0, (__global uint *)pt); px1 = vload4(0, (__global uint *)(pt + ip_stride));\n"
		"	 mf.s0 = (sx & QFB) * QFM; mf.s1 = (sy & QFB) * QFM; mf.s2 = 1.0f - mf.s0; mf.s3 = 1.0f - mf.s1; \n"
		"    f.s1 = ((px0.s0 & 0x7fff) * mf.s2 + (px0.s2 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s0 & 0x7fff) * mf.s2 + (px1.s2 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s2 = (((px0.s0 >> 16) & 0x7fff) * mf.s2 + ((px0.s2 >> 16) & 0x7fff) * mf.s0) * mf.s3 + (((px1.s0 >> 16) & 0x7fff) * mf.s2 + ((px1.s2 >> 16) & 0x7fff) * mf.s0) * mf.s1;\n"
		"    f.s3 = ((px0.s1 & 0x7fff) * mf.s2 + (px0.s3 & 0x7fff) * mf.s0) * mf.s3 + ((px1.s1 & 0x7fff) * mf.s2 + (px1.s3 & 0x7fff) * mf.s0) * mf.s1;\n"
		"    if(isSrcInvalid){f.s123=(float3)32768;}\n"
		"    outpix1.s0 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix1.s1 = (((uint)f.s3) << 16) + (uint)f.s2;\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s3 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
		output += "    Yval.s3 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
		output +=
			"    }\n";
	}
		return output;
}

// Do reading and wraping bicubic ------------------------------------------------------------------------------------------------------------
std::string warp_rgb2_bicubic_rgbx(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint4 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
			"    uint invalidPix = amd_pack((float4)(0.0f, 0.0f, 0.0f, 128.0f));\n"
			"    bool isSrcInvalid;\n"
			"    // pixel[0]\n"
			"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
			"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
			"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
			"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
			"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
			"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
			"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n";
		if (s_alpha_value)
		{
			output += "    f.s3 = (float) alpha;\n";
		}
		else
		{
			if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
				output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
			else
			{
				output +=
					"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
					"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
			}
		}
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s0 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		output +=
			"    outpix.s0 = select(amd_pack(f), invalidPix, isSrcInvalid);\n"
			"    // pixel[1]\n"
			"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
			"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
			"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
			"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
			"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
			"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
			"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n";
		if (s_alpha_value)
		{
			output += "    f.s3 = (float) alpha;\n";
		}
		else
		{
			if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
				output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
			else
			{
				output +=
					"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
					"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
			}
		}
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s1 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		output +=
			"    outpix.s1 = select(amd_pack(f), invalidPix, isSrcInvalid);\n\n"
			"    // pixel[2]\n"
			"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
			"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
			"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
			"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
			"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
			"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
			"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n";
		if (s_alpha_value)
		{
			output += "    f.s3 = (float) alpha;\n";
		}
		else
		{
			if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
				output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
			else
			{
				output +=
					"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
					"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
			}
		}
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s2 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		output +=
			"    outpix.s2 = select(amd_pack(f), invalidPix, isSrcInvalid);\n\n"
			"    // pixel[3]\n"
			"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
			"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
			"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
			"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
			"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
			"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
			"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
			"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
			"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
			"    f.s012 += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n";
		if (s_alpha_value)
		{
			output += "    f.s3 = (float) alpha;\n";
		}
		else
		{
			if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
				output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
			else
			{
				output +=
					"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
					"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
			}
		}
		if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
			output += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
			output += "    Yval.s3 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
		}
		output +=
			"    outpix.s3 = select(amd_pack(f), invalidPix, isSrcInvalid);\n\n";
	return output;
}
std::string warp_rgb2_bicubic_rgb2(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint4 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
		"    float mulFactor; float3 tf;\n"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 1 << QF; sy = 1 << QF; mulFactor = 0.0f; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 *ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s012 = tf * mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s0 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s0 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    Yval.s0 = sqrt(Yval.s0 * 0.3333333333f);\n";
		}
#endif
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 1 << QF; sy = 1 << QF; mulFactor = 0.0f; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 *ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s3 = tf.s0 * mulFactor;\n"
		"    outpix.s0 = amd_pack(f);\n"
		"    f.s01 = tf.s12 * mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s1 = (f.s3 + f.s0 + f.s1) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s1 = mad(f.s3, f.s3, mad(f.s0, f.s0, f.s1 * f.s1));\n"
				"    Yval.s1 = sqrt(Yval.s1 * 0.3333333333f);\n";
		}
#endif
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 1 << QF; sy = 1 << QF; mulFactor = 0.0f; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 *ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s23 = tf.s01 * mulFactor;\n"
		"    outpix.s1 = amd_pack(f);\n"
		"    f.s0 = tf.s2 * mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s2 = (f.s2 + f.s3 + f.s0) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s2 = mad(f.s2, f.s2, mad(f.s3, f.s3, f.s0 * f.s0));\n"
				"    Yval.s2 = sqrt(Yval.s2 * 0.3333333333f);\n";
		}
#endif
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 1 << QF; sy = 1 << QF; mulFactor = 0.0f; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 3, (int) 0, (int)ip_width*3); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 *ip_stride));                 px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf = interpolate_cubic_rgb(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + 1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride)); px.s0 = amd_bytealign(px.s1, px.s0, offset); px.s1 = amd_bytealign(px.s2, px.s1, offset); px.s2 = amd_bytealign(px.s3, px.s2, offset);\n"
		"    tf += (interpolate_cubic_rgb(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s123 = tf * mulFactor;\n"
		"    outpix.s2 = amd_pack(f);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s3 = (f.s1 + f.s2 + f.s3) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s3 = mad(f.s1, f.s1, mad(f.s2, f.s2, f.s3 * f.s3));\n"
				"    Yval.s3 = sqrt(Yval.s3 * 0.3333333333f);\n";
		}
#endif
	}
	return output;
}
std::string warp_rgbx_bicubic_rgbx(bool bWriteU8Image){
	std::string output =
		"    uint4 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
		"    uint invalidPix = amd_pack((float4)(0.0f, 0.0f, 0.0f, 128.0f));\n"
		"    bool isSrcInvalid;"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); f  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             f += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 f += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride));   f += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    outpix.s0 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s0 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); f  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             f += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 f += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride));   f += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    outpix.s1 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s1 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); f  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             f += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 f += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride));   f += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    outpix.s2 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s2 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); f  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             f += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 f += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride));   f += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    outpix.s3 = select(amd_pack(f), invalidPix, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), 0.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s3 = select(f.s3, 0.0f, isSrcInvalid);\n";
#endif
	}
	return output;
}
std::string warp_rgbx_bicubic_rgb2(bool bWriteU8Image){
	std::string output =
		"    uint4 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix;\n"
		"    float mulFactor; float4 tf;\n"
		"    // pixel[0]\n"
		"    sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); tf  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             tf += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 tf += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1 * ip_stride)); tf += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s012 = tf.s012 * mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		output += "    Yval.s0 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
	output +=
		"    // pixel[1]\n"
		"    sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); tf  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             tf += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 tf += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride));   tf += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s3 = tf.s0 * mulFactor;\n"
		"    outpix.s0 = amd_pack(f);\n"
		"    f.s01 = tf.s12 * mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		output += "    Yval.s1 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
	output +=
		"    // pixel[2]\n"
		"    sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); tf  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             tf += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 tf += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride));   tf += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s23 = tf.s01 * mulFactor;\n"
		"    outpix.s1 = amd_pack(f);\n"
		"    f.s0 = tf.s2 * mulFactor;\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s2 = mad(f.s2, RGBToY.s0, mad(f.s3, RGBToY.s1, f.s0 * RGBToY.s2));\n";
#else
		output += "    Yval.s2 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
	output +=
		"    // pixel[3]\n"
		"    sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff; mulFactor = 1.0f;\n"
		"    if(sx == 0xffff && sy == 0xffff) { mulFactor = 0.0f; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 4, (int) 0, (int)ip_width*4); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload4(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); tf  =  interpolate_cubic_rgbx(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload4(0, (__global uint *)(pt));                             tf += (interpolate_cubic_rgbx(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + ip_stride));                 tf += (interpolate_cubic_rgbx(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload4(0, (__global uint *)(pt + row_offset.s1*ip_stride));   tf += (interpolate_cubic_rgbx(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f.s123 = tf.s012 * mulFactor;\n"
		"    outpix.s2 = amd_pack(f);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s3 = mad(f.s1, RGBToY.s0, mad(f.s2, RGBToY.s1, f.s3 * RGBToY.s2));\n";
#else
		output += "    Yval.s3 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
	}
	return output;
}
std::string warp_rgb4_bicubic_rgb6(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint8 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    uint invalidPix0 = (((uint)0) << 16) + 0;\n"
		"    uint invalidPix1 = (((uint)16384) << 16) + 0;\n"
		"    bool isSrcInvalid;\n"
		"    // pixel[0]\n"
		"    if( gid & 1){\n"
		"		sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 6, (int) 0, (int)ip_width*6); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt- row_offset.s0 * ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 = interpolate_cubic_rgb4(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 += (interpolate_cubic_rgb4(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 += (interpolate_cubic_rgb4(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 += (interpolate_cubic_rgb4(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f = clamp (f, 0.0f, 32767.0f);\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s0 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s0 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s1 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n"
		"    // pixel[1]\n"
		"    if( gid & 1){\n"
		"		sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 6, (int) 0, (int)ip_width*6); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt- row_offset.s0 * ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 = interpolate_cubic_rgb4(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 += (interpolate_cubic_rgb4(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 += (interpolate_cubic_rgb4(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    f.s012 += (interpolate_cubic_rgb4(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f = clamp (f, 0.0f, 32767.0f);\n";
	if (s_alpha_value)
	{
		output += "    f.s3 = (float) alpha;\n";
	}
	else
	{
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "    f.s3 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    f.s3 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    f.s3 = sqrt(f.s3 * 0.3333333333f);\n";
		}
	}
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s1 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    outpix.s2 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s3 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n";
	return output;
}
std::string warp_rgb4_bicubic_rgb4(vx_scalar s_alpha_value, vx_enum grayscale_compute_method, bool bWriteU8Image){
	std::string output =
		"    uint8 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    bool isSrcInvalid; float3 tf;\n"
		"    // pixel[0]\n"
		"    if( gid & 1){\n"
		"		sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 1 << QF; sy = 1 << QF; isSrcInvalid = true; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 6, (int) 0, (int)ip_width*6); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt-row_offset.s0*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf = interpolate_cubic_rgb4(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf += (interpolate_cubic_rgb4(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + 1*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf += (interpolate_cubic_rgb4(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf += (interpolate_cubic_rgb4(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    if (isSrcInvalid){f.s012 = (float3)32768;}\n"
		"    else{f.s012 = clamp (tf.s012, 0.0f, 32767.0f);}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s0 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s0 = (f.s0 + f.s1 + f.s2) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s0 = mad(f.s0, f.s0, mad(f.s1, f.s1, f.s2 * f.s2));\n"
				"    Yval.s0 = sqrt(Yval.s0 * 0.3333333333f);\n";
		}
#endif
		output +=
			"    } \n";
	}
	output +=
		"    // pixel[1]\n"
		"    if( gid & 1){\n"
		"		sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { sx = 1 << QF; sy = 1 << QF; isSrcInvalid = true; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 6, (int) 0, (int)ip_width*6); pt = ip_buf + (offset & ~3);\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt-row_offset.s0*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf = interpolate_cubic_rgb4(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf += (interpolate_cubic_rgb4(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + 1*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf += (interpolate_cubic_rgb4(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1*ip_stride));\n"
		"    px = amd_bytealign(px.s12345677, px, offset);\n"
		"    tf += (interpolate_cubic_rgb4(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    if (isSrcInvalid){f.s3 = (float)32768;}\n"
		"    else{f.s3 = clamp (tf.s0, 0.0f, 32767.0f);}\n"
		"    outpix.s0 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix.s1 =(((uint)f.s3) << 16) + (uint)f.s2;\n"
		"    if (isSrcInvalid){f.s01 = (float2)32768;}\n"
		"    else{f.s01 = clamp (tf.s12, 0.0f, 32767.0f);}\n"
		"    outpix.s2 = (((uint)f.s1) << 16) + (uint)f.s0;\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s1 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		if (grayscale_compute_method == STITCH_GRAY_SCALE_COMPUTE_METHOD_AVG)
			output += "   Yval.s1 = (f.s3 + f.s0 + f.s1) * 0.3333333333f;\n";
		else
		{
			output +=
				"    Yval.s1 = mad(f.s3, f.s3, mad(f.s0, f.s0, f.s1 * f.s1));\n"
				"    Yval.s1 = sqrt(Yval.s1 * 0.3333333333f);\n";
		}
#endif
		output +=
			"    } \n";
	}
	return output;
}
std::string warp_rgb6_bicubic_rgb6(bool bWriteU8Image){
	std::string output =
		"    uint8 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    uint invalidPix0 = (((uint)0) << 16) + 0;\n"
		"    uint invalidPix1 = (((uint)16384) << 16) + 0;\n"
		"    bool isSrcInvalid;"
		"    // pixel[0]\n"
		"    if( gid & 1){\n"
		"		sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 8, (int) 0, (int)ip_width*8); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); f  =  interpolate_cubic_rgb6(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));                             f += (interpolate_cubic_rgb6(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + 1*ip_stride));               f += (interpolate_cubic_rgb6(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1 * ip_stride)); f += (interpolate_cubic_rgb6(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f = clamp (f, 0.0f, 32767.0f);\n"
		"    outpix.s0 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s1 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s0 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	output +=
		"    // pixel[1]\n"
		"    if( gid & 1){\n"
		"		sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) {isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 8, (int) 0, (int)ip_width*8); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); f  =  interpolate_cubic_rgb6(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));                             f += (interpolate_cubic_rgb6(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + 1*ip_stride));               f += (interpolate_cubic_rgb6(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1 * ip_stride)); f += (interpolate_cubic_rgb6(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    f = clamp (f, 0.0f, 32767.0f);\n"
		"    outpix.s2 = select( (((uint)f.s1) << 16) + (uint)f.s0, invalidPix0, isSrcInvalid);\n"
		"    outpix.s3 = select( (((uint)f.s3) << 16) + (uint)f.s2, invalidPix1, isSrcInvalid);\n";
	if (bWriteU8Image) {
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = select(mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2)), -10.0f, isSrcInvalid);\n";
#else
		output += "    Yval.s1 = select(f.s3, -10.0f, isSrcInvalid);\n";
#endif
	}
	return output;
}
std::string warp_rgb6_bicubic_rgb4(bool bWriteU8Image){
	std::string output =
		"    uint8 px;\n"
		"    __global uchar * pt;\n"
		"    uint4 outpix, outpix1;\n"
		"    bool isSrcInvalid; float4 tf;\n"
		"    // pixel[0]\n"
		"    if( gid & 1){\n"
		"		sx = map.s2 & 0xffff; sy = (map.s2 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s0 & 0xffff; sy = (map.s0 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 8, (int) 0, (int)ip_width*8); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); tf  =  interpolate_cubic_rgb6(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));                             tf += (interpolate_cubic_rgb6(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + 1*ip_stride));               tf += (interpolate_cubic_rgb6(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1 * ip_stride)); tf += (interpolate_cubic_rgb6(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    if(isSrcInvalid){f.s012 = (float3)32768;}\n"
		"    else{f.s012 = clamp (tf.s012, 0.0f, 32767.0f);}\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s0 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s0 = mad(f.s0, RGBToY.s0, mad(f.s1, RGBToY.s1, f.s2 * RGBToY.s2));\n";
#else
		output += "    Yval.s0 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
		output +=
			"    } \n";
	}
	output +=
		"    // pixel[1]\n"
		"    if( gid & 1){\n"
		"		sx = map.s3 & 0xffff; sy = (map.s3 >> 16) & 0xffff;\n"
		"    }\n"
		"    else{\n"
		"		sx = map.s1 & 0xffff; sy = (map.s1 >> 16) & 0xffff;\n"
		"    }\n"
		"    isSrcInvalid = false;\n"
		"    if(sx == 0xffff && sy == 0xffff) { isSrcInvalid = true; sx = 1 << QF; sy = 1 << QF; }\n"
		"    mf = compute_bicubic_coeffs((sx & QFB) * QFM); y = (sy & QFB) * QFM;\n"
		"    offset = ((sy >> QF)) * ip_stride + clamp((int)((sx >> QF) - 1) * 8, (int) 0, (int)ip_width*8); pt = ip_buf + offset;\n"
		"    if( (sy >> QF)==0){row_offset.s0=0;} else{row_offset.s0=1;}\n"
		"    if( (sy >> QF)>(ip_image_height_offset-3)){row_offset.s1=1;} else{row_offset.s1=2;}\n"
		"    px = vload8(0, (__global uint *)(pt - row_offset.s0 * ip_stride)); tf  =  interpolate_cubic_rgb6(px, mf) * (-0.5f*y + y*y - 0.5f*y*y*y);\n"
		"    px = vload8(0, (__global uint *)(pt));                             tf += (interpolate_cubic_rgb6(px, mf) * (1.0f - 2.5f*y*y + 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + 1*ip_stride));               tf += (interpolate_cubic_rgb6(px, mf) * (0.5f*y + 2.0f*y*y - 1.5f*y*y*y));\n"
		"    px = vload8(0, (__global uint *)(pt + row_offset.s1 * ip_stride)); tf += (interpolate_cubic_rgb6(px, mf) * (-0.5f*y*y + 0.5f*y*y*y));\n"
		"    if(isSrcInvalid){f.s3 = 32768;}\n"
		"    else{f.s3 = clamp (tf.s0, 0.0f, 32767.0f);}\n"
		"    outpix.s0 = (((uint)f.s1) << 16) + (uint)f.s0;\n"
		"    outpix.s1 =(((uint)f.s3) << 16) + (uint)f.s2;\n"
		"    if(isSrcInvalid){f.s01 = (float2)32768;}\n"
		"    else{f.s01 = clamp (tf.s12, 0.0f, 32767.0f);}\n"
		"    outpix.s2 = (((uint)f.s1) << 16) + (uint)f.s0;\n";
	if (bWriteU8Image) {
		output +=
			"    if(isSrcInvalid) { Yval.s1 = -10.0f; }\n"
			"    else { \n";
#if WRITE_LUMA_AS_A
		output += "    Yval.s1 = mad(f.s3, RGBToY.s0, mad(f.s0, RGBToY.s1, f.s1 * RGBToY.s2));\n";
#else
		output += "    Yval.s1 = mulFactor * ((amd_unpack3(px0.s0) * mf.s2 + amd_unpack3(px0.s1) * mf.s0) * mf.s3 + (amd_unpack3(px1.s0) * mf.s2 + amd_unpack3(px1.s1) * mf.s0) * mf.s1);\n";
#endif
		output +=
			"    } \n";
	}
	return output;
}

// Write output -------------------------------------------------------------------------------------------------------------------------------
std::string write_4pixels_to_RGBX(){
	std::string output=
		"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x << 5) + ((gid & 1) << 4);\n"
		"    *(__global uint4 *) op_buf = outpix;\n";
	return output;
}
std::string write_4pixels_to_RGB(){
	std::string output =
		"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x * 24) + ((gid & 1) * 12);\n"
		"    *(__global uint3 *) (op_buf) = outpix.s012;\n";
	return output;
}
std::string write_4pixels_to_RGB4(){
	std::string output =
		"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x * 48) + ((gid & 1) *24);\n"
		"    *(__global uint4 *) op_buf = outpix.s0123;\n"
		"    *(__global uint2 *) &op_buf[16] = outpix1.s01;\n";
	return output;
}
std::string write_4pixels_to_RGB6(){
	std::string output =
		"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x << 6) + (((gid) & 1) << 5);\n"
		"    *(__global uint4 *) op_buf = outpix;\n"
		"    *(__global uint4 *) (op_buf+16) = outpix1;\n";
	return output;
}
std::string write_2pixels_to_RGB4(){
	std::string output =
		"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x * 48) + (((gid>>1) & 1) *24) + ((gid & 1) *12);\n"
		"    *(__global uint2 *) op_buf = outpix.s01;\n"
		"    *(__global uint *)&op_buf[8] = outpix.s2;\n";
	return output;
}
std::string write_2pixels_to_RGB6(){
	std::string output =
		"    op_buf += op_offset + ((camera_id * op_image_height_offset + op_y) * op_stride) + (op_x << 6) + (((gid>>1) & 1) << 5) + ((gid & 1) << 4);\n"
		"    *(__global uint4 *) op_buf = outpix;\n";
	return output;
}
std::string write_exp_comp_img(bool write4pixels){
	std::string output =
		"	float2 output;"
		"   if(Yval.s0 == -10.0f){output.s0 = 32768;}\n"
		"   else{output.s0 = clamp (Yval.s0, 0.0f, 32767.0f);}\n"
		"   if(Yval.s1 == -10.0f){output.s1 = 32768;}\n"
		"   else{output.s1 = clamp (Yval.s1, 0.0f, 32767.0f);}\n";
	if (write4pixels){
		output +=
			"   op_s16_buf += op_s16_offset + ((camera_id * op_s16_image_height_offset + op_y) * op_s16_stride) + (op_x << 4) + (((gid) & 1) << 3);\n"
			"   *(__global uint *) op_s16_buf = (((uint)output.s1) << 16) + (uint)output.s0;\n"
			"   if(Yval.s2 == -10.0f){output.s0 = 32768;}\n"
			"   else{output.s0 = clamp (Yval.s2, 0.0f, 32767.0f);}\n"
			"   if(Yval.s3 == -10.0f){output.s1 = 32768;}\n"
			"   else{output.s1 = clamp (Yval.s3, 0.0f, 32767.0f);}\n"
			"   *(__global uint *) (op_s16_buf+4) = (((uint)output.s1) << 16) + (uint)output.s0;\n";
	}
	else{ //Write 2 pixels
		output +=
			"    op_s16_buf += op_s16_offset + ((camera_id * op_s16_image_height_offset + op_y) * op_s16_stride) + (op_x << 4) + (((gid>>1) & 1) << 3) + ((gid & 1) << 2);\n"
			"    *(__global uint *) op_s16_buf = (((uint)output.s1) << 16) + (uint)output.s0;\n";
	}
	return output;
}
std::string write_luma_U8_8bit_input(){
	std::string output = 
		"    op_u8_buf += op_u8_offset + ((camera_id * op_u8_image_height_offset + op_y) * op_u8_stride) + (op_x << 3) + ((gid & 1) << 2);\n"
		"    *(__global uint *) op_u8_buf = amd_pack(Yval.s0123);\n";
	return output;
}
std::string write_luma_U8_16bit_input(bool write4pixels){
	std::string output = 
		"   Yval = clamp (Yval, 0.0f, 32767.0f);\n"
		"	Yval = Yval * 0.007782219916379f;\n"; //255/32767
	if (write4pixels){
		output +=
			"    op_u8_buf += op_u8_offset + ((camera_id * op_u8_image_height_offset + op_y) * op_u8_stride) + (op_x << 3) + ((gid & 1) << 2);\n"
			"    *(__global uint *) op_u8_buf = amd_pack(Yval.s0123);\n";
	}
	else{ //2 pixels
		output +=
			"    op_u8_buf += op_u8_offset + ((camera_id * op_u8_image_height_offset + op_y) * op_u8_stride) + (op_x << 3) + (((gid>>1) & 1) << 2) + ((gid & 1) << 1);\n"
			"    *(__global short *) op_u8_buf = (((short)Yval.s1) << 8) + (short)Yval.s0;\n";
	}
	return output;
}
std::string write_luma_S16_16bit_input(bool write4pixels){
	std::string output;
	if (write4pixels){
		output =
			"   Yval = clamp (Yval, 0.0f, 32767.0f);\n"
			"    op_u8_buf += op_u8_offset + ((camera_id * op_u8_image_height_offset + op_y) * op_u8_stride) + (op_x << 4) + (((gid) & 1) << 3);\n"
			"    *(__global uint *) op_u8_buf = (((uint)Yval.s1) << 16) + (uint)Yval.s0;\n"
			"    *(__global uint *) (op_u8_buf+4) = (((uint)Yval.s3) << 16) + (uint)Yval.s2;\n";
	}
	else{ //2 pixels
		output =
			"   Yval = clamp (Yval, 0.0f, 32767.0f);\n"
			"    op_u8_buf += op_u8_offset + ((camera_id * op_u8_image_height_offset + op_y) * op_u8_stride) + (op_x << 4) + (((gid>>1) & 1) << 3) + ((gid & 1) << 2);\n"
			"    *(__global uint *) op_u8_buf = (((uint)Yval.s1) << 16) + (uint)Yval.s0;\n";
	}
	return output;
}

// Define helper functions --------------------------------------------------------------------------------------------------------------------
std::string Create_compute_bicubic_coeffs(){
	std::string output =
		"float4 compute_bicubic_coeffs(float x)\n"
		"{\n"
		"  float4 mf;\n"
		"  mf.s0 = -0.5f*x + x*x - 0.5f*x*x*x;\n"
		"  mf.s1 = 1.0f - 2.5f*x*x + 1.5f*x*x*x;\n"
		"  mf.s2 = 0.5f*x + 2.0f*x*x - 1.5f*x*x*x;\n"
		"  mf.s3 = 0.5f*(-x*x + x*x*x);\n"
		"  return(mf);\n"
		"}\n";
	return output;
}
std::string Create_amd_unpack(){
	std::string output =
		"float4 amd_unpack(uint src)\n"
		"{\n"
		"  return (float4)(amd_unpack0(src), amd_unpack1(src), amd_unpack2(src), amd_unpack3(src));\n"
		"}\n";
	return output;
}
std::string Create_interpolate_cubic_rgb(){
	std::string output =
		"float3 interpolate_cubic_rgb(uint4 pix, float4 mf)\n"
		"{\n"
		"  float3 res;\n"
		"  res  = ((float3)(amd_unpack0(pix.s0), amd_unpack1(pix.s0), amd_unpack2(pix.s0))) * mf.s0;\n"
		"  res += ((float3)(amd_unpack3(pix.s0), amd_unpack0(pix.s1), amd_unpack1(pix.s1))) * mf.s1;\n"
		"  res += ((float3)(amd_unpack2(pix.s1), amd_unpack3(pix.s1), amd_unpack0(pix.s2))) * mf.s2;\n"
		"  res += ((float3)(amd_unpack1(pix.s2), amd_unpack2(pix.s2), amd_unpack3(pix.s2))) * mf.s3;\n"
		"  return(res);\n"
		"}\n";
	return output;
}
std::string Create_interpolate_cubic_rgbx(){
	std::string output =
		"float4 interpolate_cubic_rgbx(uint4 pix, float4 mf)\n"
		"{\n"
		"  return(mad(amd_unpack(pix.s0), (float4)mf.s0, mad(amd_unpack(pix.s1), (float4)mf.s1, mad(amd_unpack(pix.s2), (float4)mf.s2, amd_unpack(pix.s3) * mf.s3))));\n"
		"}\n"
		"\n";
	return output;
}
std::string Create_amd_unpack15(){
	std::string output =
		"float4 amd_unpack15(uint src0, uint src1)\n"
		"{\n"
		"  return (float4)((src0 & 0x7fff), (src0 >> 16), (src1 & 0x7fff), (src1 >> 16));\n"
		"}\n";
	return output;
}
std::string Create_interpolate_cubic_rgb4(){
	std::string output =
		"float3 interpolate_cubic_rgb4(uint8 pix, float4 mf)\n"
		"{\n"
		"  float3 res;\n"
		"  res  = ((float3)((pix.s0 & 0x7fff), ((pix.s0 >> 16) & 0x7fff), (pix.s1 & 0x7fff))) * mf.s0;\n"
		"  res += ((float3)(((pix.s1 >> 16) & 0x7fff), (pix.s2 & 0x7fff), ((pix.s2 >> 16) & 0x7fff))) * mf.s1;\n"
		"  res += ((float3)((pix.s3 & 0x7fff), ((pix.s3 >> 16) & 0x7fff), (pix.s4 & 0x7fff))) * mf.s2;\n"
		"  res += ((float3)(((pix.s4 >> 16) & 0x7fff), (pix.s5 & 0x7fff), ((pix.s5 >> 16) & 0x7fff))) * mf.s3;\n"
		"  return(res);\n"
		"}\n";
	return output;
}
std::string Create_interpolate_cubic_rgb6(){
	std::string output =
		"float4 interpolate_cubic_rgb6(uint8 pix, float4 mf)\n"
		"{\n"
		"  return(mad(amd_unpack15(pix.s0,pix.s1), (float4)mf.s0, mad(amd_unpack15(pix.s2,pix.s3), (float4)mf.s1, mad(amd_unpack15(pix.s4,pix.s5), (float4)mf.s2, amd_unpack15(pix.s6,pix.s7) * mf.s3))));\n"
		"}\n"
		"\n";
	return output;
}

