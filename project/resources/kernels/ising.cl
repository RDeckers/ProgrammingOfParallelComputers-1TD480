uint update_rng(global uint *r){
  return (*r = *r * 0xDEECE66D + 0xB);
}

uint get_flip_mask(uint4 indices, uint rng_start, constant uint *chances){
  uint flip_mask = 0;
  uint4 rng_sample = (uint4)(rng_start)^(uint4)(0xDeadbeef, 0xAbba0B00, 0xEa7Cebab, 0x12345678);
  for(uint u = 0; u < 8; u++){
    rng_sample = rng_sample *  0xDEECE66D + 0xB;
    uint4 masked_indices = indices >> 4*u & 15;
    flip_mask |=
    ((chances[masked_indices.x]>rng_sample.x) << (4*u+0)) |
    ((chances[masked_indices.y]>rng_sample.y) << (4*u+1)) |
    ((chances[masked_indices.z]>rng_sample.z) << (4*u+2)) |
    ((chances[masked_indices.w]>rng_sample.w) << (4*u+3));
  }
  return flip_mask;
}

uint get_new_state(__local uint* spins, uint x, uint y, uint stride, global uint *rng, constant uint *chances){
  barrier(CLK_LOCAL_MEM_FENCE);
  uint N = spins[x+(y-1)*stride];
  uint S = spins[x+(y+1)*stride];
  uint E = spins[(x+1)+y*stride];
  uint W = spins[(x-1)+y*stride];
  uint C = spins[x+y*stride];

  uint W_mask = ((y^x)&1)*0xFFFFFFFF; //if true, west needs updating.
  uint E_mask = ~W_mask;
  E =  (E_mask & ((E >> 31) | (W << 1))) | (W_mask & E);
  W = (W_mask & ((E >> 1) | ( (1&W) << 31))) | (E_mask & W);

  uint4 shifts = (uint4)(0,1,2,3);
  uint4 masks = (uint4)(0x11111111) << shifts;
  //TODO: reduce ops.
  uint4 indices =
  ((
    (((uint4)(N)&masks) >> shifts) +
    (((uint4)(E)&masks) >> shifts) +
    (((uint4)(S)&masks) >> shifts) +
    (((uint4)(W)&masks) >> shifts)
  ) << 1) | (((uint4)(C) & masks) >> shifts);
  uint flip_mask = get_flip_mask(indices, rng, chances);
  return C^flip_mask;
}

//TODO: add attribute to fix possible size.
void blocked_ising(__global uint* spins, __global uint *rng, constant uint *chances, local uint* local_spins, int color, const uint iterations){
  uint N_x = 4*get_global_size(0);//global size is only 1/4 of the actual size, because we use double red-black nesting.
  uint N_y = get_global_size(1);

  uint group_offset_x = (get_group_id(0)+(color^(get_group_id(1)&1))) *2*get_local_size(0);
  uint x_base = group_offset_x+2*get_local_id(0);
  uint y = get_global_id(1);
  uint x_red = x_base+(y&1);
  uint x_black = x_base + (1^(y&1));

  uint local_base_x = 2*get_local_id(0)+1;//+1 for padding of borders
  uint local_red = local_base_x + (y&1);
  uint local_black = local_base_x + (1^(y&1));

  uint local_stride = (2*get_local_size(0)+2);
  uint local_y = (1+get_local_id(1));

  local_spins[local_red+local_y*local_stride] = spins[y*N_x+x_red];
  local_spins[local_black+local_y*local_stride] = spins[y*N_x+x_black];
  local_spins[get_local_id(0)*(local_base_x+2)+local_y*local_stride] = spins[y*N_x+((x_base-1+3*get_local_id(0))&(N_x-1))]; //borders, horizontal

  if(0 == get_local_id(1)){
    local_spins[local_red] = spins[((y-1)&(N_y-1))*N_x+x_red];
    local_spins[local_black] = spins[((y-1)&(N_y-1))*N_x+x_black];
  }
  else if(get_local_size(1)-1 == get_local_id(1)){
    local_spins[local_red+(local_y+1)*local_stride] = spins[((y+1)&(N_y-1))*N_x+x_red];
    local_spins[local_black+(local_y+1)*local_stride] = spins[((y+1)&(N_y-1))*N_x+x_black];
  }

  uint rn = rng[get_global_id(0)+get_global_size(0)*get_global_id(1)];
  uint red, black;
  for(int i = 0; i < iterations; i++){
    rn = rn *  0xDEECE66D + 0xB;
    red = get_new_state(local_spins, local_red, local_y, local_stride, rn, chances);
    local_spins[local_red+local_y*local_stride] = red;
    black = get_new_state(local_spins, local_black, local_y, local_stride, rn, chances);
    local_spins[local_black+local_y*local_stride] = black;
  }
  rng[get_global_id(0)+get_global_size(0)*get_global_id(1)] = rn;
  spins[y*N_x+x_red] = red;
  spins[y*N_x+x_black] = black;
}

__kernel void naive_red(__global uint* spins, __global uint *rng, constant uint *chances, local uint* local_spins, const uint iterations){
  blocked_ising(spins, rng, chances, local_spins, 0, iterations);
}

__kernel void naive_black(__global uint* spins, __global uint *rng, constant uint *chances, local uint *local_spins, const uint iterations){
  blocked_ising(spins, rng, chances, local_spins, 1, iterations);
}


__kernel void compute_energy_field(__global uint *spins, global float *energy_field){
  uint N_x = get_global_size(0);
  uint N_y = get_global_size(1);
  uint x = get_global_id(0);
  uint y = get_global_id(1);

  uint N = spins[((y+1)&(N_y-1))*N_x+x];
  uint S = spins[((y-1)&(N_y-1))*N_x+x];
  uint E = spins[y*N_x+((x+1)&(N_x-1))];
  uint W = spins[y*N_x+((x-1)&(N_x-1))];

  uint W_mask = ((y^x)&1)*0xFFFFFFFF; //if true, west needs updating.
  uint E_mask = ~W_mask;
  E =  (E_mask & ((E >> 31) | (W << 1))) | (W_mask & E);
  W = (W_mask & ((E >> 1) | ( (1&W) << 31))) | (E_mask & W);

  uint C = spins[y*N_x+x];
  for(unsigned b = 0; b < 32; b++){
    float Cf = 1.0f - 2.0f*((C >> b)&1);
    float Nf = 1.0f - 2.0f*((N >> b)&1);
    float Ef = 1.0f - 2.0f*((E >> b)&1);
    float Sf = 1.0f - 2.0f*((S >> b)&1);
    float Wf = 1.0f - 2.0f*((W >> b)&1);
    energy_field[32*(y*N_x+x)+b] = -0.5*Cf*(Nf+Ef+Sf+Wf);
  }
}
