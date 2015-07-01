#include "parameters.hpp"
#include "compute.hpp"

int main(int argc, char **argv)
{
  Parameters param(argc, argv);

  if (param._verbose > 1)
    param.print_parameters();

  param.check_parameters();

  Compute c(param);
  c.run();

  return 0;
}
