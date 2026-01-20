#pragma once

#include "kichanova_k_shellsort_batcher_oddeven_merge/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kichanova_k_shellsort_batcher_oddeven_merge {

class KichanovaKShellsortBatcherOddEvenMergeSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }

  explicit KichanovaKShellsortBatcherOddEvenMergeSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void ShellSort(std::vector<int> &arr);
  void OddEvenBatcherMerge(const std::vector<int> &left, const std::vector<int> &right, std::vector<int> &merged);
};

}  // namespace kichanova_k_shellsort_batcher_oddeven_merge
