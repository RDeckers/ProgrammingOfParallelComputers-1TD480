void naive(__global float* spins, constant float *rng,  float J,  float B,  uint offset ){
  uint N_x = 2*get_global_size(0);
  uint N_y = get_global_size(1);
  uint y = get_global_id(1);
  uint x = ((offset+2*get_global_id(0))+(y&1)) & (N_x-1);

  float N = spins[((y+1)&(N_y-1))*N_x+x];
  float S = spins[((y-1)&(N_y-1))*N_x+x];
  float E = spins[y*N_x+((x+1)&(N_x-1))];
  float W = spins[y*N_x+((x-1)&(N_x-1))];
  float C = spins[y*N_x+x];
  float f = N+E+S+W;
  float acceptance_ratio = exp(-2*C*(J*f+B));
  spins[y*N_x+x] = C*(1.0f-2.0f*(acceptance_ratio > rng[y*N_x+x]));
}

__kernel void naive_red(__global float* spins, constant float *rng,  float J,  float B){
  naive(spins, rng, J, B, 0);
}

__kernel void naive_black(__global float* spins, constant float *rng,  float J,  float B){
  naive(spins, rng, J, B, 1);
}
