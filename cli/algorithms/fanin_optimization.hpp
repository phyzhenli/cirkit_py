#include <alice/alice.hpp>
#include <mockturtle/algorithms/xag_optimization.hpp>
#include "../utils/cirkit_command.hpp"

namespace alice {

class fanin_opt_command: public cirkit::cirkit_command<fanin_opt_command, xag_t> {
public:
  fanin_opt_command(environment::ptr& env):
    cirkit::cirkit_command<fanin_opt_command, xag_t>(env, "Optimizes AND gates by computing transitive linear fanin", "applies optimization to {0}")
  { }

  template<class Store> void execute_store() {
    if (store<xag_t>().current_index() >= 0) {
      auto xag_p = static_cast<mockturtle::xag_network*>(store<xag_t>().current().get());
      *xag_p = mockturtle::xag_constant_fanin_optimization(*xag_p);
    }
  }

};

ALICE_ADD_COMMAND(fanin_opt, "Synthesis")

} // namespace alice
