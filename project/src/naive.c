#include <clu.h>
#include <utilities/logging.h>
#include <utilities/file.h>

void fill_rng(cl_float *array, size_t n){
  for(unsigned u = 0; u < n; u++)
  array[u] = ((cl_float) rand())/RAND_MAX;
}

int main(int argc, char **argv){
  srand(time(NULL));
  REPORT_W_COLORS = 1;
  /*/////////////////////////////////////////////////////
  // START SETUP
  /////////////////////////////////////////////////////*/
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
  const size_t dimensions[2] = {1 << 7, 1 << 7};

  cl_mem mem_spin[2];
  mem_spin[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, dimensions[0]*dimensions[1]*sizeof(cl_float), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (spin) returned: %s (%d)", cluErrorString(ret), ret);
  }
  // mem_spin[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, dimensions[0]*dimensions[1]*sizeof(cl_float), NULL, &ret);
  // if(CL_SUCCESS != ret){
  //   report(FAIL, "clCreateBuffer (spin) returned: %s (%d)", cluErrorString(ret), ret);
  // }
  cl_mem mem_rng[2];
  mem_rng[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, dimensions[0]*dimensions[1]*sizeof(cl_float), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (rng) returned: %s (%d)", cluErrorString(ret), ret);
  }
  // mem_rng[1] = clCreateBuffer(context, CL_MEM_READ_ONLY, dimensions[0]*dimensions[1]*sizeof(cl_float), NULL, &ret);
  // if(CL_SUCCESS != ret){
  //   report(FAIL, "clCreateBuffer (rng) returned: %s (%d)", cluErrorString(ret), ret);
  // }

  cl_float rng_field_a[dimensions[0]*dimensions[1]];
  // cl_float rng_field_b[dimensions[0]*dimensions[1]];
  cl_float spin_field[dimensions[0]*dimensions[1]];
  //clEnqueueWriteBuffer(com_qs[0], mem_B, CL_TRUE, 0, list_size * sizeof(float), B, 0, NULL, NULL);

  set_cwdir_to_bin_dir();
  char *program_src = NULL;
  cl_program program = cluProgramFromFilename(context, "../resources/kernels/naive_ising.cl");
  ret = clBuildProgram(program, device_count, devices, NULL, NULL, NULL);
  if(CL_SUCCESS != ret){
    report(FAIL, "clBuildProgram returned: %s (%d)", cluErrorString(ret), ret);
    char *log = NULL;
    for(int i = 0; i < device_count; i++){
      cluGetProgramLog(program, devices[0], CLU_DYNAMIC, &log);
      report(INFO, "log device[%d]\n==============\n%s",i, log);
    }
    free(log);
  }
  cl_float B = 0.0;
  cl_kernel kernels[2];

  kernels[0] = clCreateKernel(program, "naive_red", &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateKernel (0) returned: %s (%d)", cluErrorString(ret), ret);
  }

  kernels[1] = clCreateKernel(program, "naive_black", &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateKernel (1) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[0], 0, sizeof(cl_mem), (void *)&(mem_spin[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[0], 1, sizeof(cl_mem), (void *)&(mem_rng[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,1) returned: %s (%d)", cluErrorString(ret), ret);
  }
  // ret = clSetKernelArg(kernels[0], 2, sizeof(cl_float), (void *)&coupling);
  // if(CL_SUCCESS != ret){
  //   report(FAIL, "clSetKernelArg (0,2) returned: %s (%d)", cluErrorString(ret), ret);
  // }
  ret = clSetKernelArg(kernels[0], 3, sizeof(cl_float), (void *)&B);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,3) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[1], 0, sizeof(cl_mem), (void *)&(mem_spin[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[1], 1, sizeof(cl_mem), (void *)&(mem_rng[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,1) returned: %s (%d)", cluErrorString(ret), ret);
  }
  // ret = clSetKernelArg(kernels[1], 2, sizeof(cl_float), (void *)&coupling);
  // if(CL_SUCCESS != ret){
  //   report(FAIL, "clSetKernelArg (0,2) returned: %s (%d)", cluErrorString(ret), ret);
  // }
  ret = clSetKernelArg(kernels[1], 3, sizeof(cl_float), (void *)&B);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,3) returned: %s (%d)", cluErrorString(ret), ret);
  }

  fill_rng(spin_field, dimensions[0]*dimensions[1]);
  for(unsigned u = 0; u < dimensions[0]*dimensions[1]; u++){
    spin_field[u] = 1-2*(rand()&1);
  }
  if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_spin[0], CL_TRUE, 0, dimensions[0]*dimensions[1] * sizeof(cl_float), spin_field, 0, NULL, NULL))){
    report(FAIL, "enqueue write spins returned: %s (%d)",cluErrorString(ret), ret);
  }

  for(cl_float coupling = 0.4; coupling <= 0.5; coupling += 0.01){
    ret = clSetKernelArg(kernels[1], 2, sizeof(cl_float), (void *)&coupling);
    if(CL_SUCCESS != ret){
      report(FAIL, "clSetKernelArg (1,2) returned: %s (%d)", cluErrorString(ret), ret);
    }

    ret = clSetKernelArg(kernels[0], 2, sizeof(cl_float), (void *)&coupling);
    if(CL_SUCCESS != ret){
      report(FAIL, "clSetKernelArg (0,2) returned: %s (%d)", cluErrorString(ret), ret);
    }

    const size_t n_loops = 1 << 14;
    const size_t work_dim[] = {dimensions[0]/2, dimensions[1]};
    fill_rng(rng_field_a, dimensions[0]*dimensions[1]);

    if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_rng[0], CL_TRUE, 0, dimensions[0]*dimensions[1] * sizeof(cl_float), rng_field_a, 0, NULL, NULL))){
      report(FAIL, "enqueue read rng_a returned: %s (%d)",cluErrorString(ret), ret);
      break;
    }
    for(unsigned u = 0; u < n_loops; u++){
      //read rng
      if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_rng[0], CL_TRUE, 0, dimensions[0]*dimensions[1] * sizeof(cl_float), rng_field_a, 0, NULL, NULL))){
        report(FAIL, "enqueue read rng_a returned: %s (%d)",cluErrorString(ret), ret);
        break;
      }

      //run kernel 0
      if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[0], 2, NULL, work_dim, NULL, 0, NULL, NULL))){
        report(FAIL, "enqueue kernel[0] returned: %s (%d)",cluErrorString(ret), ret);
        break;
      }
      //run kernel 1
      if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[1], 2, NULL, work_dim, NULL, 0, NULL, NULL))){
        report(FAIL, "enqueue kernel[1] returned: %s (%d)",cluErrorString(ret), ret);
        break;
      }

      ret = clFlush(com_qs[0]);
      if(CL_SUCCESS != ret){
        report(FAIL, "clFlush returned: %s (%d)", cluErrorString(ret), ret);
      }

      fill_rng(rng_field_a, dimensions[0]*dimensions[1]);
      ret = clFinish(com_qs[0]);
      if(CL_SUCCESS != ret){
        report(FAIL, "clFinish returned: %s (%d)", cluErrorString(ret), ret);
      }
    }

    ret = clFinish(com_qs[0]);
    if(CL_SUCCESS != ret){
      report(FAIL, "clFinish returned: %s (%d)", cluErrorString(ret), ret);
    }

    ret = clEnqueueReadBuffer(com_qs[0], mem_spin[0], CL_TRUE, 0,dimensions[0]*dimensions[1]*sizeof(cl_float), spin_field, 0, NULL, NULL);
    if(CL_SUCCESS != ret){
      report(FAIL, "clEnqueueReadBuffer returned: %s (%d)", cluErrorString(ret), ret);
    }
    float total_M = 0;
    for(int y = 0; y < dimensions[1]; y++){
      for(int x = 0; x < dimensions[0]; x++){
        //printf("%s", spin_field[y*dimensions[0]+x] > 0? "\033[1;34mx\033[0m" : "\033[1;36mx\033[0m");
        //printf("%2.0f ", spin_field[y*dimensions[0]+x]);
        //printf("%s", spin_field[y*dimensions[0]+x] > 0? "+" : "-");
        total_M += spin_field[y*dimensions[0]+x];
      }
      //puts("");
    }
    printf("%f %f\n", coupling, fabs(total_M)/(dimensions[0]*dimensions[1]));
  }
}
