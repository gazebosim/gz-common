#include <ignition/common.hh>

int main(int argc, char **argv)
{
  try
  {
    // The ignthrow macro is a thin wrapper around std::runtime_error. In
    // addition to the given erorr message, ignthrow preprends the file
    // and line number where the exception occured.
    ignthrow("An example of an exception that is caught.");
  }
  catch(const ignition::common::exception &_e)
  {
    std::cerr << "Caught a runtime error " << _e.what() << "\n\n";
  }

  // This is an uncaught exception.
  ignthrow("This is a run time error");
  return 0;
}
