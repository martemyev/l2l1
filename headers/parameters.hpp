#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <climits>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>



const std::string DEFAULT_FILE_NAME = "no-file";
const int DEFAULT_PRINT_LEN = 10; ///< Default length of strings for printing
                                  ///< aligned key words and values of the
                                  ///< parameters
const int SPACE_BETWEEN = 5; ///< The space between the name of the option (to
                             ///< define a parameter) and its description.




//==============================================================================
//
// Base abstract class for a one parameter
//
//==============================================================================
class ParamBase
{
public:

  ParamBase()
    : _description(""),
      _priority(0)
  { }

  ParamBase(const std::string &desc, int priority)
    : _description(desc),
      _priority(priority)
  { }

  ParamBase(const ParamBase &pb)
    : _description(pb._description),
      _priority(pb._priority)
  { }

  ParamBase& operator=(const ParamBase &pb)
  {
    _description = pb._description;
    _priority = pb._priority;
    return *this;
  }

  virtual ~ParamBase() { }

  std::string get_description() const { return _description; }

  int get_priority() const { return _priority; }

  /// Read the value of the parameter from a string
  /// @param from a string from which the value is read
  virtual void read(const std::string &from) = 0;

  /// Convert the value of the parameter into a string
  virtual std::string str() const = 0;

protected:

  /// Description of the parameter (appears when help is invoked, for example)
  std::string _description;

  /// This attribute controls the order of outputting the parameters (when help
  /// is invoked, for example). Without this parameter (or, when there are
  /// several parameters with the same priority) the parameters appear in
  /// alphabetical order of keys.
  int _priority;
};

/**
 * Shared (smart) pointer to an object of an abstract ParamBase class.
 */
typedef std::shared_ptr<ParamBase> ParamBasePtr;




//==============================================================================
//
// Template class for a one parameter
//
//==============================================================================
template <typename T>
class OneParam : public ParamBase
{
public:

  /// Constructor takes a description of the parameter and a pointer to a value.
  /// It also may take a priority value of the parameter, however it's optional
  OneParam(const std::string &desc,
           T* val,
           int priority = INT_MAX)
    : ParamBase(desc, priority),
      _value(val)
  { }

  OneParam(const OneParam &op)
    : ParamBase(op),
      _value(op._value)
  { }

  OneParam& operator=(const OneParam &op)
  {
    ParamBase::operator=(op);
    _value = op._value;
    return *this;
  }

  virtual ~OneParam() { }

  /// Value of the parameter is saved somewhere else, and here we keep the
  /// pointer to it. This member is public, yes.
  T* _value;

  /// Read the value of the parameter from a string
  /// @param a string from which the value is read
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

  ~Parameters() { }

  /// Two files for comparison or two components of one solution. In case we
  /// compute relative errors we assume that the _file_0 will contain a
  /// reference solution. The files are in binary format with floating point
  /// numbers in SINGLE precision. If the files represent seismograms we assume
  /// that columns represent traces and each row is a time step.
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

  /// Whether or not compute the L2 and L1 norms of differences between the
  /// given files.
  int _l2l1;

  /// The name of the file representing the difference between two datasets from
  /// the files _file_0 and _file_1. The data in the difference file are also
  /// saved in single precision.
  std::string _diff_file;

  /// Whether to scale the data from the _file_1 in such a way that it might be
  /// closer to the data from the _file_0. That creates a new file with the
  /// suffix 'scaled' (or similar). This works when _scale_file_1 == 1, but when
  /// it's 2, the scaling factor is handled by the variable _scale_factor.
  int _scale_file_1;

  /// A factor for the scaling of the _file_1, when _scale_file_1 == 2.
  double _scale_factor;

  /// Whether to shift the data from the _file_1 in such a way that it might be
  /// closer to the data from the _file_0. That creates a new file with the
  /// suffix 'shifted' (or similar).
  bool _shift_file_1;

  /// Compute the cross correlation between the data from the given files. This
  /// parameter may take the following values:
  /// 0 (default) - do not compute cross correlation
  /// 1 - compute cross correlation trace-by-trace and then show the min and max
  ///     values among all the traces
  /// 2 - compute cross correlation (one value) for all traces
  int _cross_correlation;

  /// Compute the cross correlation (as defined by the _cross_correlation
  /// parameter) in the set of lags from -(_lag_region) to +(_lag_region). The
  /// second dataset is supposed to be lagged against the first one.
  int _lag_region;

  /// Compute the RMS (root mean square) for each column (trace), so that the
  /// output is an array. Depending on the value of this parameter there may be
  /// computed:
  /// rms = 0 (default: no computation)
  /// rms = 1 (compute 2 RMS arrays for each input file)
  /// rms = 2 (compute 1 RMS array of amplitude of a vector solution - based on
  ///          two input files representing Ux and Uz components of the field)
  int _rms;


  typedef std::map<std::string, ParamBasePtr> ParaMap;

  /// The map between the key word representing a parameters, and its value
  /// (and maybe other attributes such as description)
  ParaMap _parameters;

  /// Read the values from the command line
  void read_command_line(int argc, char **argv);

  /// Print possible options of the program
  std::ostream& print_options(std::ostream &out = std::cout) const;

  /// Print the parameters with which the program is going to work
  std::ostream& print_parameters(std::ostream &out = std::cout) const;

  /// Check that the parameters make sense
  void check_parameters() const;

  /// Length of the longest string representing the key words of the parameters
  mutable int _longest_string_key_len;

  /// Length of the longest string representing the values of the parameters
  mutable int _longest_string_value_len;


protected:

  /// The name of the function is self-explaining
  void update_longest_string_key_len() const;
  /// The name of the function is self-explaining
  void update_longest_string_value_len() const;

  Parameters(const Parameters&);
  Parameters& operator =(const Parameters&);
};




//==============================================================================
//
// Compare two pairs containing info about parameters by the priority of the
// parameters
//
//==============================================================================
bool compare_by_parameter_priority(const std::pair<std::string, ParamBasePtr> &a,
                                   const std::pair<std::string, ParamBasePtr> &b);


#endif // PARAMETERS_HPP
