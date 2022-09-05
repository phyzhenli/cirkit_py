#include <alice/alice.hpp>
#include <mockturtle/algorithms/mapper.hpp>
#include "../utils/cirkit_command.hpp"

namespace alice {

class map_command: public cirkit::cirkit_command<map_command, aig_t, mig_t, xag_t, xmg_t, klut_t> {
    mockturtle::map_params ps;
    mockturtle::map_stats  st;

public:
    map_command(environment::ptr& env):
      cirkit::cirkit_command<map_command, aig_t, mig_t, xag_t, xmg_t, klut_t>(env, "technology mapping", "apply technology mapping to {0}") {
        add_new_option();
        add_option("-d,--required_time", ps.required_time, "required time", true);
    }

    rules validity_rules() const override { return {has_store_element<tlib_t>(env)}; }

    template<class Store>
    void execute_store() {
        auto lib = env->store<tlib_t>().current();
        auto mapped_ntk = mockturtle::map(*env->store<Store>().current(), *lib, ps, &st);
        if (mapped_ntk.size() == 0) {
            env->err() << "[e] map failed\n";
            return;
        }
        extend_if_new<mapped_t>();
        store<mapped_t>().current() = std::make_shared<mapped_nt>(mapped_ntk);
        set_default_option<mapped_t>();
    }

    nlohmann::json log() const override {
        return {
            {"area",  st.area },
            {"delay", st.delay},
            {"power", st.power},
            {"time_mapping", mockturtle::to_seconds(st.time_mapping)},
            {"time_total",   mockturtle::to_seconds(st.time_total)}
        };
    }
};

ALICE_ADD_COMMAND(map, "Mapping")

}
