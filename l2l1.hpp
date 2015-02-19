#ifndef L2L1_HPP
#define L2L1_HPP

#include "parameters.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>



void l2l1(Parameters &param)
{
  std::ifstream in0(param._file_0.c_str(), std::ios::binary);
  if (!in0)
  {
    std::cerr << "File '" << param._file_0 << "' can't be opened. Check that "
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
  int n_rows = n_numbers / param._n_cols;
  if (n_rows < 1)
  {
    std::cerr << "The number of rows should be positive: " << n_rows << "\n";
    exit(1);
  }

  // allocate and read the data from the given files
  float **data0 = new float*[n_rows];
  for (int i = 0; i < n_rows; ++i)
  {
    data0[i] = new float[param._n_cols];
    for (int j = 0; j < param._n_cols; ++j)
      in0.read((char*)&data0[i][j], sizeof(float));
  }

  in0.close();



  std::ifstream in1(param._file_1.c_str(), std::ios::binary);
  if (!in1)
  {
    std::cerr << "File '" << param._file_1 << "' can't be opened. Check that "
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
  float **data1 = new float*[n_rows];
  for (int i = 0; i < n_rows; ++i)
  {
    data1[i] = new float[param._n_cols];
    for (int j = 0; j < param._n_cols; ++j)
      in1.read((char*)&data1[i][j], sizeof(float));
  }

  in1.close();



  // comparison
  if (param._row_end < 0) param._row_end = n_rows;

  float l2_0 = 0, l1_0 = 0;
  float l2_1 = 0, l1_1 = 0;
  float l2_diff = 0, l1_diff = 0;
  for (int i = param._row_beg; i < param._row_end; ++i)
  {
    for (int j = param._col_beg; j < param._col_end; ++j)
    {
      l2_0 += data0[i][j] * data0[i][j];
      l2_1 += data1[i][j] * data1[i][j];
      l2_diff += (data0[i][j] - data1[i][j]) * (data0[i][j] - data1[i][j]);

      l1_0 += fabs(data0[i][j]);
      l1_1 += fabs(data1[i][j]);
      l1_diff += fabs(data0[i][j] - data1[i][j]);
    }
  }

  l2_0 = sqrt(l2_0);
  l2_1 = sqrt(l2_1);
  l2_diff = sqrt(l2_diff);

  const double l2_diff_rel = l2_diff / l2_0;
  const double l1_diff_rel = l1_diff / l1_0;

  if (param._verbose > 1)
  {
    std::cout << "\nn_rows      = " << n_rows;
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
  else if (param._verbose > 0)
  {
    std::cout << "\nL2_diff_rel = " << l2_diff_rel * 100 << " %";
    std::cout << "\nL1_diff_rel = " << l1_diff_rel * 100 << " %\n";
  }
  else
  {
    std::cout << l2_diff_rel * 100 << " " << l1_diff_rel * 100 << "\n";
  }

  if (!param._diff_file.empty() &&
      param._diff_file != Parameters::DEFAULT_FILE_NAME)
  {
    std::ofstream out(param._diff_file.c_str(), std::ios::binary);
    if (!out)
    {
      std::cerr << "File '" << param._diff_file << "' can't be opened for "
                   "writing.\n";
      exit(1);
    }

    for (int i = param._row_beg; i < param._row_end; ++i)
    {
      for (int j = param._col_beg; j < param._col_end; ++j)
      {
        float val = data0[i][j] - data1[i][j];
        out.write(reinterpret_cast<char*>(&val), sizeof(val));
      }
    }

    out.close();
  }




  for (int i = 0; i < n_rows; ++i)
  {
    delete[] data1[i];
    delete[] data0[i];
  }
  delete[] data1;
  delete[] data0;
}

#endif // L2L1_HPP
