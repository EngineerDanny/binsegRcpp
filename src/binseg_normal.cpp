#include "binseg_normal.h"
#include <math.h>//INFINITY
#include <set>//multiset
#include <vector>

// This class saves the optimal parameters/loss value for each segment
// (before and after) resulting from a split. Currently there is only
// one parameter (mean) because the model is normal change in mean
// with constant variance but there could be more parameters for other
// models (e.g., normal change in mean and variance).
class MeanLoss {
public:
  double mean, loss;
};

// This class computes and stores the statistics that we need to
// compute the optimal loss/parameters of a segment from first to
// last. In the case of normal change in mean with constant variance
// the only statistic we need is the cumulative sum.
class SetCumsum {
public:
  std::vector<double> cumsum_vec;
  double get_sum(int first, int last){
    double total = cumsum_vec[last];
    if(0 < first){
      total -= cumsum_vec[first-1];
    }
    return total;
  }
  // Compute/store optimal mean and loss values for a single segment
  // from first to last in constant O(1) time.
  void first_last_mean_loss
  (int first, int last, double *mean, double *loss){
    double s = get_sum(first, last);
    int N = last-first+1;
    *loss = -s*s/N;
    *mean = s/N;
  }
  /* Above we compute the optimal square loss for a segment with sum of
     data = s and number of data = N.

     If x_i is data point i, and s = sum_{i=1}^N x_i is the sum over N
     data points, then the optimal mean is s/n and the optimal square
     loss is

     sum_{i=1}^N (x_i - s/N)^2 =

     sum [x_i^2] - 2*(s/N)*s + N*(s/N)^2 =

     sum [x_i^2] - s^2 / N

     The first term (sum of squares of data) can be ignored during
     optimization, because it does not depend on the optimization
     variable (segment mean). It is added back after optimization, in
     the R code.
  */
  void first_last_mean_loss(int first, int last, MeanLoss *mc){
    first_last_mean_loss(first, last, &(mc->mean), &(mc->loss));
  }
};

// Split class stores info for a single candidate split to consider.
class Split {
public:
  int this_end;//index of last data point on the first/before segment.
  MeanLoss before, after;
  double set_mean_loss
  (SetCumsum &cumsums, int first, int end_i, int last){
    this_end = end_i;
    cumsums.first_last_mean_loss(first, end_i, &before);
    cumsums.first_last_mean_loss(end_i+1, last, &after);
    return before.loss + after.loss;
  }
};

// Segment class stores information about a segment that could be
// split.
class Segment {
public:
  //indices of first/last data points on this segment.
  int first, last;
  //index and after indicator of a previous segment whose mean
  //parameter is invalidated if this segment in included. For example
  //invalidates_index=2 and invalidates_after=0 implies before_mean
  //parameter at index 2 is invalidated; invalidates_index=1 and
  //invalidates_after=1 implies after_mean parameter at index 1 is
  //invalidated. It is necessary to store this information in order to
  //recover the optimal mean parameters of any model size.
  int invalidates_index, invalidates_after;
  double best_decrease;
  Split best_split;
  int n_changes() const {
    return last-first;
  }
  // Segments are kept sorted by best_decrease value, so that we can
  // find the best segment to split in constant O(1) time.
  friend bool operator<(const Segment& l, const Segment& r)
  {
    if(l.best_decrease == r.best_decrease){
      // if two segments are equally good to split in terms of the
      // loss, then to save time we should split the larger.
      return l.n_changes() > r.n_changes();
    }else{
      return l.best_decrease < r.best_decrease;
    }
  }
  // constructor which considers all possible splits from first to
  // last, and then stores the best split and loss decrease.
  Segment
  (SetCumsum &cumsums, int first, int last,
   int invalidates_after, int invalidates_index,
   double loss_no_split
   ): first(first), last(last),
      invalidates_index(invalidates_index),
      invalidates_after(invalidates_after)
  {
    Split candidate_split;
    int n_candidates = last-first;
    double best_loss_split = INFINITY, loss_split;
    // for loop over all possible splits on this Segment.
    for(int ci=0; ci<n_candidates; ci++){
      loss_split = candidate_split.set_mean_loss
	(cumsums, first, first+ci, last);
      if(loss_split < best_loss_split){
	best_loss_split = loss_split;
	best_split = candidate_split;
      }
    }
    best_decrease = best_loss_split - loss_no_split;
  }
};

class Candidates {
public:
  std::multiset<Segment> candidates;
  SetCumsum subtrain, validation;
  // computes the cumulative sum vectors in linear O(n_data) time.
  int init
  (const double *data_vec, const int n_data,
   const int *position_vec, const int *is_validation_vec
   ){
    int n_validation = 0;
    for(int data_i=0; data_i<n_data; data_i++){
      if(is_validation_vec[data_i]){
	n_validation++;
      }
    }
    subtrain.cumsum_vec.resize(n_validation);
    validation.cumsum_vec.resize(n_validation);
    int last_subtrain_i=-1;
    int n_subtrain = n_data - n_validation;
    double pos_total, pos_change, subtrain_total=0, validation_total=0;
    int read_start=0, write_index=0;
    for(int data_i=0; data_i<n_data; data_i++){
      bool is_subtrain = !is_validation_vec[data_i];
      bool write_subtrain = last_subtrain_i >= 0 && is_subtrain;
      bool write_end = data_i == n_data-1;
      if(write_subtrain || write_end){
	if(write_subtrain){
	  pos_total = position_vec[data_i]+position_vec[last_subtrain_i];
	  pos_change = pos_total/2;
	}else{
	  pos_change = position_vec[data_i]+1;
	}
	int read_index=read_start;
	while(read_index < n_data && position_vec[read_index] <= pos_change){
	  if(is_validation_vec[read_index]){
	    validation_total += data_vec[read_index];
	  }else{
	    subtrain_total += data_vec[read_index];
	  }
	  read_index++;
	}
	validation_cumsum[write_index] = validation_total;
	subtrain_cumsum[write_index] = subtrain_total;
	read_start = read_index;
	write_index++;
      }
      if(is_subtrain){
	last_subtrain_i = data_i;
      }
    }
    return n_subtrain;
  }
  // Add a new Segment to candidates if it is big enough to split.
  void maybe_add
  (int first, int last,
   int invalidates_after, int invalidates_index,
   double loss_no_split)
  {
    if(first < last){
      // if it is possible to split (more than one data point on this
      // segment) then insert new segment into the candidates set.
      candidates.emplace
	(subtrain, first, last,
	 invalidates_after, invalidates_index, loss_no_split);
    }
  }
};

/* Binary segmentation algorithm for change in mean in the normal
   distribution, loss function is square loss.

   This code assumes, and the code which calls this function needs to
   have error checking for, the following:

   At least one data points (0 < n_data), data_vec is an array
   of input data, size n_data.

   Positive number of segments (0 < max_segments), all of the other
   pointers are output arrays of size max_segments (need to be
   allocated by the code which calls this function).

   See coef.binseg_normal in R code for a procedure that uses these
   output arrays to efficiently compute the segment means for any
   model size.
 */
int binseg_normal
(const double *data_vec, const int n_data, const int max_segments,
 const int *is_validation_vec, const int *position_vec,
 int *seg_end, double *loss, double *validation_loss,
 double *before_mean, double *after_mean, 
 int *before_size, int *after_size, 
 int *invalidates_index, int *invalidates_after){
  if(n_data < max_segments){
    return ERROR_TOO_MANY_SEGMENTS;
  }
  Candidates V;
  // Begin by initializing cumulative sum vectors.
  int n_subtrain = V.init(data_vec, n_data, position_vec, is_validation_vec);
  // Then store the trivial segment mean/loss (which starts at the
  // first and ends at the last data point).
  V.subtrain.first_last_mean_loss
    (0, n_subtrain-1, before_mean, loss);
  before_size[0] = n_subtrain;
  seg_end[0] = n_subtrain-1;
  after_mean[0] = INFINITY;
  after_size[0] = -2; // unused/missing indicator.
  invalidates_index[0]=-2;
  invalidates_after[0]=-2;
  // Add a segment and split to the set of candidates.
  V.maybe_add(0, n_subtrain-1, 0, 0, loss[0]);
  // For loop over number of segments. During each iteration we find
  // the Segment/split which results in the best loss decrease, store
  // the resulting model parameters, and add new Segment/split
  // candidates if necessary.
  for(int seg_i=1; seg_i < max_segments; seg_i++){
    // The multiset is a red-black tree which is sorted in increasing
    // order (by best_decrease values), so the first element is always
    // the segment which results in the best loss decrease.
    std::multiset<Segment>::iterator it = V.candidates.begin();
    // Store loss and model parameters associated with this split.
    loss[seg_i] = loss[seg_i-1] + it->best_decrease;
    seg_end[seg_i] = it->best_split.this_end;
    before_mean[seg_i] = it->best_split.before.mean;
    after_mean[seg_i] = it->best_split.after.mean;
    // Also store invalidates index/after so we know which previous
    // model parameters are no longer used because of this split.
    invalidates_index[seg_i] = it->invalidates_index;
    invalidates_after[seg_i] = it->invalidates_after;
    // The sizes below are not strictly necessary to store (they can
    // be derived from start/end) but it makes it easier to analyze
    // the time complexity of the algorithm. Splits which result in
    // equal sizes before/after make the algorithm run fastest: first
    // split sizes N/2,N/2, second split sizes N/4,N/4, etc. Worst
    // case is when first split sizes 1,N-1 second split sizes 1,N-2,
    // etc.
    before_size[seg_i] = it->best_split.this_end - it->first + 1;
    after_size[seg_i]  = it->last - it->best_split.this_end;
    // Finally add new split candidates if necessary.
    V.maybe_add
      (it->first, it->best_split.this_end,
       0,//invalidates_after=0 => before_mean invalidated.
       seg_i, it->best_split.before.loss);
    V.maybe_add
      (it->best_split.this_end+1, it->last,
       1,//invalidates_after=1 => after_mean invalidated.
       seg_i, it->best_split.after.loss);
    // Erase at end because we need it->values during maybe_add
    // inserts above.
    V.candidates.erase(it);
  }
  return 0;//SUCCESS.
}

