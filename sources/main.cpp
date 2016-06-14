#include "compute.hpp"
#include "parameters.hpp"

int main(int argc, char **argv)
{
  try
  {
    Parameters param(argc, argv);

    if (param._verbose > 1)
      param.print_parameters();

    param.check_parameters();

    Compute c(param);
    c.run();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cout << "\n\nUnknown exception\n" << std::endl;
    return 2;
  }

  return 0;
}
