#ifndef RMS_H
#define RMS_H



void compute_rms_diff_files(float **data0,
                            float **data1,
                            int row_beg,
                            int row_end,
                            int col_beg,
                            int col_end,
                            std::vector<double> &RMS_0,
                            std::vector<double> &RMS_1)
{
  RMS_0.clear();
  RMS_0.resize(col_end - col_beg, 0.0);
  RMS_1.clear();
  RMS_1.resize(col_end - col_beg, 0.0);

  for (int i = row_beg; i < row_end; ++i)
  {
    for (int j = col_beg; j < col_end; ++j)
    {
      const double d0 = data0[i][j];
      const double d1 = data1[i][j];

      RMS_0[j - col_beg] += d0*d0;
      RMS_1[j - col_beg] += d1*d1;
    }
  }

  for (size_t i = 0; i < RMS_0.size(); ++i)
  {
    RMS_0[i] /= row_end - row_beg;
    RMS_1[i] /= row_end - row_beg;

    RMS_0[i] = sqrt(RMS_0[i]);
    RMS_1[i] = sqrt(RMS_1[i]);
  }
}




void compute_rms_amplitude(float **data0,
                           float **data1,
                           int row_beg,
                           int row_end,
                           int col_beg,
                           int col_end,
                           std::vector<double> &RMS)
{
  RMS.clear();
  RMS.resize(col_end - col_beg, 0.0);
  
  for (int i = row_beg; i < row_end; ++i)
  {
    for (int j = col_beg; j < col_end; ++j)
    {
      const double d0 = data0[i][j];
      const double d1 = data1[i][j];

      RMS[j - col_beg] += (d0*d0 + d1*d1);
    }
  }

  for (size_t i = 0; i < RMS.size(); ++i)
  {
    RMS[i] /= row_end - row_beg;
    RMS[i] = sqrt(RMS[i]);
  }
}



#endif // RMS_H
