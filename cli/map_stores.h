#include <alice/alice.hpp>

#include <lorina/genlib.hpp>
#include <mockturtle/io/genlib_reader.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/views/binding_view.hpp>
#include <mockturtle/utils/tech_library.hpp>


namespace alice {
/////////////////////////////////////////
// technology library

ALICE_ADD_FILE_TYPE_READ_ONLY(genlib, "GENLIB");

using tlib_nt = mockturtle::tech_library<>;
using tlib_t = std::shared_ptr<tlib_nt>;
ALICE_ADD_STORE(tlib_t, "tlib", "", "technology library", "technology libraries");

ALICE_DESCRIBE_STORE(tlib_t, tlib)
{  return fmt::format("max_gate_size = {}   gates = {}", tlib->max_gate_size(), tlib->get_gates().size()); }

ALICE_PRINT_STORE_STATISTICS(tlib_t, os, tlib) {
    auto [inv_area, inv_delay, inv_id] = tlib->get_inverter_info();
    auto [buf_area, buf_delay, buf_id] = tlib->get_buffer_info();
    os << fmt::format("max_gate_size = {}   gates = {}   inverter_info = ({}, {})   buffer_info = ({}, {})\n",
                    tlib->max_gate_size(), tlib->get_gates().size(), inv_area, inv_delay, buf_area, buf_delay);
}

ALICE_LOG_STORE_STATISTICS(tlib_t, tlib) {
    auto [inv_area, inv_delay, inv_id] = tlib->get_inverter_info();
    auto [buf_area, buf_delay, buf_id] = tlib->get_buffer_info();
    return {
        {"max_gate_size", tlib->max_gate_size()},
        {"gates",         tlib->get_gates().size()},
        {"inv_area",      inv_area},
        {"inv_delay",     inv_delay},
        {"buf_area",      buf_area},
        {"buf_delay",     buf_delay}
    };
}

ALICE_READ_FILE(tlib_t, genlib, filename, cmd) {
    std::vector<mockturtle::gate> gates;
    if (lorina::read_genlib(filename, mockturtle::genlib_reader(gates)) != lorina::return_code::success)
        std::cout << "[w] parse error\n";
    return std::make_shared<tlib_nt>(gates);
}

/////////////////////////////////////////
// mapped network

using mapped_nt = mockturtle::binding_view<mockturtle::klut_network>;
using mapped_t = std::shared_ptr<mapped_nt>;
ALICE_ADD_STORE(mapped_t, "mapped", "", "mapped network", "mapped networks");

ALICE_DESCRIBE_STORE(mapped_t, mapped)
{   return fmt::format("area = {}   delay = {}", mapped->compute_area(), mapped->compute_worst_delay()); }

ALICE_PRINT_STORE_STATISTICS(mapped_t, os, mapped)
{   os << fmt::format("area = {}   delay = {}\n", mapped->compute_area(), mapped->compute_worst_delay()); }

ALICE_LOG_STORE_STATISTICS(mapped_t, mapped) {
    return {
        {"area",  mapped->compute_area()},
        {"delay", mapped->compute_worst_delay()}
    };
}

// requires mockturtle 0.4
//ALICE_WRITE_FILE(mapped_t, verilog, mapped, filename, cmd)
//{ mockturtle::write_verilog_with_binding(*mapped, filename); }

}   // namespace alice
