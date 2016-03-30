float int_to_chance(int x){
  union int_and_float{
    int i;
    float f;
  };
  union int_and_float x_if;
  x_if.i = x & ((1 << 23) -1) | (127 << 23); //bitmask the fractional part, first 23 bits, and set the exponent so that it's in the range 1 to ~2, uniform 23 bits
  return x_if.f-1;
}

long update_rng(long r){
  return ((((long)1) << 48)-1) & (r * 0x5DEECE66DL+ 0xBL);
}

inline void naive(__global float* spins, __global int *rng,  float J,  float B,  uint offset ){
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
   int rng_sample = update_rng(rng[y*N_x/2+get_global_id(0)]);
   rng[y*N_x/2+get_global_id(0)] = rng_sample;
   float chance = int_to_chance(rng_sample);
   spins[y*N_x+x] = C*(1.0f-2.0f*(acceptance_ratio > chance));
}

__kernel void naive_red(__global float* spins, __global int *rng,  float J,  float B){
  naive(spins, rng, J, B, 0);
}

__kernel void naive_black(__global float* spins, __global int *rng,  float J,  float B){
  naive(spins, rng, J, B, 1);
}

__kernel void compute_energy_field(__global float *spins, global float *energy_field, float J){
  uint N_x = get_global_size(0);
  uint N_y = get_global_size(1);
  uint x = get_global_id(0);
  uint y = get_global_id(1);
  float N = spins[((y+1)&(N_y-1))*N_x+x];
  float S = spins[((y-1)&(N_y-1))*N_x+x];
  float E = spins[y*N_x+((x+1)&(N_x-1))];
  float W = spins[y*N_x+((x-1)&(N_x-1))];
  float C = spins[y*N_x+x];
  energy_field[y*N_x+x] = -0.5*C*(N+E+S+W);
}
