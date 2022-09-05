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
#include <mockturtle/algorithms/refactoring.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xmg_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/bidecomposition.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice {
template<typename S> struct resyn_traits {};
template<> struct resyn_traits<xag_t> { using npn_type = mockturtle::xag_npn_resynthesis<mockturtle::xag_network>; };
template<> struct resyn_traits<aig_t> { using npn_type = mockturtle::xag_npn_resynthesis<mockturtle::aig_network>; };
template<> struct resyn_traits<mig_t> { using npn_type = mockturtle::mig_npn_resynthesis; };
template<> struct resyn_traits<xmg_t> { using npn_type = mockturtle::xmg_npn_resynthesis; };

class refactor_command : public cirkit::cirkit_command<refactor_command, aig_t, mig_t, xag_t, xmg_t> {
  mockturtle::refactoring_params ps;
  mockturtle::refactoring_stats st;
  unsigned strategy{0u};

public:
  refactor_command(environment::ptr& env): cirkit::cirkit_command<refactor_command, aig_t, mig_t, xag_t, xmg_t>(env, "Performs cut rewriting", "apply cut rewriting to {0}") {
    add_option( "--max_pis", ps.max_pis, "maximum number of PIs in MFFC", true );
    add_option( "--strategy", strategy, "resynthesis strategy", true )->set_type_name( "strategy in {npn=0, akers=1, bidec=2, dsd=3}" );
    add_flag( "-z,--zero_gain", ps.allow_zero_gain, "enable zero-gain refactoring" );
    add_flag( "-p,--progress", ps.progress, "show progress" );
    add_flag( "-v,--verbose", ps.verbose, "show statistics" );
  }

  template<class Store> void execute_store() {
    using ntk_t = typename store_traits<Store>::net_type;
    auto  ntk_p = static_cast<ntk_t*>(store<Store>().current().get());
    switch (strategy) {
    default:
    case 0:  {
      typename resyn_traits<Store>::npn_type resyn;
      mockturtle::refactoring(*ntk_p, resyn, ps);
      break; }
    case 1:  {
      mockturtle::akers_resynthesis<ntk_t> resyn;
      mockturtle::refactoring(*ntk_p, resyn, ps);
      break; }
    case 2:  {
      mockturtle::bidecomposition_resynthesis<ntk_t> resyn;
      mockturtle::refactoring(*ntk_p, resyn, ps);
      break; }
    case 3:  {
      mockturtle::bidecomposition_resynthesis<ntk_t> fallback;
      mockturtle::dsd_resynthesis<ntk_t, decltype(fallback)> resyn{fallback};
      mockturtle::refactoring(*ntk_p, resyn, ps);
      break; }
    }
    *ntk_p = mockturtle::cleanup_dangling(*ntk_p);
  }

  nlohmann::json log() const override
  { return { {"time_total", mockturtle::to_seconds( st.time_total )} }; }

};

ALICE_ADD_COMMAND(refactor, "Synthesis")

} // namespace alice
