#include "best_splits.h"
#define SIZE2SPLITS(SIZE) ( ((SIZE) < min_segment_length*2) ? 0 : (1+(SIZE)-min_segment_length*2) )

Splitter::Splitter
(int n_data_, int min_segment_length_){
  n_data = n_data_;
  min_segment_length = min_segment_length_;
  max_segments = n_data/min_segment_length;
}

int Splitter::best_splits(int *out_splits_, int *out_depth_){
  if(min_segment_length < 1){
    return ERROR_BEST_SPLITS_MIN_SEGMENT_LENGTH_MUST_BE_POSITIVE;
  }
  if(n_data < min_segment_length){
    return ERROR_BEST_SPLITS_N_DATA_MUST_BE_AT_LEAST_MIN_SEGMENT_LENGTH;
  }
  if(0 < max_segments){
    out_splits = out_splits_;
    out_depth = out_depth_;
    out_index=0;
    for(int seg_i=0; seg_i < max_segments; seg_i++){
      write_splits_depth(-1, -1);
    }
    out_index=0;
    children(n_data, 0, 0);
  }
  return 0;
}

void Splitter::write_splits_depth(int splits, int depth){
  out_splits[out_index] = splits;
  out_depth[out_index] = depth;
  out_index++;
}

void Splitter::children
(int smaller_size, int larger_size, int depth){
  int smaller_splits = SIZE2SPLITS(smaller_size);
  int larger_splits = SIZE2SPLITS(larger_size);
  write_splits_depth(smaller_splits + larger_splits, depth); 
  split_if_possible(smaller_size, depth);
  split_if_possible(larger_size, depth);
}

void Splitter::split_if_possible
(int size_to_split, int depth){
  if(size_to_split >= min_segment_length*2){
    int smaller_size = size_to_split / 2;
    int larger_size = smaller_size + size_to_split % 2;
    children(smaller_size, larger_size, depth+1);
  }
}

