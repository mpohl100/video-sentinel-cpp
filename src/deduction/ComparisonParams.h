#pragma once

namespace deduct {

struct ComparisonParams {
  ComparisonParams() = default;
  ComparisonParams(const ComparisonParams &) = default;
  ComparisonParams(ComparisonParams &&) = default;
  ComparisonParams &operator=(const ComparisonParams &) = default;
  ComparisonParams &operator=(ComparisonParams &&) = default;
  ComparisonParams(double tolerance, bool only_outer_form, bool use_forms)
      : tolerance{tolerance},
        only_outer_form{only_outer_form}, use_forms{use_forms} {}
  double tolerance = 0.1;
  bool only_outer_form = false;
  bool use_forms = false;
};

} // namespace deduct
