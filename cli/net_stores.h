#include <alice/alice.hpp>

#include <lorina/aiger.hpp>
#include <lorina/blif.hpp>
#include <lorina/bench.hpp>
#include <lorina/verilog.hpp>

#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/blif_reader.hpp>
#include <mockturtle/io/bench_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/io/write_blif.hpp>
#include <mockturtle/io/write_verilog.hpp>

#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/networks/xmg.hpp>
#include <mockturtle/networks/klut.hpp>
// #include <mockturtle/networks/cover.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>
#include <mockturtle/views/names_view.hpp>
// #include <mockturtle/algorithms/cover_to_graph.hpp>

#include "mockturtle/algorithms/node_resynthesis/mig_npn.hpp"
#include "mockturtle/algorithms/node_resynthesis/xag_npn.hpp"
#include "mockturtle/algorithms/node_resynthesis/xmg_npn.hpp"
#include "mockturtle/algorithms/node_resynthesis.hpp"

using namespace std::string_literals;

#define READ_NET(tag, filename, elem) \
  if (lorina::read_##tag(filename, mockturtle::tag##_reader{elem}) != lorina::return_code::success) \
    std::cout << "[w] parse error\n";

#define WRITE_NET_FILE(type, tag) \
  ALICE_WRITE_FILE(type, tag, elem, filename, cmd) { mockturtle::write_##tag(*elem, filename); }

#define ADD_NET_STORE(type, _mnemonic, _name) \
  using type##_nt = mockturtle::names_view<mockturtle::type##_network>; \
  using type##_t  = std::shared_ptr<type##_nt>; \
  /*template<> struct store_traits<type##_t> { using net_type = mockturtle::type##_network; };*/ \
  ALICE_ADD_STORE(type##_t, #type, _mnemonic, _name, _name"s") \
  ALICE_DESCRIBE_STORE(type##_t, elem) \
  { return fmt::format("i/o = {}/{}   gates = {}", elem->num_pis(), elem->num_pos(), elem->num_gates()); } \
  ALICE_PRINT_STORE_STATISTICS(type##_t, os, elem) { \
    mockturtle::depth_view depth_net{*elem}; \
    os << fmt::format("{}   i/o = {}/{}   gates = {}   level = {}", \
            _name, elem->num_pis(), elem->num_pos(), elem->num_gates(), depth_net.depth()); \
    os << std::endl; \
  } \
  ALICE_LOG_STORE_STATISTICS(type##_t, elem) { \
    mockturtle::depth_view depth_net{*elem};  \
    return { {"pis",   elem->num_pis()},   {"pos",   elem->num_pos()}, \
             {"gates", elem->num_gates()}, {"depth", depth_net.depth()} }; \
  } \
  ALICE_READ_FILE(type##_t, aiger, filename, cmd) { \
    mockturtle::type##_network network; \
    READ_NET(aiger, filename, network); \
    return std::make_shared<type##_nt>(network); \
  } \
  ALICE_READ_FILE(type##_t, ascii_aiger, filename, cmd) { \
    mockturtle::type##_network network; \
    if (lorina::read_ascii_aiger(filename, mockturtle::aiger_reader{network}) != lorina::return_code::success) { \
      std::cout << "[w] parse error\n"; \
    } \ 
    else { \
      return std::make_shared<type##_nt>(network); \
    } \
  } \
  WRITE_NET_FILE(type##_t, bench) \
  WRITE_NET_FILE(type##_t, blif)

#define ADD_NET_STORE_EXT(type, _mnemonic, _name) \
  ADD_NET_STORE(type, _mnemonic, _name) \
  ALICE_READ_FILE(type##_t, verilog, filename, cmd) { \
    mockturtle::type##_network network; \
    READ_NET(verilog, filename, network); \
    return std::make_shared<type##_nt>(network); \
  } \
  WRITE_NET_FILE(type##_t, verilog)


namespace alice {

ALICE_ADD_FILE_TYPE_READ_ONLY(ascii_aiger, "ASCII Aiger");
ALICE_ADD_FILE_TYPE_READ_ONLY(aiger, "Aiger");
ALICE_ADD_FILE_TYPE(bench, "BENCH");
ALICE_ADD_FILE_TYPE(blif, "BLIF");
ALICE_ADD_FILE_TYPE(verilog, "Verilog");

ADD_NET_STORE_EXT(aig, "a", "AIG");
ADD_NET_STORE_EXT(mig, "m", "MIG");
ADD_NET_STORE_EXT(xag, "",  "XAG");
ADD_NET_STORE_EXT(xmg, "x", "XMG");
ADD_NET_STORE(klut, "l", "LUT network");

ALICE_READ_FILE(klut_t, bench, filename, cmd) {
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut{klut};
  READ_NET(bench, filename, named_klut);
  return std::make_shared<klut_nt>(named_klut);
}

ALICE_READ_FILE(klut_t, blif, filename, cmd) {
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut{klut};
  READ_NET(blif, filename, named_klut);
  return std::make_shared<klut_nt>(named_klut);
}
ALICE_READ_FILE(aig_t, blif, filename, cmd) {
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut{klut};
  READ_NET(blif, filename, named_klut);
  mockturtle::aig_network network;
  aig_nt named_dest(network);
  mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
  mockturtle::node_resynthesis(named_dest, named_klut, resyn);
  return std::make_shared<aig_nt>(named_dest);
}
ALICE_READ_FILE(xag_t, blif, filename, cmd) {
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut{klut};
  READ_NET(blif, filename, named_klut);
  mockturtle::xag_network network;
  xag_nt named_dest(network);
  mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;
  mockturtle::node_resynthesis(named_dest, named_klut, resyn);
  return std::make_shared<xag_nt>(named_dest);
}
ALICE_READ_FILE(mig_t, blif, filename, cmd) {
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut{klut};
  READ_NET(blif, filename, named_klut);
  mockturtle::mig_network network;
  mig_nt named_dest(network);
  mockturtle::mig_npn_resynthesis resyn;
  mockturtle::node_resynthesis(named_dest, named_klut, resyn);
  return std::make_shared<mig_nt>(named_dest);
}
ALICE_READ_FILE(xmg_t, blif, filename, cmd) {
  mockturtle::klut_network klut;
  mockturtle::names_view<mockturtle::klut_network> named_klut{klut};
  READ_NET(blif, filename, named_klut);
  mockturtle::xmg_network network;
  xmg_nt named_dest(network);
  mockturtle::xmg_npn_resynthesis resyn;
  mockturtle::node_resynthesis(named_dest, named_klut, resyn);
  return std::make_shared<xmg_nt>(named_dest);
}

}   // namespace alice
