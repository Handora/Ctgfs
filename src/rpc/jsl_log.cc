#include "rpc/jsl_log.h"

namespace ctgfs {
namespace rpc {

int JSL_DEBUG_LEVEL = 0;
void
jsl_set_debug(int level) {
	JSL_DEBUG_LEVEL = level;
}

}} // namespace rpc, ctgfs
