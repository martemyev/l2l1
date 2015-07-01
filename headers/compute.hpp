#ifndef COMPUTE_HPP
#define COMPUTE_HPP

class Parameters;



class Compute
{
public:

  Compute(Parameters &param);

  ~Compute();


  void run();


protected:

  Parameters &_param;

  float **_data0;
  float **_data1;

  int _n_rows; ///< number of rows in the input files = number of samples


  void read();
  void l2l1() const;
  void diff_file() const;
  void scale() const;
  void shift() const;
  void compute_xcorrelation() const;
  void compute_rms() const;


  Compute(const Compute&);
  Compute& operator =(const Compute&);
};



#endif // COMPUTE_HPP
