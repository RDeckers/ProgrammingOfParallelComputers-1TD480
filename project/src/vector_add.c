#include <clu.h>
#include <utilities/logging.h>
#include <utilities/file.h>

int main(int argc, char **argv){
  REPORT_W_COLORS = 1;
  cl_platform_id *platforms = NULL;
  cl_device_id *devices = NULL;
  cl_uint n_platforms = cluGetPlatforms(&platforms, CLU_DYNAMIC);
  if(!n_platforms){
    report(FAIL, "No OpenCL platforms found!");
    return -1;
  }
  cl_context context;
  int device_count = 0;
  for(unsigned p = 0; p < n_platforms; p++){
    if(device_count = cluGetDevices(platforms[p], CL_DEVICE_TYPE_GPU, CLU_DYNAMIC, &devices)){
      context = cluCreateContextFromTypes(platforms[p], CL_DEVICE_TYPE_GPU);
      break;
    }
  }
  if(!device_count){
    report(WARN, "No GPU system found, falling back to CPU");
    for(unsigned p = 0; p < n_platforms; p++){
      if(device_count = cluGetDevices(platforms[p], CL_DEVICE_TYPE_CPU, CLU_DYNAMIC, &devices)){
        context = cluCreateContextFromTypes(platforms[p], CL_DEVICE_TYPE_CPU);
        break;
      }
    }
  }
  if(!device_count){
    report(FAIL, "No GPU or CPUs found, but a platform was found...");
    return -1;
  }
  report(PASS, "Created a context with %d device(s)", device_count);
  cl_command_queue *com_qs = NULL;
  cluCreateCommandQueues(context, devices, device_count, &com_qs);
  report(PASS, "Command queues created");
  cl_int ret;
  const size_t list_size = 16;
  float A[list_size];
  float B[list_size];
  float C[list_size];
  for(unsigned u = 0; u < list_size; u++){
    A[u] = u;
    B[u] = list_size-u;
  }

  cl_mem mem_A = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, list_size*sizeof(float), A, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (A) returned: %s (%d)", cluErrorString(ret), ret);
  }
  cl_mem mem_B = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, list_size*sizeof(float), B, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (B) returned: %s (%d)", cluErrorString(ret), ret);
  }
  cl_mem mem_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, list_size*sizeof(float), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (C) returned: %s (%d)", cluErrorString(ret), ret);
  }

  //clEnqueueWriteBuffer(com_qs[0], mem_A, CL_TRUE, 0, list_size * sizeof(float), A, 0, NULL, NULL);
  //clEnqueueWriteBuffer(com_qs[0], mem_B, CL_TRUE, 0, list_size * sizeof(float), B, 0, NULL, NULL);

  set_cwdir_to_bin_dir();
  char *program_src = NULL;
  //report(INFO, "file contains: \"%s\"", file_to_string("../../resources/kernels/vector_add.cl", 0, &program_src));
  cl_program program = cluProgramFromFilename(context, "../resources/kernels/vector_add.cl");
  ret = clBuildProgram(program, device_count, devices, NULL, NULL, NULL);
  if(CL_SUCCESS != ret){
    report(FAIL, "clBuildProgram returned: %s (%d)", cluErrorString(ret), ret);
  }
  cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateKernel returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&mem_A);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&mem_B);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&mem_C);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg returned: %s (%d)", cluErrorString(ret), ret);
  }

  // Execute the OpenCL kernel on the list
  if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernel, 1, NULL, &list_size, NULL, 0, NULL, NULL))){
    report(FAIL, "enqueue returned: %s (%d)",cluErrorString(ret), ret);
    return -1;
  }

  ret = clEnqueueReadBuffer(com_qs[0], mem_C, CL_TRUE, 0,list_size * sizeof(float), C, 0, NULL, NULL);
  if(CL_SUCCESS != ret){
    report(FAIL, "clEnqueueReadBuffer returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clFlush(com_qs[0]);
  if(CL_SUCCESS != ret){
    report(FAIL, "clFlush returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clFinish(com_qs[0]);
  if(CL_SUCCESS != ret){
    report(FAIL, "clFinish returned: %s (%d)", cluErrorString(ret), ret);
  }
  // Display the result to the screen
  for(int i = 0; i < list_size; i++)
   printf("%.0f + %.0f = %.0f\n", A[i], B[i], C[i]);
}
