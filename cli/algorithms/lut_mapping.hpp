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

#include <mockturtle/algorithms/cut_enumeration/spectr_cut.hpp>
#include <mockturtle/algorithms/lut_mapping.hpp>
#include <mockturtle/algorithms/collapse_mapped.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice {

class lut_mapping_command : public cirkit::cirkit_command<lut_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t> {
public:
  lut_mapping_command( environment::ptr& env ) : cirkit::cirkit_command<lut_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t>( env, "Performs k-LUT mapping", "apply LUT-mapping to {0}" ) {
    add_option( "-k,--lutsize", ps.cut_enumeration_ps.cut_size, "cut size", true );
    add_option( "--lutcount", ps.cut_enumeration_ps.cut_limit, "number of cuts per node", true );
    add_option( "--cost", cost, "cost function for priority cut selection", true )->set_type_name( "cost function in {mf=0, spectral=1}");
    add_flag( "--nofun", "do not compute cut functions (only when cost function is 0)" );
    add_new_option();
  }

  template<class Store>
  inline void execute_store() {
    using ntk_t = typename Store::element_type;
    using mapped_t = mockturtle::mapping_view<ntk_t, true>;
    auto  ntk = store<Store>().current().get();
    auto  mapped = mapped_t{*ntk};
    if (is_set("nofun"))
      mockturtle::lut_mapping( mapped, ps );
    else {
      if ( cost == 0u )
        mockturtle::lut_mapping<mapped_t, true>( mapped, ps );
      else if ( cost == 1u ) {
        if constexpr ( mockturtle::has_is_xor_v<mapped_t> )
          mockturtle::lut_mapping<mapped_t, true, mockturtle::cut_enumeration_spectr_cut>( mapped, ps );
        else
          env->err() << "[e] network type must support XOR gates to apply spectral cut function\n";
      }
    }
    mockturtle::klut_network klut_ntk;
    mockturtle::names_view<mockturtle::klut_network> named_ntk( klut_ntk );
    bool success = mockturtle::collapse_mapped_network<mockturtle::names_view<mockturtle::klut_network>>( named_ntk, mapped );
    if (success) {
      extend_if_new<klut_t>();
      store<klut_t>().current() = std::make_shared<klut_nt>(named_ntk);
      set_default_option<klut_t>();
    }
    else
      env->err() << "[e] collapse_mapped_network failed\n";
  }

private:
  mockturtle::lut_mapping_params ps;
  unsigned cost{0u};
};

ALICE_ADD_COMMAND( lut_mapping, "Mapping" )

}
