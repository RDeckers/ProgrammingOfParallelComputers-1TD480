#include <clu.h>
#include <utilities/logging.h>
int initialize(cl_context *context, cl_device_id **devices, cl_command_queue **queues){
  cl_platform_id *platforms = NULL;
  cl_uint n_platforms = cluGetPlatforms(&platforms, CLU_DYNAMIC);
  if(!n_platforms){
    report(FAIL, "No OpenCL platforms found!");
    return 0;
  }
  int device_count = 0;
  for(unsigned p = 0; p < n_platforms; p++){
    if(device_count = cluGetDevices(platforms[p], CL_DEVICE_TYPE_GPU, CLU_DYNAMIC, devices)){
      *context = cluCreateContextFromTypes(platforms[p], CL_DEVICE_TYPE_GPU);
      break;
    }
  }
  if(!device_count){
    report(WARN, "No GPU system found, falling back to CPU");
    for(unsigned p = 0; p < n_platforms; p++){
      if(device_count = cluGetDevices(platforms[p], CL_DEVICE_TYPE_CPU, CLU_DYNAMIC, devices)){
        *context = cluCreateContextFromTypes(platforms[p], CL_DEVICE_TYPE_CPU);
        break;
      }
    }
  }
  if(!device_count){
    report(FAIL, "No GPU or CPUs found, but a platform was found...");
    return 0;
  }
  report(PASS, "Created a context with %d device(s)", device_count);
  cluCreateCommandQueues(*context, *devices, device_count, queues);
  report(PASS, "Command queues created");
  return device_count;
}
