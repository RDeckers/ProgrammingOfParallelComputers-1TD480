#include <clu.h>
#include <utilities/logging.h>
#include <utilities/file.h>
#include <utilities/benchmarking.h>
#include <math.h>
#include <time.h>
#include <errno.h>

void fill_rng(cl_float *array, size_t n){
  for(unsigned u = 0; u < n; u++)
  array[u] = ((cl_float) rand())/RAND_MAX;
}

int main(int argc, char **argv){
  int null_kernel = 0;
  int size = 1 << 7;
  size_t n_loops = 1 << 12;
  size_t avg_loops = 10;
  cl_float B = 0.0;
  cl_float coupling = 0.2237;
  srand(time(NULL));
  REPORT_W_COLORS = 1;
  int arg_i;
  for(arg_i = 0; arg_i < argc; arg_i++){
    if(!strcmp(argv[arg_i], "-s")){
      size = 1 << atoi(argv[++arg_i]);
      report(INFO, "Size set to %u", size);
    }
    if(!strcmp(argv[arg_i], "-l")){
      n_loops = 1 << atoi(argv[++arg_i]);
      report(INFO, "n_loops set to %u", n_loops);
    }
    if(!strcmp(argv[arg_i], "-B")){
      B = atof(argv[++arg_i]);
      report(INFO, "B set to %f", B);
    }
    if(!strcmp(argv[arg_i], "-J")){
      coupling = atof(argv[++arg_i]);
      report(INFO, "coupling set to %f", coupling);
    }
    if(!strcmp(argv[arg_i], "--transfers-only")){
      null_kernel = 1;
      report(INFO, "Using Null kernels");
    }
  }
  size_t dimensions[2] = {size,size};


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


  cl_mem mem_spin[1];
  mem_spin[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, dimensions[0]*dimensions[1]*sizeof(cl_float), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (spin) returned: %s (%d)", cluErrorString(ret), ret);
  }

  cl_mem mem_rng[2];
  mem_rng[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, dimensions[0]*dimensions[1]/2*sizeof(cl_float), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (rng) returned: %s (%d)", cluErrorString(ret), ret);
  }
  mem_rng[1] = clCreateBuffer(context, CL_MEM_READ_ONLY, dimensions[0]*dimensions[1]/2*sizeof(cl_float), NULL, &ret);
  if(CL_SUCCESS != ret){
   report(FAIL, "clCreateBuffer (rng) returned: %s (%d)", cluErrorString(ret), ret);
  }

  report(PASS, "Buffers created");

  cl_float *rng_field_a = malloc(sizeof(cl_float)*dimensions[0]*dimensions[1]/2);
  if(NULL == rng_field_a){
    report(FAIL, "Allocation failure: %s (%d)", strerror(errno), errno);
    return -1;
  }
  cl_float *rng_field_b = malloc(sizeof(cl_float)*dimensions[0]*dimensions[1]/2);
  if(NULL == rng_field_b){
    report(FAIL, "Allocation failure: %s (%d)", strerror(errno), errno);
    return -1;
  }
  cl_float *spin_field = malloc(sizeof(cl_float)*dimensions[0]*dimensions[1]);
  if(NULL == spin_field){
    report(FAIL, "Allocation failure: %s (%d)", strerror(errno), errno);
    return -1;
  }
  //clEnqueueWriteBuffer(com_qs[0], mem_B, CL_TRUE, 0, list_size * sizeof(float), B, 0, NULL, NULL);

  set_cwdir_to_bin_dir();
  char *program_src = NULL;
  cl_program program;
  if(null_kernel){
    program = cluProgramFromFilename(context, "../resources/kernels/null_kernels.cl");
  }else{
    program = cluProgramFromFilename(context, "../resources/kernels/naive_ising.cl");
  }
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

  cl_mem mem_energy = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dimensions[0]*dimensions[1]*sizeof(cl_float), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (energy) returned: %s (%d)", cluErrorString(ret), ret);
  }

  report(PASS, "program created");
  cl_kernel kernels[3];

  kernels[0] = clCreateKernel(program, "naive_red", &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateKernel (0) returned: %s (%d)", cluErrorString(ret), ret);
  }

  kernels[1] = clCreateKernel(program, "naive_black", &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateKernel (1) returned: %s (%d)", cluErrorString(ret), ret);
  }

  kernels[2] = clCreateKernel(program, "compute_energy_field", &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateKernel (2) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[0], 0, sizeof(cl_mem), (void *)&(mem_spin[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[0], 1, sizeof(cl_mem), (void *)&(mem_rng[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,1) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[0], 3, sizeof(cl_float), (void *)&B);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,3) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[1], 0, sizeof(cl_mem), (void *)&(mem_spin[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[1], 1, sizeof(cl_mem), (void *)&(mem_rng[1]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,1) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[1], 3, sizeof(cl_float), (void *)&B);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,3) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[1], 2, sizeof(cl_float), (void *)&coupling);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (1,2) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[0], 2, sizeof(cl_float), (void *)&coupling);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,2) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[2], 0, sizeof(cl_mem), (void *)&(mem_spin[0]));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (2,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[2], 1, sizeof(cl_mem), (void *)&(mem_energy));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (2,1) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[2], 2, sizeof(cl_float), (void *)&coupling);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (1,2) returned: %s (%d)", cluErrorString(ret), ret);
  }

  report(PASS, "kernel args set");

  fill_rng(spin_field, dimensions[0]*dimensions[1]);
  for(unsigned u = 0; u < dimensions[0]*dimensions[1]; u++){
    spin_field[u] = 1-2*(rand()&1);
  }
  report(PASS, "spin field filled");
  if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_spin[0], CL_TRUE, 0, dimensions[0]*dimensions[1] * sizeof(cl_float), spin_field, 0, NULL, NULL))){
    report(FAIL, "enqueue write spins returned: %s (%d)",cluErrorString(ret), ret);
  }

  const size_t work_dim[] = {dimensions[0]/2, dimensions[1]};
/*///////////////////////////////////////////////////
// START EXECUTION                                //
/////////////////////////////////////////////////*/
    cl_event *events = malloc(sizeof(cl_event)*n_loops*4);
    if(NULL == events){
      report(FAIL, "Allocation failed: %s (%d)", strerror(errno), errno);
      return -1;
    }
    struct timespec T;
    double time_taken = 0;
    double avg_E = 0;
    for(unsigned w = 0; w < avg_loops; w++){
    tick(&T);
    fill_rng(rng_field_a, dimensions[0]*dimensions[1]/2); //TODO: smaller rng needed
    //upload rng[0]
    if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_rng[0], CL_FALSE, 0, dimensions[0]*dimensions[1]/2 * sizeof(cl_float), rng_field_a, 0, NULL, events))){
      report(FAIL, "enqueue read rng_a returned: %s (%d)",cluErrorString(ret), ret);
      return -1;
    }
    //add kernels[0] dependent upon rng[0] transfer
    size_t work_item_dim[] ={16,16};
    if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[0], 2, NULL, work_dim, work_item_dim, 1, events, events+1))){
      report(FAIL, "enqueue kernel(0) returned: %s (%d)",cluErrorString(ret), ret);
    }
    //(flush kernels to start execution)
    ret = clFlush(com_qs[0]);
    if(CL_SUCCESS != ret){
      report(FAIL, "clFlush returned: %s (%d)", cluErrorString(ret), ret);
    }
    fill_rng(rng_field_b, dimensions[0]*dimensions[1]/2); //TODO: smaller rng needed
    //upload rng[1]
    if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_rng[1], CL_FALSE, 0, dimensions[0]*dimensions[1]/2 * sizeof(cl_float), rng_field_b, 0, NULL, events+2))){
      report(FAIL, "enqueue read rng_a returned: %s (%d)",cluErrorString(ret), ret);
      return -1;
    }
    //kernel[1] dependent upon kernel[0] and rng[1]
    if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[1], 2, NULL, work_dim, work_item_dim, 2, events+1, events+3))){
      report(FAIL, "enqueue kernel(1) returned: %s (%d)",cluErrorString(ret), ret);
    }
    for(unsigned u = 1; u < n_loops; u++){
      //update rng[0], host side.
      fill_rng(rng_field_a, dimensions[0]*dimensions[1]/2);
      //upload rng[0], dependent upon kernel[0] finished
      if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_rng[0], CL_FALSE, 0, dimensions[0]*dimensions[1]/2 * sizeof(cl_float), rng_field_a, 1, events+4*u-3, events+4*u+0))){
        report(FAIL, "enqueue read rng_a returned: %s (%d)",cluErrorString(ret), ret);
        return -1;
      }
      //add kernels[0] dependent upon rng[0] transfer and kernels[1] finished
      if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[0], 2, NULL, work_dim, work_item_dim, 2, events+4*u-1, events+4*u+1))){
        report(FAIL, "enqueue kernel[0] returned: %s (%d)",cluErrorString(ret), ret);
        return -4;
      }
      //(flush)
      ret = clFlush(com_qs[0]);
      if(CL_SUCCESS != ret){
        report(FAIL, "clFlush returned: %s (%d)", cluErrorString(ret), ret);
        return -3;
      }
      //update rng[1]
      fill_rng(rng_field_b, dimensions[0]*dimensions[1]/2);
      //upload rng[1],dependent upon kernel[1] finished (which depends on the previous transfer)
      if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_rng[1], CL_FALSE, 0, dimensions[0]*dimensions[1]/2 * sizeof(cl_float), rng_field_b, 1, events+4*u-1, events+4*u+2))){
        report(FAIL, "enqueue read rng_a returned: %s (%d)",cluErrorString(ret), ret);
        return -1;
      }
      //add kernels[1] dependent upon rng[1] and kernels[0] finished
      if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[1], 2, NULL, work_dim, work_item_dim, 2, events+4*u+1, events+4*u+3))){
        report(FAIL, "enqueue kernel[1] returned: %s (%d)",cluErrorString(ret), ret);
        return -2;
      }
    }

    ret = clFinish(com_qs[0]);
    if(CL_SUCCESS != ret){
      report(FAIL, "clFinish returned: %s (%d)", cluErrorString(ret), ret);
    }
/*/////////////////////////////////////////////////
//  END EXECUTION                               //
///////////////////////////////////////////////*/
time_taken += tock(&T);
if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[2], 2, NULL, dimensions, work_item_dim, 0, NULL, NULL))){
  report(FAIL, "enqueue kernel[2] returned: %s (%d)",cluErrorString(ret), ret);
  return -4;
}
ret = clFinish(com_qs[0]);
if(CL_SUCCESS != ret){
  report(FAIL, "clFinish returned: %s (%d)", cluErrorString(ret), ret);
}
ret = clEnqueueReadBuffer(com_qs[0], mem_energy, CL_TRUE, 0,dimensions[0]*dimensions[1]*sizeof(cl_float), spin_field, 0, NULL, NULL);
if(CL_SUCCESS != ret){
  report(FAIL, "clEnqueueReadBuffer returned: %s (%d)", cluErrorString(ret), ret);
}
double total_E = 0;
for(int y = 0; y < dimensions[1]; y++){
  for(int x = 0; x < dimensions[0]; x++){
     total_E += spin_field[y*dimensions[0]+x];
  }
}
avg_E += total_E;
}
printf("%d %f %e %e\n", size, coupling, fabs(avg_E/avg_loops)/(dimensions[0]*dimensions[1]), time_taken/(avg_loops*n_loops));
}
