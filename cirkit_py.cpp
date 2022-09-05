#include "cirkit.h"
#include "alice/detail/python.hpp"

using namespace alice;

using cli_base = cli<aig_t, mig_t, xag_t, xmg_t, klut_t>;
class cirkit_cli: public cli_base {
    nlohmann::json last_result_;
    // a simplified version of cli::execute_line
    bool execute_line(const std::string& line, bool save_result);

public:
    cirkit_cli(): cli_base("cirkit")
    { process_file("cirkit.rc", false, false); }
    py::object run_cmd(const std::string& cmd);
};

bool cirkit_cli::execute_line(const std::string& line, bool save_result) {
    if (line.empty() || line[0] == '#') return false;

    /* split commands if line contains a semi-colon */
    const auto lines = detail::split_with_quotes<';'>(line);
    /* if more than one command is detected recurse on each part */
    if (lines.size() > 1u) {
        auto i = 0u;
        for (const auto& cline : lines)
            if (!execute_line(preprocess_alias(cline), ++i >= lines.size())) return false;
        return true;
    }

    last_result_.clear();
    auto vline = detail::split_with_quotes<' '>(line);
    const auto& vcmd = vline.front();
    if (!env->commands().count(vcmd)) {
        env->err() << "[e] unknown command: " << vcmd << std::endl;
        return false;
    }
    auto& cmd = env->commands().at(vcmd);
    auto result  = cmd->run(vline);
    if (result && save_result) last_result_ = cmd->log();
    return result;
}

using detail::return_value_dict;
py::object cirkit_cli::run_cmd(const std::string& cmd) {
    execute_line(cmd, true);
    return last_result_.is_object() ?
             py::cast(return_value_dict(last_result_)) :
             py::none();
}

_ALICE_END_LIST(alice_stores)
_ALICE_END_LIST(alice_commands)
_ALICE_END_LIST(alice_read_tags)
_ALICE_END_LIST(alice_write_tags)

cirkit_cli ckt_cli{};
insert_read_commands<cirkit_cli, alice_read_tags, std::tuple_size<alice_read_tags>::value> irc(ckt_cli);
insert_write_commands<cirkit_cli, alice_write_tags, std::tuple_size<alice_write_tags>::value> iwc(ckt_cli);
insert_commands<cirkit_cli, alice_commands, std::tuple_size<alice_commands>::value> ic(ckt_cli);

PYBIND11_MODULE(cirkit_py, m) {
    m.doc() = "cirkit Python bindings";
    m.def("run", [](const std::string& cmd) { return ckt_cli.run_cmd(cmd); });

    py::class_<return_value_dict>(m, "ReturnValueDict")
        .def("__getitem__", &return_value_dict::__getitem__)
        .def("__repr__",    &return_value_dict::__repr__   )
        .def("_repr_html_", &return_value_dict::_repr_html_)
        .def("dict",        &return_value_dict::dict       );
}
