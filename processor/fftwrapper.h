#ifndef FFTWRAPPER_H_
#define FFTWRAPPER_H_

#include <complex>

// XXX this assumes that std::complex<float> is bit-compatible with
// fftwf_complex, which SHOULD be true...
#include <fftw3.h>

class RealFft {
 public:
  typedef std::complex<float> Complex;

  RealFft() : inited_(false), size_(0), data_(0) {}
  explicit RealFft(size_t sz) : inited_(false), data_(0) { setSize(sz); }

  bool isInited() const { return inited_; }
  size_t getSize() const { return size_; }
  const float* getBuffer() const { return data_; }
  float* getBuffer() { return data_; }
  const Complex* getOutput() const { return out_; }

  void setSize (size_t sz) {
    done();
    data_ = ((sz != 0) ? (new float[sz]) : 0);
    size_ = sz;
    inited_ = false;
  }

  bool init() {
    done();
    if (size_ == 0)
      return false;

    out_ = (Complex*)fftwf_malloc (sizeof (Complex)*(size_ / 2 + 1));
    plan_ = fftwf_plan_dft_r2c_1d (size_, data_, (float (*)[2])out_,
      FFTW_MEASURE);
    inited_ = true;
    return true;
  }
  
  bool done() {
    if (!inited_)
      return true;

    fftwf_destroy_plan(plan_);
    fftwf_free(out_);
    if (data_)
      delete[] data_;

    inited_ = false;
    return true;
  }

  void exec() {
    if (!inited_)
      if (!init())
        return;
    fftwf_execute(plan_);
  }

  ~RealFft() { done(); }
 
 protected:
  bool		inited_;
  size_t	size_;
  float*	data_;
  Complex*	out_;
  fftwf_plan	plan_;
};

#endif
