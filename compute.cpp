#include "compute.hpp"
#include "parameters.hpp"
#include "correlation.hpp"
#include "utilities.hpp"
#include "rms.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <algorithm>




Compute::Compute(Parameters &param)
  : _param(param),
    _data0(nullptr),
    _data1(nullptr),
    _n_rows(0)
{ }




Compute::~Compute()
{
  for (int i = 0; i < _n_rows; ++i)
  {
    delete[] _data1[i];
    delete[] _data0[i];
  }
  delete[] _data1;
  delete[] _data0;
}



void Compute::run()
{
  read();

  if (_param._l2l1)
    l2l1();

  if (!_param._diff_file.empty() && _param._diff_file != DEFAULT_FILE_NAME)
    diff_file();

  if (_param._scale_file_1)
    scale();

  if (_param._shift_file_1)
    shift();

  if (_param._cross_correlation != 0)
    compute_xcorrelation();

  if (_param._rms != 0)
    compute_rms();
}




void Compute::read()
{
  //----------------------------------------------------------------------------
  // file0
  //----------------------------------------------------------------------------
  std::ifstream in0(_param._file_0.c_str(), std::ios::binary);
  if (!in0)
  {
    std::cerr << "File '" << _param._file_0 << "' can't be opened. Check that "
                 "it exists.\n";
    exit(1);
  }
  in0.seekg(0, in0.end);
  int length0 = in0.tellg(); // total length of the file0 in bytes
  in0.seekg(0, in0.beg);

  // since we know that there are only float numbers in single precision, we get
  // the total number of numbers in the file
  int n_numbers = length0 / sizeof(float);

  // and we also know the number of rows in the matrix
  _n_rows = n_numbers / _param._n_cols;
  if (_n_rows < 1)
  {
    std::cerr << "The number of rows should be positive: " << _n_rows << "\n";
    exit(1);
  }

  // allocate and read the data from the given files
  _data0 = new float*[_n_rows];
  for (int i = 0; i < _n_rows; ++i)
  {
    _data0[i] = new float[_param._n_cols];
    for (int j = 0; j < _param._n_cols; ++j)
      in0.read((char*)&_data0[i][j], sizeof(float));
  }

  in0.close();

  //----------------------------------------------------------------------------
  // file1
  //----------------------------------------------------------------------------
  std::ifstream in1(_param._file_1.c_str(), std::ios::binary);
  if (!in1)
  {
    std::cerr << "File '" << _param._file_1 << "' can't be opened. Check that "
                 "it exists.\n";
    exit(1);
  }

  in1.seekg(0, in1.end);
  int length1 = in1.tellg(); // total length of the file1 in bytes
  in1.seekg(0, in1.beg);

  if (length0 != length1)
  {
    std::cerr << "The given files have different length!\n";
    in1.close();
    exit(1);
  }

  // allocate and read the data from the given files
  _data1 = new float*[_n_rows];
  for (int i = 0; i < _n_rows; ++i)
  {
    _data1[i] = new float[_param._n_cols];
    for (int j = 0; j < _param._n_cols; ++j)
      in1.read((char*)&_data1[i][j], sizeof(float));
  }

  in1.close();

  //----------------------------------------------------------------------------
  // adjust _row_end in the parameters
  //----------------------------------------------------------------------------
  if (_param._row_end < 0) _param._row_end = _n_rows;
}




void Compute::l2l1() const
{
  float l2_0 = 0, l1_0 = 0;
  float l2_1 = 0, l1_1 = 0;
  float l2_diff = 0, l1_diff = 0;
  for (int i = _param._row_beg; i < _param._row_end; ++i)
  {
    for (int j = _param._col_beg; j < _param._col_end; ++j)
    {
      const float d0  = _data0[i][j];
      const float d1  = _data1[i][j];
      const float d01 = d0 - d1;

      l2_0 += d0 * d0;
      l2_1 += d1 * d1;
      l2_diff += d01 * d01;

      l1_0 += fabs(d0);
      l1_1 += fabs(d1);
      l1_diff += fabs(d01);
    }
  }

  l2_0 = sqrt(l2_0);
  l2_1 = sqrt(l2_1);
  l2_diff = sqrt(l2_diff);

  const double l2_diff_rel = l2_diff / l2_0;
  const double l1_diff_rel = l1_diff / l1_0;

  if (_param._verbose > 1)
  {
    std::cout << "\nn_rows      = " << _n_rows;
    std::cout << "\nL2_0        = " << l2_0;
    std::cout << "\nL2_1        = " << l2_1;
    std::cout << "\nL2_diff_abs = " << l2_diff;
    std::cout << "\nL2_diff_rel = " << l2_diff_rel
              << " = " << l2_diff_rel * 100 << " %";
    std::cout << "\nL1_0        = " << l1_0;
    std::cout << "\nL1_1        = " << l1_1;
    std::cout << "\nL1_diff_abs = " << l1_diff;
    std::cout << "\nL1_diff_rel = " << l1_diff_rel
              << " = " << l1_diff_rel * 100 << " %\n";
  }
  else if (_param._verbose > 0)
  {
    std::cout << "\nL2_diff_rel = " << l2_diff_rel * 100 << " %";
    std::cout << "\nL1_diff_rel = " << l1_diff_rel * 100 << " %\n";
  }
  else
  {
    std::cout << l2_diff_rel * 100 << " " << l1_diff_rel * 100 << "\n";
  }
}



void Compute::diff_file() const
{
  if (_param._verbose > 1)
    std::cout << "Make a file of difference: " << _param._diff_file
              << std::endl;

  std::ofstream out(_param._diff_file.c_str(), std::ios::binary);
  if (!out)
  {
    std::cerr << "File '" << _param._diff_file << "' can't be opened for "
                 "writing.\n";
    exit(1);
  }

  for (int i = _param._row_beg; i < _param._row_end; ++i)
  {
    for (int j = _param._col_beg; j < _param._col_end; ++j)
    {
      float val = _data0[i][j] - _data1[i][j];
      out.write(reinterpret_cast<char*>(&val), sizeof(val));
    }
  }

  out.close();
}




void Compute::scale() const
{
  if (_param._verbose > 1)
    std::cout << "Make a scaled file 1\n";

  // find the absolute max values of the two datasets
  float max_value0 = fabs(_data0[_param._row_beg][_param._col_beg]);
  float max_value1 = fabs(_data1[_param._row_beg][_param._col_beg]);
  for (int i = _param._row_beg + 1; i < _param._row_end; ++i)
  {
    for (int j = _param._col_beg + 1; j < _param._col_end; ++j)
    {
      if (fabs(_data0[i][j]) > max_value0) max_value0 = fabs(_data0[i][j]);
      if (fabs(_data1[i][j]) > max_value1) max_value1 = fabs(_data1[i][j]);
    }
  }

  const float ratio = max_value0 / max_value1;

  if (_param._verbose > 1)
    std::cout << "  max_value0 = " << max_value0 << "\n"
              << "  max_value1 = " << max_value1 << "\n"
              << "  ratio      = " << ratio << std::endl;

  // now create a new file with scaled data from the file 1
  const std::string scaled_file_1 = file_path(_param._file_1) +
                                    file_stem(_param._file_1) +
                                    "_scaled.bin";
  std::ofstream out(scaled_file_1.c_str(), std::ios::binary);
  if (!out)
  {
    std::cerr << "File '" << scaled_file_1 << "' can't be opened for "
                 "writing.\n";
    exit(1);
  }
  for (int i = _param._row_beg; i < _param._row_end; ++i)
  {
    for (int j = _param._col_beg; j < _param._col_end; ++j)
    {
      float val = ratio * _data1[i][j];
      out.write(reinterpret_cast<char*>(&val), sizeof(val));
    }
  }
  out.close();
}




void Compute::shift() const
{
  if (_param._verbose > 1)
    std::cout << "Make a shifted file 1\n";

  // find the absolute max values of the two datasets and the corresponding
  // time (time step number)
  float max_value0 = fabs(_data0[_param._row_beg][_param._col_beg]);
  float max_value1 = fabs(_data1[_param._row_beg][_param._col_beg]);
  int timestep0 = _param._row_beg; // time step corresponding to max_value0
  int timestep1 = _param._row_beg; // time step corresponding to max_value1

  for (int i = _param._row_beg + 1; i < _param._row_end; ++i)
  {
    for (int j = _param._col_beg + 1; j < _param._col_end; ++j)
    {
      if (fabs(_data0[i][j]) > max_value0)
      {
        max_value0 = fabs(_data0[i][j]);
        timestep0  = i;
      }
      if (fabs(_data1[i][j]) > max_value1)
      {
        max_value1 = fabs(_data1[i][j]);
        timestep1  = i;
      }
    }
  }

  // the shift is defined in terms of time steps
  const int shift_step = timestep0 - timestep1;

  if (_param._verbose > 1)
    std::cout << "  shift in timesteps = " << shift_step << std::endl;

  // now create a new file with shifted data from the file 1
  const std::string shifted_file_1 = file_path(_param._file_1) +
                                     file_stem(_param._file_1) +
                                     "_shifted.bin";
  std::ofstream out(shifted_file_1.c_str(), std::ios::binary);
  if (!out)
  {
    std::cerr << "File '" << shifted_file_1 << "' can't be opened for "
                 "writing.\n";
    exit(1);
  }
  for (int i = _param._row_beg; i < _param._row_end; ++i)
  {
    const int tmp = std::max(i - shift_step, _param._row_beg);
    const int tstep = std::min(tmp, _param._row_end-1);
    for (int j = _param._col_beg; j < _param._col_end; ++j)
    {
      float val = _data1[tstep][j];
      out.write(reinterpret_cast<char*>(&val), sizeof(val));
    }
  }
  out.close();
}




void Compute::compute_xcorrelation() const
{
  if (_param._verbose > 0) std::cout << "Cross correlation:\n";

  if (_param._cross_correlation == 1)
  {
    for (int lag = -_param._lag_region; lag <= _param._lag_region; ++lag)
    {
      std::vector<double> xcorrelation;
      x_correlation_by_traces(_data0, _data1,
                              _param._row_beg, _param._row_end,
                              _param._col_beg, _param._col_end,
                              lag, xcorrelation);

      const double minXCor = *std::min_element(xcorrelation.begin(), xcorrelation.end());
      const double maxXCor = *std::max_element(xcorrelation.begin(), xcorrelation.end());

      if (_param._verbose > 0)
        std::cout << "  lag = " << lag
                  << " min = " << minXCor
                  << " max = " << maxXCor << std::endl;
      else // with no verbosity we just print the numbers
        std::cout << minXCor << " " << maxXCor << std::endl;
    }
  }
  else if (_param._cross_correlation == 2)
  {
    for (int lag = -_param._lag_region; lag <= _param._lag_region; ++lag)
    {
      double xcorrelation;
      x_correlation_whole(_data0, _data1,
                          _param._row_beg, _param._row_end,
                          _param._col_beg, _param._col_end,
                          lag, xcorrelation);

      if (_param._verbose > 0)
        std::cout << "  lag = " << lag
                  << " value = " << xcorrelation << std::endl;
      else
        std::cout << xcorrelation << std::endl;
    }
  }
  else require(false, "Unknown xcorrelation option");
}




void Compute::compute_rms() const
{
  if (_param._verbose > 0)
    std::cout << "RMS computation" << std::endl;

  if (_param._rms == 1)
  {
    std::vector<double> RMS_0, RMS_1;
    compute_rms_diff_files(_data0, _data1,
                           _param._row_beg, _param._row_end,
                           _param._col_beg, _param._col_end,
                           RMS_0, RMS_1);
    std::cout << "RMS_0:\n";
    for (size_t i = 0; i < RMS_0.size(); ++i)
      std::cout << RMS_0[i] << " ";
    std::cout << "\nRMS_1:\n";
    for (size_t i = 0; i < RMS_1.size(); ++i)
      std::cout << RMS_1[i] << " ";
  }
  else if (_param._rms == 2)
  {
    std::vector<double> RMS;
    compute_rms_amplitude(_data0, _data1,
                          _param._row_beg, _param._row_end,
                          _param._col_beg, _param._col_end,
                          RMS);
    std::cout << "\nRMS:\n";
    for (size_t i = 0; i < RMS.size(); ++i)
      std::cout << RMS[i] << " ";
  }
  else require(false, "Unknown rms option");
}
