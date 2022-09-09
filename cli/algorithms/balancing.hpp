#include <alice/alice.hpp>

#include <mockturtle/algorithms/balancing.hpp>
#include <mockturtle/algorithms/balancing/sop_balancing.hpp>
#include <mockturtle/algorithms/balancing/esop_balancing.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice {

class balancing_command: public cirkit::cirkit_command<balancing_command, aig_t, mig_t, xag_t, xmg_t, klut_t> {
    mockturtle::balancing_params ps;
    mockturtle::balancing_stats  st;

public:
    balancing_command(environment::ptr& env):
      cirkit::cirkit_command<balancing_command, aig_t, mig_t, xag_t, xmg_t, klut_t>(env, "Performs balancing", "apply balancing to {0}") {
        add_flag("-p,--progress", ps.progress, "show progress");
        add_flag("-v,--verbose", ps.verbose, "show statistics");
        add_flag("-e,--esop", "using ESOP balancing");
    }

    template<class Store>
    void execute_store() {
        using namespace mockturtle;
        using ntk_t = typename Store::element_type;
        auto  ntk_p = static_cast<ntk_t*>(store<Store>().current().get());
        if (is_set("esop"))
            *ntk_p = balancing(*ntk_p, {esop_rebalancing<ntk_t>{}}, ps, &st);
        else
            *ntk_p = balancing(*ntk_p, {sop_rebalancing<ntk_t>{}}, ps, &st);
    }

    nlohmann::json log() const override
    { return { {"time_total", mockturtle::to_seconds( st.time_total )} }; }
};

ALICE_ADD_COMMAND(balancing, "Synthesis")

} // namespace alice
