/* CirKit: A circuit toolkit
 * Copyright (C) 2017-2019  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>
#include <mockturtle/algorithms/xmg_algebraic_rewriting.hpp>
#include <mockturtle/views/depth_view.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice {
template<typename S> struct mighty_traits {};
template<> struct mighty_traits<mig_t> { using params_type = mockturtle::mig_algebraic_depth_rewriting_params; using ntk_type = mockturtle::mig_network; };
template<> struct mighty_traits<xmg_t> { using params_type = mockturtle::xmg_algebraic_depth_rewriting_params; using ntk_type = mockturtle::xmg_network; };

class mighty_command : public cirkit::cirkit_command<mighty_command, mig_t, xmg_t> {
public:
  mighty_command(environment::ptr& env) : cirkit::cirkit_command<mighty_command, mig_t, xmg_t>(env, "Performs algebraic MIG rewriting", "applies algebraic MIG rewriting to {0}") {
    opts.add_option("--strategy", strategy, "optimization strategy", true)->set_type_name("strategy in {dfs=0, aggressive=1, selective=2}");
    opts.add_flag("--area_aware", "do not increase area");
  }

  template<class Store> void execute_store() {
    using params_type = typename mighty_traits<Store>::params_type;
    using ntk_type = typename mighty_traits<Store>::ntk_type;
    params_type ps;
    ps.strategy = static_cast<typename params_type::strategy_t>(strategy);
    ps.allow_area_increase = !is_set("area_aware");
    auto* ntk_p = static_cast<ntk_type*>(store<Store>().current().get());
    mockturtle::depth_view depth_ntk{*ntk_p};
    if constexpr (std::is_same_v<Store, mig_t>)
      mockturtle::mig_algebraic_depth_rewriting(depth_ntk, ps, &st);
    else if constexpr (std::is_same_v<Store, xmg_t>)
      mockturtle::xmg_algebraic_depth_rewriting(depth_ntk, ps);
    *ntk_p = mockturtle::cleanup_dangling(*ntk_p);
  }

  nlohmann::json log() const override {
   return { {"time_total", mockturtle::to_seconds( st.time_total )} };
  }

private:
  unsigned strategy{0u};
  mockturtle::mig_algebraic_depth_rewriting_stats st;
};

ALICE_ADD_COMMAND( mighty, "Synthesis" )

} // namespace alice
