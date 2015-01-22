#include "parameters.hpp"
#include <cstring>
#include <algorithm>



//==============================================================================
//
// Parameters class
//
//==============================================================================

std::string Parameters::DEFAULT_FILE_NAME = "no-file";
int Parameters::DEFAULT_PRINT_LEN = 10;




Parameters::Parameters(int argc, char **argv)
  : _file_0(DEFAULT_FILE_NAME),
    _file_1(DEFAULT_FILE_NAME),
    _n_cols(0),
    _col_beg(0),
    _col_end(-1),
    _row_beg(0),
    _row_end(-1),
    _verbose(2),
    _longest_string_key_len(DEFAULT_PRINT_LEN),
    _longest_string_value_len(DEFAULT_PRINT_LEN)
{
  _parameters["-f0"]    = std::unique_ptr<ParamBase>(new OneParam<std::string>("file name (reference solution)", &_file_0));
  _parameters["-f1"]    = std::unique_ptr<ParamBase>(new OneParam<std::string>("file name (solution to compare)", &_file_1));
  _parameters["-ncols"] = std::unique_ptr<ParamBase>(new OneParam<int>("number of columns in the files", &_n_cols));
  _parameters["-c0"]    = std::unique_ptr<ParamBase>(new OneParam<int>("first column for comparison", &_col_beg));
  _parameters["-c1"]    = std::unique_ptr<ParamBase>(new OneParam<int>("last column for comparison (not including)", &_col_end));
  _parameters["-r0"]    = std::unique_ptr<ParamBase>(new OneParam<int>("first row for comparison", &_row_beg));
  _parameters["-r1"]    = std::unique_ptr<ParamBase>(new OneParam<int>("last row for comparison (not including)", &_row_end));
  _parameters["-v"]     = std::unique_ptr<ParamBase>(new OneParam<int>("verbosity level", &_verbose));

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
  for (size_t ar = 1; ar < arguments.size(); ar += 2)
  {
    std::map<std::string, std::unique_ptr<ParamBase> >::const_iterator iter = _parameters.find(arguments[ar]);
    if (iter == _parameters.end())
    {
      std::cerr << "\nCommand line argument '" << arguments[ar]
                << "' wasn't found\n\n";
      exit(1);
    }
    if (ar+1 >= arguments.size())
    {
      std::cerr << "\nCommand line argument '" << arguments[ar] << "' doesn't "
                   "have any value\n\n";
      exit(1);
    }
    iter->second->read(arguments[ar+1]);
  }
}




std::ostream& Parameters::print_options(std::ostream &out) const
{
  std::map<std::string, std::unique_ptr<ParamBase> >::const_iterator iter = _parameters.begin();

  out << "\nAvailable options (default values in brackets)\n\n";

  for (; iter != _parameters.end(); ++iter)
  {
    const ParamBase *par = iter->second.get();
    out << add_space(iter->first, _longest_string_key_len+1)
        << par->_description
        << " [" << par->str() << "]\n";
  }

  out << "\n";

  return out;
}




std::ostream& Parameters::print_parameters(std::ostream &out) const
{
  std::map<std::string, std::unique_ptr<ParamBase> >::const_iterator iter = _parameters.begin();

  for (; iter != _parameters.end(); ++iter)
  {
    const ParamBase *par = iter->second.get();
    out << add_space(iter->first, _longest_string_key_len+1)
        << add_space(par->str(), _longest_string_value_len+1)
        << par->_description << "\n";
  }
  out << "\n";

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
}




void Parameters::update_longest_string_key_len()
{
  _longest_string_key_len = 0;

  std::map<std::string, std::unique_ptr<ParamBase> >::const_iterator iter = _parameters.begin();

  for (; iter != _parameters.end(); ++iter)
  {
    const int len_key_string = iter->first.size();

    if (len_key_string > _longest_string_key_len)
      _longest_string_key_len = len_key_string;
  }
}




void Parameters::update_longest_string_value_len()
{
  _longest_string_value_len = 0;

  std::map<std::string, std::unique_ptr<ParamBase> >::const_iterator iter = _parameters.begin();

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
// Auxiliary functions
//
//==============================================================================
int argcheck(int argc, char **argv, const char *arg)
{
  for(int i = 1; i < argc; ++i)
  {
    // strcmp returns 0 if the strings are equal
    if(strcmp(argv[i], arg) == 0)
      return(i);
  }

  return 0;
}




std::string add_space(const std::string &str, int length)
{
  const int n_spaces = std::max(length - (int)str.size(), 0);
  return str + std::string(n_spaces, ' ');
}

