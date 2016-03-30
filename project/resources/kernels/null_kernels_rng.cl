float int_to_chance(int x){
  union int_and_float{
    int i;
    float f;
  };
  union int_and_float x_if;
  x_if.i = x & ((1 << 24) -1) | (126 << 23); //bitmask the fractional part, first 23 bits, and set the exponent so that it's in the range 0 to 1.
  return x_if.f;
}

long update_rng(long r){
  return ((1 << 48)-1) & (r * 0x5DEECE66DL+ 0xBL);
}

inline void naive(__global float* spins, __global int *rng,  float J,  float B,  uint offset ){
   uint N_x = 2*get_global_size(0);
   uint N_y = get_global_size(1);
   uint y = get_global_id(1);

   int rng_sample = update_rng(rng[y*N_x/2+get_global_id(0)]);
   rng[y*N_x/2+get_global_id(0)] = rng_sample;
   float chance = int_to_chance(rng_sample);
}

__kernel void naive_red(__global float* spins, __global float *rng,  float J,  float B){
  naive(spins, rng, J, B, 0);
}

__kernel void naive_black(__global float* spins, __global float *rng,  float J,  float B){
  naive(spins, rng, J, B, 1);
}
