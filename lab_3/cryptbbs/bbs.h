#include <cstdint>
#include <sys/types.h>
#include <unistd.h>
#include <devctl.h>

namespace bbs {


struct BBSParams
{
    std::uint32_t seed;
    std::uint32_t p;
    std::uint32_t q;
};


}

#define GET_ELEMENT __DIOF(_DCMD_MISC, 1 ,bbs::BBSParams)
#define GEN_PARAM_INIT __DIOT(_DCMD_MISC, 2, std::uint32_t)
