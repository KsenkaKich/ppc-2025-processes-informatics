#pragma once

#include "task/include/task.hpp"
#include "kichanova_k_shellsort_batcher_oddeven_merge/common/include/common.hpp"

namespace kichanova_k_shellsort_batcher_oddeven_merge {

class KichanovaKShellsortBatcherOddEvenMergeMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  
  explicit KichanovaKShellsortBatcherOddEvenMergeMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  
  void ShellSort(std::vector<int> &arr);
  void ExchangeAndMerge(std::vector<int>& local_data, int partner, int rank, int tag);
};

}  // namespace kichanova_k_shellsort_batcher_oddeven_merge