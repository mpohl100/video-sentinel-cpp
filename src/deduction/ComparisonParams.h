#pragma once

namespace deduct {

struct ComparisonParams{
    ComparisonParams() = default;
    ComparisonParams(const ComparisonParams &) = default;
    ComparisonParams(ComparisonParams &&) = default;
    ComparisonParams &operator=(const ComparisonParams &) = default;
    ComparisonParams &operator=(ComparisonParams &&) = default;
    ComparisonParams(double tolerance, bool only_outer_form)
        : tolerance{tolerance}, only_outer_form{only_outer_form} {}
    double tolerance = 0.1;
    bool only_outer_form = false;
};

} // namespace deduct
