#include "utilities.hpp"

#if defined(__linux__) || defined(__APPLE__)
  #include <sys/time.h> // for time measurements
#endif

#include <execinfo.h>
#include <cstdlib>
#include <fstream>
#include <climits>
#include <cerrno>
#include <cstring>

//------------------------------------------------------------------------------
//
// expect and require
//
//------------------------------------------------------------------------------
void requirement_fails(const char *file,
                       unsigned int line,
                       std::string message)
{
  std::string exc = "Exception:\nfile = " + std::string(file) +
                    "\nline = " + d2s(line) +
                    "\nmessage = " + message + "\n";

#if defined(__linux__)
  const int backtraceSize = 20;
  void *array[backtraceSize];
  int size = backtrace(array, backtraceSize);
  char **strings = backtrace_symbols(array, size);

  exc += "backtrace:\nsize = " + d2s<int>(size) + "\n";
  for (int i = 0; i < size; ++i)
    exc += std::string(strings[i]) + "\n";

  free(strings);
#endif

  throw std::runtime_error(exc);
}

//------------------------------------------------------------------------------
//
// Time measurement (wall time)
//
//------------------------------------------------------------------------------
double get_wall_time()
{
#if defined(__linux__) || defined(__APPLE__)
  struct timeval time;
  const int ierr = gettimeofday(&time, NULL);
  require(ierr == 0, "gettimeofday returned an error code " + d2s(ierr));

  return (1.0*time.tv_sec + 1.0e-6*time.tv_usec);

#elif(_WIN32)
  require(false, "Not implemented for Windows");
  return 0.;
#else
  require(false, "Not implemented for this unknown OS");
  return 0.;
#endif
}

//------------------------------------------------------------------------------
//
// Show time elapsed from the given t_wall_begin
//
//------------------------------------------------------------------------------
void show_time(double t_wall_begin)
{
//  std::cout.setf(std::ios::scientific);
  std::cout.precision(8);

  std::cout << "\nTOTAL TIME\n";
  std::cout << "wall time = " << get_wall_time() - t_wall_begin << " seconds"
            << std::endl;
}

//------------------------------------------------------------------------------
//
// Name of a file without a path
//
//------------------------------------------------------------------------------
std::string file_name(const std::string &path)
{
  if (path == "") return path;

  size_t pos = 0;
#if defined(__linux__) || defined(__APPLE__)
  pos = path.find_last_of('/');
#elif defined(_WIN32)
  pos = path.find_last_of('\\');
#endif

  if (pos == std::string::npos)
    return path; // there is no '/' in the path, so this is the filename

  return path.substr(pos + 1);
}

//------------------------------------------------------------------------------
//
// Path of a given file
//
//------------------------------------------------------------------------------
std::string file_path(const std::string &path)
{
  if (path == "") return path;

  size_t pos = 0;
#if defined(__linux__) || defined(__APPLE__)
  pos = path.find_last_of('/');
#elif defined(_WIN32)
  pos = path.find_last_of('\\');
#endif

  if (pos == std::string::npos)
    return ""; // there is no '/' in the path, the path is "" then

  return path.substr(0, pos + 1);
}

//------------------------------------------------------------------------------
//
// Stem of a given file (no path, no extension)
//
//------------------------------------------------------------------------------
std::string file_stem(const std::string &path)
{
  if (path == "") return path;

  // get a file name from the path
  const std::string fname = file_name(path);

  // extract a stem and return it
  size_t pos = fname.find_last_of('.');
  if (pos == std::string::npos)
    return fname; // there is no '.', so this is the stem

  return fname.substr(0, pos);
}

//------------------------------------------------------------------------------
//
// Extension of a given file
//
//------------------------------------------------------------------------------
std::string file_extension(const std::string &path)
{
  if (path == "") return path;

  // extract a file name from the path
  const std::string fname = file_name(path);

  size_t pos = fname.find_last_of('.');
  if (pos == std::string::npos)
    return ""; // there is no '.', so there is no extension

  // extract an extension and return it
  return fname.substr(pos);
}

//------------------------------------------------------------------------------
//
// Check if the given file exists
//
//------------------------------------------------------------------------------
bool file_exists(const std::string &path)
{
  if (path == "") return false; // no file - no existance

  // This is not the fastest method, but it should work on all operating
  // systems. Some people also not that this method check 'availibility' of the
  // file, not its 'existance'. But that's what we actually need. If a file
  // exists, but it's not available (even for reading), we believe, that the
  // file doesn't exist.
  bool exists = false;
  std::ifstream in(path.c_str());
  if (in.good())
    exists = true; // file exists and is in a good state
  in.close();

  return exists;
}

//------------------------------------------------------------------------------
//
// Get an absolute path according to the given relative one
//
//------------------------------------------------------------------------------
std::string absolute_path(const std::string &rel_path)
{
#if defined(__linux__) || defined(__APPLE__)
  char abs_path[PATH_MAX];
  char *res = realpath(rel_path.c_str(), abs_path);
  require(res != NULL, "The function realpath() failed with the input "
          "(relative path) = '" + rel_path + "'. errno is set to " + d2s(errno)+
          " which means '" + std::string(strerror(errno)) + "'");
  return std::string(abs_path);
#else
  require(false, "absolute_path() is not implemented for this OS");
#endif
}

//------------------------------------------------------------------------------
//
// Check endianness
//
//------------------------------------------------------------------------------
bool is_big_endian()
{
  union
  {
    int i;
    char c[sizeof(int)];
  } x;
  x.i = 1;
  return x.c[0] == 1;
}

//------------------------------------------------------------------------------
//
// Get the endianness of the machine
//
//------------------------------------------------------------------------------
std::string endianness()
{
  return (is_big_endian() ? "BigEndian" : "LittleEndian");
}

//------------------------------------------------------------------------------
//
// Check if there is a string in the array of strings
//
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
//
// Add some space to the given string so it has the requires length
//
//------------------------------------------------------------------------------
std::string add_space(const std::string &str, int length)
{
  // how spaces need to add (if the string longer than the required length,
  // nothing is added)
  const int n_spaces = std::max(length - (int)str.size(), 0);
  return str + std::string(n_spaces, ' ');
}

//------------------------------------------------------------------------------
//
// Get the info about memory consumption during the runtime
//
//------------------------------------------------------------------------------
int parse_proc_line(char *line)
{
  int i = strlen(line);
  while (*line < '0' || *line > '9') ++line;
  line[i-3] = '\0';
  i = atoi(line);
  return i;
}
void get_memory_consumption(int &resident_memory, int &virtual_memory)
{
#if defined(__linux__)
  FILE *file = fopen("/proc/self/status", "r");
  char line[128];
  bool vm_found = false, rm_found = false;
  while (fgets(line, 128, file) != NULL || !(vm_found && rm_found))
  {
    if (strncmp(line, "VmSize:", 7) == 0)
    {
      virtual_memory = parse_proc_line(line);
      vm_found = true;
    }
    if (strncmp(line, "VmRSS:", 6) == 0)
    {
      resident_memory = parse_proc_line(line);
      rm_found = true;
    }
  }
  fclose(file);
  return;

#elif defined(__APPLE__)

/*
  task_t task = MACH_PORT_NULL;

  require(task_for_pid(current_task(), getpid(), &task) == KERN_SUCCESS,
          "Error trying to get the memory consumption on Mac OS X, the call "
          "'task_for_pid' failed");

  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

  require(task_info(task, TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count)
          == KERN_SUCCESS, "Error trying to get the memory consumption on Mac "
          "OS X, the call 'task_info' failed");

  vm_region_basic_info_data_64_t b_info;
  vm_address_t address = GLOBAL_SHARED_TEXT_SEGMENT;
  vm_size_t size;
  mach_port_t object_name;
  t_info_count = VM_REGION_BASIC_INFO_COUNT_64;
  if (vm_region_64(task,
                   &address,
                   &size,
                   VM_REGION_BASIC_INFO,
                   (vm_region_info_t)&b_info,
                   &t_info_count,
                   &object_name) == KERN_SUCCESS &&
      b_info.reserved &&
      size == SHARED_TEXT_REGION_SIZE &&
      t_info.virtual_size > SHARED_TEXT_REGION_SIZE + SHARED_DATA_REGION_SIZE)
  {
    t_info.virtual_size -= SHARED_TEXT_REGION_SIZE + SHARED_DATA_REGION_SIZE;
  }

  int resident_pages = t_info.resident_size; // this refers to pages
  int virtual_pages  = t_info.virtual_size;  // this refers to pages

  require(mach_port_deallocate(mach_task_self(), task) == KERN_SUCCESS,
          "Error trying to get the memory consumption on Mac OS X, the call "
          "'mach_port_deallocate' failed");

  int psize = getpagesize(); // size of a page in bytes

  resident_memory = resident_pages * psize / 1024; // RAM memory in KB
  virtual_memory  = virtual_pages  * psize / 1024; // VM in KB
  return;

  pid_t processID = getpid(); // get the ID of the current process

  // get the info
  struct proc_taskallinfo task_info;
  int nb = proc_pidinfo(processID, PROC_PIDTASKALLINFO, 0, &task_info, sizeof(task_info));

  if (nb <= 0 || nb < sizeof(task_info))
  {
    std::cerr << "\nWARNING: proc_pidinfo returned wrong results trying to get memory"
                 " consumption on Mac OS X\n\n";
    resident_memory = 0;
    virtual_memory  = 0;
    return;
  }

  resident_memory = task_info.ptinfo.pti_resident_size; // in bytes
  virtual_memory  = task_info.ptinfo.pti_virtual_size;  // in bytes
  return;
*/

  resident_memory = 0;
  virtual_memory  = 0;
  return;

#else

  std::cerr << "\nWARNING: get_memory_consumption was not implemented "
               "for this OS\n\n";
  resident_memory = 0;
  virtual_memory  = 0;
  return;

#endif
}



void read_binary(const std::string &filename,
                 int n_values,
                 double *values)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
  require(in, "File '" + filename + "' can't be opened.");

  in.seekg(0, in.end); // jump to the end of the file
  int length = in.tellg(); // total length of the file in bytes
  int size_value = length / n_values; // size (in bytes) of one value

  require(length % n_values == 0, "The number of of bytes in the file " +
          filename + " is not divisible by the number of elements " +
          d2s(n_values));

  in.seekg(0, in.beg); // jump to the beginning of the file

  if (size_value == sizeof(double))
  {
    in.read((char*)values, n_values*size_value); // read all at once

    require(n_values == (int)in.gcount(), "The number of successfully read "
            "elements is different from the expected one");
  }
  else if (size_value == sizeof(float))
  {
    float val = 0;
    for (int i = 0; i < n_values; ++i)  // read element-by-element
    {
      in.read((char*)&val, size_value); // read a 'float' value
      values[i] = val;                  // convert it to a 'double' value
    }
  }
  else require(false, "Uknown size of an element (" + d2s(size_value) + ") in "
               "bytes. Expected one is either sizeof(float) = " +
               d2s(sizeof(float)) + ", or sizeof(double) = " +
               d2s(sizeof(double)));

  in.close();
}


