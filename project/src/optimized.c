#include <clu.h>
#include <utilities/logging.h>
#include <utilities/file.h>
#include <utilities/benchmarking.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

double compute_average_abs_M(cl_uint *array, size_t L){
  size_t total_set = 0;
  for(unsigned y = 0; y < L; y++){
    for(unsigned x =0 ; x < L/32; x++){
      total_set += __builtin_popcount(array[y*L/32+x]);
    }
  }
  size_t total_size = L*L;
  size_t dominant_spins = total_set > total_size/2 ? total_set : total_size-total_set;
  size_t non_dominant_spins = total_size - dominant_spins;
  size_t abs_M = dominant_spins-non_dominant_spins;
  report(INFO, "total abs M = %u (%u - %u), (%u, %u)", abs_M, dominant_spins, non_dominant_spins, dominant_spins+non_dominant_spins, L*L);
  return abs_M/((double)L*L);
}

void fill_rng(cl_uint *array, size_t n){
  for(unsigned u = 0; u < n; u++)
  array[u] = rand();
}

cl_uint acceptance_chance(cl_float B, cl_float J, cl_float f, cl_float c, cl_uint max){
  uint64_t tmp = max*exp(-2.0*c*(J*f+B));
  report(INFO, "chance should be %f", exp(-2.0*c*(J*f+B)));
  return tmp > max? max:tmp;
}

void compute_acceptance_chances(cl_uint *chances, cl_float B, cl_float J){
    chances[0] = acceptance_chance(B, J, -4, -1, 0xFFFFFFFF);
    chances[1] = acceptance_chance(B, J, -4, 1, 0xFFFFFFFF);
    chances[2] = acceptance_chance(B, J, -2, -1, 0xFFFFFFFF);
    chances[3] = acceptance_chance(B, J, -2, 1, 0xFFFFFFFF);

    chances[4] = acceptance_chance(B, J, 0, -1, 0xFFFFFFFF);
    chances[5] = acceptance_chance(B, J, 0, 1, 0xFFFFFFFF);

    chances[6] = acceptance_chance(B, J, 2, -1, 0xFFFFFFFF);
    chances[7] = acceptance_chance(B, J, 2, 1, 0xFFFFFFFF);
    chances[8] = acceptance_chance(B, J, 4, -1, 0xFFFFFFFF);
    chances[9] = acceptance_chance(B, J, 4, 1, 0xFFFFFFFF);
}

int main(int argc, char **argv){
  report(INFO, "RAND_MAX = %d", RAND_MAX);
  int size = 1 << 7;
  size_t n_loops = 1 << 16;
  cl_float B = 0.0;
  cl_float coupling = 0.2237;
  srand(time(NULL));
  REPORT_W_COLORS = 1;
  size_t avg_loops = 10;
  cl_uint k = 1;
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
    if(!strcmp(argv[arg_i], "-k")){
      k = atoi(argv[++arg_i]);
      report(INFO, "k set to %u", k);
    }
  }
  size_t dimensions[2] = {size,size};
  const size_t work_dim[] = {dimensions[0]/128, dimensions[1]};
  const size_t work_item_dim[] = {2,128};
  cl_uint acceptance_chances[10];
  compute_acceptance_chances(acceptance_chances, B, coupling);
  for(int i = 0; i < 10; i++){
    report(INFO, "chance %u: %u (%f)", i , acceptance_chances[i], acceptance_chances[i]/((float)0xFFFFFFFF));
  }
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


  cl_mem mem_spin, mem_energy, mem_chances;
  mem_spin = clCreateBuffer(context, CL_MEM_READ_WRITE, dimensions[0]/32*dimensions[1]*sizeof(cl_uint), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (spin) returned: %s (%d)", cluErrorString(ret), ret);
  }

  mem_chances = clCreateBuffer(context, CL_MEM_READ_ONLY, 10*sizeof(cl_uint), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (chances) returned: %s (%d)", cluErrorString(ret), ret);
  }

  mem_energy = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dimensions[0]*dimensions[1]*sizeof(cl_float), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (energy) returned: %s (%d)", cluErrorString(ret), ret);
  }

  cl_mem mem_rng;
  mem_rng = clCreateBuffer(context, CL_MEM_READ_ONLY, work_dim[0]*work_dim[1]*sizeof(cl_uint), NULL, &ret);
  if(CL_SUCCESS != ret){
    report(FAIL, "clCreateBuffer (rng) returned: %s (%d)", cluErrorString(ret), ret);
  }

  report(PASS, "Buffers created");

  cl_uint *rng_field = malloc(sizeof(cl_uint)*work_dim[0]*work_dim[1]);
  if(NULL == rng_field){
    report(FAIL, "Allocation failure: %s (%d)", strerror(errno), errno);
    return -1;
  }
  cl_uint *spin_field = malloc(sizeof(cl_uint)*dimensions[0]/32*dimensions[1]);
  if(NULL == spin_field){
    report(FAIL, "Allocation failure: %s (%d)", strerror(errno), errno);
    return -1;
  }
  cl_float *energy_field = malloc(sizeof(cl_float)*dimensions[0]*dimensions[1]);
  if(NULL == energy_field){
    report(FAIL, "Allocation failure: %s (%d)", strerror(errno), errno);
    return -1;
  }
  //clEnqueueWriteBuffer(com_qs[0], mem_B, CL_TRUE, 0, list_size * sizeof(float), B, 0, NULL, NULL);

  set_cwdir_to_bin_dir();
  char *program_src = NULL;
  cl_program program = cluProgramFromFilename(context, "../resources/kernels/ising.cl");
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

  ret = clSetKernelArg(kernels[0], 0, sizeof(cl_mem), (void *)&mem_spin);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[0], 1, sizeof(cl_mem), (void *)&(mem_rng));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,1) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[0], 2, sizeof(cl_mem), (void *)&mem_chances);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,2) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[0],  3, sizeof(cl_uint)*2*(2+work_item_dim[0])*(2+work_item_dim[1]), NULL);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,3) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[0],  4, sizeof(cl_uint), &k);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (0,3) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[1], 0, sizeof(cl_mem), (void *)&mem_spin);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (1,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[1], 1, sizeof(cl_mem), (void *)&(mem_rng));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (1,1) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[1],  2, sizeof(cl_mem), (void *)&mem_chances);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (1,2) returned: %s (%d)", cluErrorString(ret), ret);
  }

  ret = clSetKernelArg(kernels[1],  3, sizeof(cl_uint)*(2*(work_item_dim[0]+1)*(work_item_dim[1]+2)), NULL);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (1,3) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[1],  4, sizeof(cl_uint), &k);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (1,4) returned: %s (%d)", cluErrorString(ret), ret);
  }


  ret = clSetKernelArg(kernels[2], 0, sizeof(cl_mem), (void *)&mem_spin);
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (2,0) returned: %s (%d)", cluErrorString(ret), ret);
  }
  ret = clSetKernelArg(kernels[2], 1, sizeof(cl_mem), (void *)&(mem_energy));
  if(CL_SUCCESS != ret){
    report(FAIL, "clSetKernelArg (2,1) returned: %s (%d)", cluErrorString(ret), ret);
  }

  report(PASS, "kernel args set");

  for(unsigned u = 0; u < dimensions[0]/32*dimensions[1]; u++){
    //workaround for windows' horrible RNG
    cl_uint x = rand() & 0xff;
    x |= (rand() & 0xff) << 8;
    x |= (rand() & 0xff) << 16;
    x |= (rand() & 0xff) << 24;
    spin_field[u] = x;
  }
  report(PASS, "spin field filled");
  if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_spin, CL_TRUE, 0, dimensions[0]/32*dimensions[1] * sizeof(cl_uint), spin_field, 0, NULL, NULL))){
    report(FAIL, "enqueue write spins returned: %s (%d)",cluErrorString(ret), ret);
  }
  if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_chances, CL_TRUE, 0, 10 * sizeof(cl_uint), acceptance_chances, 0, NULL, NULL))){
    report(FAIL, "enqueue write chances returned: %s (%d)",cluErrorString(ret), ret);
    return -1;
  }


  fill_rng(rng_field, work_dim[0]*work_dim[1]);
  //upload rng[0]
  cl_event events[2*n_loops+1];
  if(CL_SUCCESS != (ret = clEnqueueWriteBuffer(com_qs[0], mem_rng, CL_FALSE, 0, work_dim[0]*work_dim[1] * sizeof(cl_uint), rng_field, 0, NULL, events))){
    report(FAIL, "enqueue read rng_a returned: %s (%d)",cluErrorString(ret), ret);
    return -1;
  }
/*///////////////////////////////////////////////////
// START EXECUTION                                //
/////////////////////////////////////////////////*/

    struct timespec T;
    ret = clFinish(com_qs[0]);
    if(CL_SUCCESS != ret){
      report(FAIL, "clFinish returned: %s (%d)", cluErrorString(ret), ret);
    }
    double time_taken = 0;
    double avg_E = 0;
    for(unsigned w = 0; w < avg_loops; w++){
    tick(&T);
    for(unsigned u = 0; u < n_loops/k; u++){
      //add kernels[0]
      if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[0], 2, NULL, work_dim, work_item_dim, 1, events+u*2, events+1+u*2))){
        report(FAIL, "enqueue kernel[0] returned: %s (%d)",cluErrorString(ret), ret);
        return -4;
      }
      if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[1], 2, NULL, work_dim, work_item_dim, 1, events+1+u*2, events+2+u*2))){
        report(FAIL, "enqueue kernel[1] returned: %s (%d)",cluErrorString(ret), ret);
        return -4;
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
    size_t energy_kernel_dim[] = {dimensions[0]/32, dimensions[1]};
    if(CL_SUCCESS != (ret = clEnqueueNDRangeKernel(com_qs[0], kernels[2], 2, NULL, energy_kernel_dim, work_item_dim, 0, NULL, NULL))){
      report(FAIL, "enqueue kernel[2] returned: %s (%d)",cluErrorString(ret), ret);
      return -4;
    }
    ret = clFinish(com_qs[0]);
    if(CL_SUCCESS != ret){
      report(FAIL, "clFinish returned: %s (%d)", cluErrorString(ret), ret);
    }
    report(PASS, "prepping output");
    ret = clEnqueueReadBuffer(com_qs[0], mem_spin, CL_TRUE, 0,dimensions[0]/32*dimensions[1]*sizeof(cl_uint), spin_field, 0, NULL, NULL);
    if(CL_SUCCESS != ret){
      report(FAIL, "clEnqueueReadBuffer returned: %s (%d)", cluErrorString(ret), ret);
    }
    ret = clEnqueueReadBuffer(com_qs[0], mem_energy, CL_TRUE, 0,dimensions[0]*dimensions[1]*sizeof(cl_float), energy_field, 0, NULL, NULL);
    if(CL_SUCCESS != ret){
      report(FAIL, "clEnqueueReadBuffer returned: %s (%d)", cluErrorString(ret), ret);
    }
    report(INFO, "Dimensions: %d x %d\t Work Dimensions: %d x %d", dimensions[0], dimensions[1], work_dim[0], work_dim[1]);
    
    double total_E = 0;
    for(int y = 0; y < dimensions[1]; y++){
      for(int x = 0; x < dimensions[0]; x++){
        total_E += energy_field[y*dimensions[0]+x];
      }
    }
    avg_E += total_E;
  }
    printf("%d %f %e %e\n", size, coupling, fabs(avg_E/avg_loops)/(dimensions[0]*dimensions[1]), time_taken/(avg_loops*n_loops));
}
