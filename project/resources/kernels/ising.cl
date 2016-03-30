float int_to_chance(int x){
  union int_and_float{
    int i;
    float f;
  };
  union int_and_float x_if;
  x_if.i = x & ((1 << 23) -1) | (127 << 23); //bitmask the fractional part, first 23 bits, and set the exponent so that it's in the range 1 to ~2, uniform 23 bits
  return x_if.f-1;
}

int update_rng(int r){
  return 0xFFFFFFFF & (r * 0x5DEECE66DL+ 0xBL);
}

inline void ising(__global int* spins, __global int *rng, constant uint *chances, uint offset){
   uint N_x = 2*get_global_size(0);
   uint N_y = get_global_size(1);
   uint y = get_global_id(1);
   uint x = ((offset+2*get_global_id(0))+(y&1)) & (N_x-1);

   int N = spins[((y+1)&(N_y-1))*N_x+x];
   int S = spins[((y-1)&(N_y-1))*N_x+x];
   int E = spins[y*N_x+((x+1)&(N_x-1))];
   int W = spins[y*N_x+((x-1)&(N_x-1))];
   int C = spins[y*N_x+x];
   int index = (N+E+S+W)+4+(C>0); //0 to 9.[f=-4, c= -1],[f=-4, c= 1], [f=-2, c=-1], [f=-2, c = 1], etc.

   int rng_sample = update_rng(rng[y*get_global_size(0)+get_global_id(0)]);
   rng[y*get_global_size(0)+get_global_id(0)] = rng_sample;

   spins[y*N_x+x] = C*(-2.0f*(chances[index] > rng_sample)+1.0f);
}

__kernel void naive_red(__global int* spins, __global int *rng, constant uint *chances){
  ising(spins, rng,chances,0);
}

__kernel void naive_black(__global int* spins, __global int *rng, constant uint *chances){
  ising(spins, rng,chances,1);
}

__kernel void compute_energy_field(__global int *spins, global float *energy_field){
  uint N_x = get_global_size(0);
  uint N_y = get_global_size(1);
  uint x = get_global_id(0);
  uint y = get_global_id(1);
  int N = spins[((y+1)&(N_y-1))*N_x+x];
  int S = spins[((y-1)&(N_y-1))*N_x+x];
  int E = spins[y*N_x+((x+1)&(N_x-1))];
  int W = spins[y*N_x+((x-1)&(N_x-1))];
  int C = spins[y*N_x+x];
  energy_field[y*N_x+x] = -0.5*C*(N+E+S+W);
}
