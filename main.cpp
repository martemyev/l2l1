#include "parameters.hpp"
#include "l2l1.hpp"

int main(int argc, char **argv)
{
  Parameters param(argc, argv);

  if (param._verbose > 1)
    param.print_parameters();

  l2l1(param);

  return 0;
}
