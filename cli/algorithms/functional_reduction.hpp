#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/functional_reduction.hpp>


#include "../utils/cirkit_command.hpp"

namespace alice {

class reduction_command: public cirkit::cirkit_command<reduction_command, aig_t, mig_t, xag_t, xmg_t> {
    mockturtle::functional_reduction_params ps;
    mockturtle::functional_reduction_stats  st;

public:
    reduction_command(environment::ptr& env):
      cirkit::cirkit_command<reduction_command, aig_t, mig_t, xag_t, xmg_t>(env, "Performs functional reduction", "apply functional reduction to {0}") {
        add_flag("-p,--progress", ps.progress, "show progress");
        add_flag("-v,--verbose", ps.verbose, "show statistics");
    }

    template<class Store>
    void execute_store() {
        using ntk_t = typename store_traits<Store>::net_type;
        auto  ntk_p = static_cast<ntk_t*>(store<Store>().current().get());
        mockturtle::functional_reduction(*ntk_p, ps, &st);
        *ntk_p = mockturtle::cleanup_dangling(*ntk_p);
    }

    nlohmann::json log() const override
    { return { {"time_total", mockturtle::to_seconds( st.time_total )} }; }
};

ALICE_ADD_COMMAND(reduction, "Synthesis")

} // namespace alice
