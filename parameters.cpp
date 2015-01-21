#include "parameters.hpp"
#include <cstring>
#include <algorithm>



//==============================================================================
//
// Parameters class
//
//==============================================================================
Parameters::Parameters(int argc, char **argv)
  : _file_0("no-file"),
    _file_1("no-file"),
    _n_cols(0),
    _col_beg(0),
    _col_end(-1),
    _row_beg(0),
    _row_end(-1),
    _verbose(2)
{
  _parameters["-f0"]    = std::unique_ptr<ParamBase>(new OneParam<std::string>("file name (reference solution)", &_file_0));
  _parameters["-f1"]    = std::unique_ptr<ParamBase>(new OneParam<std::string>("file name (solution to compare)", &_file_1));
  _parameters["-ncols"] = std::unique_ptr<ParamBase>(new OneParam<int>("number of columns in the files", &_n_cols));
  _parameters["-c0"]    = std::unique_ptr<ParamBase>(new OneParam<int>("first column for comparison", &_col_beg));
  _parameters["-c1"]    = std::unique_ptr<ParamBase>(new OneParam<int>("last column for comparison (not including)", &_col_end));
  _parameters["-r0"]    = std::unique_ptr<ParamBase>(new OneParam<int>("first row for comparison", &_row_beg));
  _parameters["-r1"]    = std::unique_ptr<ParamBase>(new OneParam<int>("last row for comparison (not including)", &_row_end));
  _parameters["-v"]     = std::unique_ptr<ParamBase>(new OneParam<int>("verbosity level", &_verbose));

  if (argc == 1 || argcheck(argc, argv, "-help") || argcheck(argc, argv, "-h"))
  {
    print_options();
    exit(0);
  }

  read_command_line(argc, argv);

  if (_col_end < 0) _col_end = _n_cols;
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
      std::cerr << "Command line argument " << arguments[ar] << " wasn't find\n";
      exit(1);
    }
    iter->second->read(arguments[ar+1]);
  }
}




std::ostream& Parameters::print_options(std::ostream &out) const
{
  std::map<std::string, std::unique_ptr<ParamBase> >::const_iterator iter = _parameters.begin();

  for (; iter != _parameters.end(); ++iter)
  {
    const ParamBase *par = iter->second.get();
    out << add_space(iter->first, _PRINT_LEN)
        << par->_description
        << " [" << par->str() << "]\n";
  }

  return out;
}




std::ostream& Parameters::print_parameters(std::ostream &out) const
{
  std::map<std::string, std::unique_ptr<ParamBase> >::const_iterator iter = _parameters.begin();

  for (; iter != _parameters.end(); ++iter)
  {
    const ParamBase *par = iter->second.get();
    out << add_space(iter->first, _PRINT_LEN)
        << add_space(par->str(), _PRINT_LEN)
        << par->_description << "\n";
  }

  return out;
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

