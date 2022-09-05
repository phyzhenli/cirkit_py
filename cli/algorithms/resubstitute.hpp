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
#include <mockturtle/algorithms/resubstitution.hpp>
#include <mockturtle/algorithms/aig_resub.hpp>
#include <mockturtle/algorithms/mig_resub.hpp>
#include <mockturtle/algorithms/xmg_resub.hpp>
#include <mockturtle/algorithms/xag_resub_withDC.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class resub_command : public cirkit::cirkit_command<resub_command, aig_t, mig_t, xag_t, xmg_t>
{
public:
  resub_command( environment::ptr& env ) : cirkit::cirkit_command<resub_command, aig_t, mig_t, xag_t, xmg_t>( env, "Performs resubstitution", "apply resubstitution to {0}" )
  {
    add_option( "--max_pis", ps.max_pis, "maximum number of PIs in reconvergence-driven window", true );
    add_option( "--max_divisors", ps.max_divisors, "maximum number of divisors to consider", true );
    add_option( "--skip_fanout_limit_for_roots", ps.skip_fanout_limit_for_roots, "maximum fanout of a node to be considered as root", true );
    add_option( "--skip_fanout_limit_for_divisors", ps.skip_fanout_limit_for_divisors, "maximum fanout of a node to be considered as divisor", true );
    add_option( "--depth", ps.max_inserts, "maximum number of nodes inserted by resubstitution", true );
    // add_flag( "-z,--zero_gain", ps.zero_gain, "enable zero-gain resubstitution" );
    add_flag( "-p,--progress", ps.progress, "show progress" );
    add_flag( "-v,--verbose", ps.verbose, "show statistics" );
  }

  template<class Store>
  inline void execute_store()
  {
    using namespace mockturtle;
    if constexpr ( std::is_same_v<Store, aig_t> )
    {
      auto* aig_p = static_cast<aig_network*>( store<Store>().current().get() );
      fanout_view<aig_network> fov{*aig_p};
      depth_view<fanout_view<aig_network>> rsv{fov};
      aig_resubstitution( rsv, ps, &st );
      *aig_p = cleanup_dangling( *aig_p );
    }
    else if constexpr ( std::is_same_v<Store, mig_t> )
    {
      auto* mig_p = static_cast<mig_network*>( store<Store>().current().get() );
      fanout_view<mig_network> fov{*mig_p};
      depth_view<fanout_view<mig_network>> rsv{fov};
      mig_resubstitution( rsv, ps, &st );
      *mig_p = cleanup_dangling( *mig_p );
    }
    else if constexpr ( std::is_same_v<Store, xag_t> )
    {
      auto* xag_p = static_cast<xag_network*>( store<Store>().current().get() );
      fanout_view<xag_network> fov{*xag_p};
      depth_view<fanout_view<xag_network>> rsv{fov};
      resubstitution_minmc_withDC( rsv, ps, &st );
      *xag_p = cleanup_dangling( *xag_p );
    }
    else if constexpr ( std::is_same_v<Store, xmg_t> )
    {
      auto* xmg_p = static_cast<xmg_network*>( store<Store>().current().get() );
      fanout_view<xmg_network> fov{*xmg_p};
      depth_view<fanout_view<xmg_network>> rsv{fov};
      xmg_resubstitution( rsv, ps, &st );
      *xmg_p = cleanup_dangling( *xmg_p );
    }
  }

  nlohmann::json log() const override
  {
   return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  mockturtle::resubstitution_params ps;
  mockturtle::resubstitution_stats st;
};

ALICE_ADD_COMMAND( resub, "Synthesis" )

} // namespace alice
