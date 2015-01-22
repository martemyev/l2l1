#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <ostream>
#include <iostream>
#include <sstream>




//==============================================================================
//
// Base abstract class for a one parameter
//
//==============================================================================
class ParamBase
{
public:

  /// Description of the parameter (appears when help is invoked, for example)
  std::string _description;

  /// Read the value of the parameter from a string
  /// @param from a string from which the value is read
  virtual void read(const std::string &from) = 0;

  /// Convert the value of the parameter into a string
  virtual std::string str() const = 0;

  static bool compare_by_desc(const std::unique_ptr<ParamBase> &a, const std::unique_ptr<ParamBase> &b)
  {
    return a->_description < b->_description;
  }
};




//==============================================================================
//
// Template class for a one parameter
//
//==============================================================================
template <typename T>
class OneParam : public ParamBase
{
public:

  /// Construator takes a description of the parameter and a pointer to a value
  OneParam(const std::string &desc,
           T* val)
    : _value(val)
  {
    _description = desc;
  }

  /// Value of the parameter is saved somewhere else, and here we keep the
  /// pointer to it
  T* _value;

  /// Read the value of the parameter from a string
  /// @param from a string from which the value is read
  virtual void read(const std::string &from)
  {
    std::istringstream is(from);
    is >> *(_value);
  }

  /// Convert the value of the parameter into a string
  virtual std::string str() const
  {
    std::ostringstream os;
    os << *(_value);
    return os.str();
  }
};




//==============================================================================
//
// Class which handles the parameters of this program
//
//==============================================================================
class Parameters
{
public:

  /// Constructor takes the command line
  Parameters(int argc, char **argv);

  /// Two files for comparison. Since we compute relative errors we assume that
  /// the file0 will contain a reference solution. The files are in binary
  /// format with floating point numbers in single precision.
  std::string _file_0, _file_1;

  /// The files are binary, in form of a table. However, in general case, we
  /// don't need to know the number of the columns to compute the errors.
  /// Nevertheless, sometimes we need to know the errors in specific columns,
  /// therefore we need to keep the data from the files in a table (2D array)
  /// format.
  int _n_cols;

  /// Compute the errors in a specific region of columns [_col_beg, _col_end)
  int _col_beg, _col_end;

  /// Compute the errors in a specific region of columns [_row_beg, _row_end)
  int _row_beg, _row_end;

  /// Verbosity level
  int _verbose;

  /// The map between the key word representing a parameters, and its value
  /// (and maybe other attributes such as description)
  std::map<std::string, std::unique_ptr<ParamBase> > _parameters;

  /// Read the values from the command line
  void read_command_line(int argc, char **argv);

  /// Print possible options of the program
  std::ostream& print_options(std::ostream &out = std::cout) const;

  /// Print the parameters with which the program is going to work
  std::ostream& print_parameters(std::ostream &out = std::cout) const;

  /// Check that the parameters make sense
  void check_parameters() const;

  /// Length of the longest string representing the key words of the parameters
  int _longest_string_key_len;

  /// Length of the longest string representing the values of the parameters
  int _longest_string_value_len;

  /// Default file name
  static std::string DEFAULT_FILE_NAME;

  /// Default length of strings for printing aligned key words and values of the
  /// parameters
  static int DEFAULT_PRINT_LEN;

protected:

  void update_longest_string_key_len();
  void update_longest_string_value_len();
};




//==============================================================================
//
// Auxiliary functions
//
//==============================================================================
int argcheck(int argc, char **argv, const char *arg);
std::string add_space(const std::string &str, int length);

#endif // PARAMETERS_HPP
