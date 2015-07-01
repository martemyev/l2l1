#include "parameters.hpp"
#include "utilities.hpp"

#include <cstring>
#include <algorithm>



//==============================================================================
//
// Parameters class
//
//==============================================================================




Parameters::Parameters(int argc, char **argv)
  : _file_0(DEFAULT_FILE_NAME),
    _file_1(DEFAULT_FILE_NAME),
    _n_cols(0),
    _col_beg(0),
    _col_end(-1),
    _row_beg(0),
    _row_end(-1),
    _verbose(2),
    _l2l1(0),
    _diff_file(DEFAULT_FILE_NAME),
    _scale_file_1(0),
    _scale_factor(0.0),
    _shift_file_1(false),
    _cross_correlation(0),
    _lag_region(0),
    _rms(0),
    _parameters(),
    _longest_string_key_len(DEFAULT_PRINT_LEN),
    _longest_string_value_len(DEFAULT_PRINT_LEN)
{
  int p = 0;

  _parameters["-f0"]    = ParamBasePtr(new OneParam<std::string>("file name (reference solution or Ux)", &_file_0, ++p));
  _parameters["-f1"]    = ParamBasePtr(new OneParam<std::string>("file name (solution to compare or Uz)", &_file_1, ++p));
  _parameters["-ncols"] = ParamBasePtr(new OneParam<int>("number of columns in the files", &_n_cols, ++p));
  _parameters["-c0"]    = ParamBasePtr(new OneParam<int>("first column for comparison", &_col_beg, ++p));
  _parameters["-c1"]    = ParamBasePtr(new OneParam<int>("last column for comparison (not including)", &_col_end, ++p));
  _parameters["-r0"]    = ParamBasePtr(new OneParam<int>("first row for comparison", &_row_beg, ++p));
  _parameters["-r1"]    = ParamBasePtr(new OneParam<int>("last row for comparison (not including)", &_row_end, ++p));
  _parameters["-v"]     = ParamBasePtr(new OneParam<int>("verbosity level", &_verbose, ++p));
  _parameters["-l2l1"]  = ParamBasePtr(new OneParam<int>("compute L2 and L1 norms of difference", &_l2l1, ++p));
  _parameters["-df"]    = ParamBasePtr(new OneParam<std::string>("name of file with difference", &_diff_file, ++p));
  _parameters["-sc1"]   = ParamBasePtr(new OneParam<int>("scale data 1 with respect to data 0 (1) or to scale factor (2)", &_scale_file_1, ++p));
  _parameters["-sf"]    = ParamBasePtr(new OneParam<double>("scale factor for data 1", &_scale_factor, ++p));
  _parameters["-sh1"]   = ParamBasePtr(new OneParam<bool>("shift data 1 with respect to data 0", &_shift_file_1, ++p));
  _parameters["-xcor"]  = ParamBasePtr(new OneParam<int>("compute cross correlation", &_cross_correlation, ++p));
  _parameters["-lag"]   = ParamBasePtr(new OneParam<int>("lag region for cross correlation computation", &_lag_region, ++p));
  _parameters["-rms"]   = ParamBasePtr(new OneParam<int>("compute RMS of traces", &_rms, ++p));

  update_longest_string_key_len();

  if (argc == 1 || argcheck(argc, argv, "-help") || argcheck(argc, argv, "-h"))
  {
    print_options();
    exit(0);
  }

  read_command_line(argc, argv);

  if (_col_end < 0) _col_end = _n_cols;

  update_longest_string_value_len();
}




void Parameters::read_command_line(int argc, char **argv)
{
  // all the command line entries
  std::vector<std::string> arguments(argv, argv+argc);

  // starting from the 1-st (not 0-th, because arguments[0] is the path to the
  // executable file of this program). Then we consider every second parameter,
  // since the command line goes like this: param0 value0 param1 value1 ...
  // and we need to consider only parameters.
  require((argc-1) % 2 == 0, "The number of command line arguments must be even"
          ", because every parameter is accompanied by a value. But there are "
          "only " + d2s(argc-1) + " of the arguments");
  for (size_t ar = 1; ar < arguments.size(); ar += 2)
  {
    ParaMap::const_iterator iter = _parameters.find(arguments[ar]);
    require(iter != _parameters.end(), "Command line argument '" + arguments[ar]
            + "' wasn't found");
    require(ar+1 < arguments.size(), "Command line argument '" + arguments[ar]
            + "' doesn't have any value");
    iter->second->read(arguments[ar+1]);
  }
}




std::ostream& Parameters::print_options(std::ostream &out) const
{
  update_longest_string_key_len();

  out << "\nAvailable options [default values in brackets]\n\n";

  typedef std::vector<std::pair<std::string, ParamBasePtr> > ParaVec;

  // sort the map of parameters according to their priority values
  ParaVec sorted_parameters(_parameters.begin(), _parameters.end());
  std::sort(sorted_parameters.begin(),
            sorted_parameters.end(),
            compare_by_parameter_priority);

  ParaVec::const_iterator iter = sorted_parameters.begin();

  for (; iter != sorted_parameters.end(); ++iter)
  {
    const ParamBase *par = iter->second.get();

    std::cout << add_space(iter->first, _longest_string_key_len + SPACE_BETWEEN)
              << par->get_description()
              << " [" << par->str() << "]\n";
  }

  return out;
}




std::ostream& Parameters::print_parameters(std::ostream &out) const
{
  update_longest_string_key_len();
  update_longest_string_value_len();

  typedef std::vector<std::pair<std::string, ParamBasePtr> > ParaVec;

  // sort the map of parameters according to their priority values
  ParaVec sorted_parameters(_parameters.begin(), _parameters.end());
  std::sort(sorted_parameters.begin(),
            sorted_parameters.end(),
            compare_by_parameter_priority);

  ParaVec::const_iterator iter = sorted_parameters.begin();

  for (; iter != sorted_parameters.end(); ++iter)
  {
    const ParamBase *par = iter->second.get();
    std::cout << add_space(iter->first, _longest_string_key_len + SPACE_BETWEEN)
              << add_space(par->str(), _longest_string_value_len + SPACE_BETWEEN)
              << par->get_description() << "\n";
  }
  std::cout << "\n";

  return out;
}




void Parameters::check_parameters() const
{
  if (_file_0.empty() || _file_0 == DEFAULT_FILE_NAME)
  {
    std::cerr << "\nFile0 with reference solution is empty or not defined\n\n";
    exit(1);
  }
  if (_file_1.empty() || _file_1 == DEFAULT_FILE_NAME)
  {
    std::cerr << "\nFile1 with solution for comparison is empty or not defined\n\n";
    exit(1);
  }
  if (_n_cols <= 0)
  {
    std::cerr << "\nNumber of columns of the data is wrong: " << _n_cols << "\n\n";
    exit(1);
  }
  if (_col_end > _n_cols)
  {
    std::cerr << "\nLast column for comparison (" << _col_end << ") is out of "
                 "range (0, " << _n_cols << "]\n\n";
    exit(1);
  }
  if (_col_beg < 0)
  {
    std::cerr << "\nFirst column for comparison (" << _col_beg << ") must be >= 0\n\n";
    exit(1);
  }
  if (_col_beg >= _col_end)
  {
    std::cerr << "\nFirst column for comparison (" << _col_beg << ") must be less"
                 " than the last column for comparison (" << _col_end << ")\n\n";
    exit(1);
  }
  if (_row_beg < 0)
  {
    std::cerr << "\nFirst row for comparison (" << _row_beg << ") must be >= 0\n\n";
    exit(1);
  }
  if (_row_end > 0 && _row_beg >= _row_end)
  {
    std::cerr << "\nFirst row for comparison (" << _row_beg << ") must be less"
                 " than the last row for comparison (" << _row_end << ")\n\n";
    exit(1);
  }

  require(_l2l1 == 0 || _l2l1 == 1, "Unexpected value of -l2l1");

  if (_cross_correlation != 0 && _cross_correlation != 1 && _cross_correlation != 2)
  {
    std::cerr << "The parameter for computation of the cross correlation has "
                 "invalid value: " << _cross_correlation << ". The valid "
                 "options are: 0, 1, 2 (see the parameters.hpp for details)\n\n";
    exit(1);
  }
  if (_lag_region < 0)
  {
    std::cerr << "The lag region parameter (" << _lag_region << ") should be "
                 " >= 0\n\n";
    exit(1);
  }

  require(_rms == 0 || _rms == 1 || _rms == 2, "Unexpected value of -rms");
}




void Parameters::update_longest_string_key_len() const
{
  _longest_string_key_len = 0;

  ParaMap::const_iterator iter = _parameters.begin();

  for (; iter != _parameters.end(); ++iter)
  {
    const int len_key_string = iter->first.size();

    if (len_key_string > _longest_string_key_len)
      _longest_string_key_len = len_key_string;
  }
}




void Parameters::update_longest_string_value_len() const
{
  _longest_string_value_len = 0;

  ParaMap::const_iterator iter = _parameters.begin();

  for (; iter != _parameters.end(); ++iter)
  {
    const ParamBase *par = iter->second.get();
    const int len_value_string = par->str().size();

    if (len_value_string > _longest_string_value_len)
      _longest_string_value_len = len_value_string;
  }
}




//==============================================================================
//
// Compare two pairs containing info about parameters by the priority of the
// parameters
//
//==============================================================================
bool compare_by_parameter_priority(const std::pair<std::string, ParamBasePtr> &a,
                                   const std::pair<std::string, ParamBasePtr> &b)
{
  return (a.second->get_priority() < b.second->get_priority());
}


