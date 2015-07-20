#include <iostream>
#include <unistd.h>
#include "scene_lua.hpp"
#include "render.hpp"

int main(int argc, char** argv)
{
  std::string filename = "scene.lua";
  if (argc >= 2) {
    filename = argv[1];
  }

  int c;
  while ((c = getopt(argc, argv, ":t:s:oa")) != -1) {
    switch(c) {
    case 't':
      xDiv = yDiv = atoi(optarg);
      break;
    case 's':
      SuperSamples = atoi(optarg);
      if (bUseAdaptive)
      {
        std::cerr << "Can only use one type of anti-aliasing" << std::endl;
        return 1;
      }
      break;
    case 'o':
      bUseOctree = true;
      break;
    case 'a':
      bUseAdaptive = true;
      if (SuperSamples > 1)
      {
        std::cerr << "Can only use one type of anti-aliasing" << std::endl;
        return 1;
      }
      break;
    case ':':
      fprintf(stderr,
          "Option -%c requires an operand\n", optopt);
      break;
    case '?':
      fprintf(stderr,
          "Unrecognized option: '-%c'\n", optopt);
    }
  }

  if (!run_lua(filename)) {
    std::cerr << "Could not open " << filename << std::endl;
    return 1;
  }
}
