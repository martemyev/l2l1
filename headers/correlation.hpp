#ifndef CORRELATION_HPP
#define CORRELATION_HPP

#include <vector>
#include <cmath>


/**
 *
 * Cross correlation between each trace of the two datasets.
 *
 * @param data0[in] First dataset
 * @param data1[in] Second dataset
 * @param row_beg[in] Starting row in the datasets
 * @param row_end[in] Ending row (not including) in the datasets
 * @param col_beg[in] Starting trace in the datasets
 * @param col_end[in] Ending trace (not including) in the datasets
 * @param lag[in] Lag for the second dataset
 * @param xcorrelation[out] Vector of cross correlation values for each trace
 */
void x_correlation_by_traces(float **data0,
                             float **data1,
                             int row_beg,
                             int row_end,
                             int col_beg,
                             int col_end,
                             int lag,
                             std::vector<double> &xcorrelation)
{
  // average and standard deviation for every time log, i.e. every trace,
  // of each dataset
  std::vector<double> mu[2];
  std::vector<double> sigma[2];
  for (int i = 0; i < 2; ++i)
  {
    mu[i].resize(col_end - col_beg, 0.);
    sigma[i].resize(col_end - col_beg, 0.);
  }

  // compute an average and sigma for every time series of every dataset
  for (int j = col_beg; j < col_end; ++j)
  {
    double part0 = 0.;
    double part1 = 0.;
    for (int i = row_beg; i < row_end; ++i)
    {
      const double d0 = data0[i][j];
      const double d1 = data1[i][j];

      mu[0][j - col_beg] += d0;
      mu[1][j - col_beg] += d1;
      part0 += d0 * d0;
      part1 += d1 * d1;
    }
    mu[0][j - col_beg] /= row_end - row_beg;
    mu[1][j - col_beg] /= row_end - row_beg;
    part0 /= row_end - row_beg;
    part1 /= row_end - row_beg;

    sigma[0][j - col_beg] = sqrt(part0 - pow(mu[0][j-col_beg],2));
    sigma[1][j - col_beg] = sqrt(part1 - pow(mu[1][j-col_beg],2));
  }

  // compute the normalized cross correlation
  xcorrelation.resize(col_end - col_beg, 0.);
  for (int j = col_beg; j < col_end; ++j)
  {
    double mu0 = mu[0][j - col_beg];
    double mu1 = mu[1][j - col_beg];
    double sum = 0.;
    for (int i = row_beg; i < row_end; ++i)
    {
      const double d0 = data0[i][j]; // we take the data from the first dataset
                                     // as is
      double d1 = 0.; // for the second dataset we shift the sample index
      if (i + lag < row_end && i + lag >= row_beg) // according to the lag value
        d1 = data1[i+lag][j]; // and pad the other values with the zero

      sum += (d0 - mu0) * (d1 - mu1);
    }

    sum /= row_end - row_beg;

    xcorrelation[j-col_beg] = sum / (sigma[0][j-col_beg]*sigma[1][j-col_beg]);
  }
}




/**
 *
 * Cross correlation between all traces of the two datasets as a whole.
 *
 * @param data0[in] First dataset
 * @param data1[in] Second dataset
 * @param row_beg[in] Starting row in the datasets
 * @param row_end[in] Ending row (not including) in the datasets
 * @param col_beg[in] Starting trace in the datasets
 * @param col_end[in] Ending trace (not including) in the datasets
 * @param lag[in] Lag for the second dataset
 * @param xcorrelation[out] Vector of cross correlation values for each trace
 */
void x_correlation_whole(float **data0,
                         float **data1,
                         int row_beg,
                         int row_end,
                         int col_beg,
                         int col_end,
                         int lag,
                         double &xcorrelation)
{
  // compute the average and the deviation for the whole datasets
  double mu0 = 0.;
  double mu1 = 0.;
  double part0 = 0.;
  double part1 = 0.;
  for (int i = row_beg; i < row_end; ++i)
  {
    for (int j = col_beg; j < col_end; ++j)
    {
      const double d0 = data0[i][j];
      const double d1 = data1[i][j];
      mu0 += d0;
      mu1 += d1;
      part0 += d0 * d0;
      part1 += d1 * d1;
    }
  }
  mu0 /= (row_end - row_beg) * (col_end - col_beg);
  mu1 /= (row_end - row_beg) * (col_end - col_beg);
  part0 /= (row_end - row_beg) * (col_end - col_beg);
  part1 /= (row_end - row_beg) * (col_end - col_beg);

  double sigma0 = sqrt(part0 - mu0*mu0);
  double sigma1 = sqrt(part1 - mu1*mu1);

  // compute the normalized cross correlation
  double sum = 0.;
  for (int i = row_beg; i < row_end; ++i)
  {
    for (int j = col_beg; j < col_end; ++j)
    {
      const double d0 = data0[i][j]; // we take the data from the first dataset
                                     // as is
      double d1 = 0.; // for the second dataset we shift the sample index
      if (i + lag < row_end && i + lag >= row_beg) // according to the lag value
        d1 = data1[i+lag][j]; // and pad the other values with the zero

      sum += (d0 - mu0) * (d1 - mu1);
    }

  }
  sum /= (row_end - row_beg) * (col_end - col_beg);

  xcorrelation = sum / (sigma0 * sigma1);
}

#endif // CORRELATION_HPP
