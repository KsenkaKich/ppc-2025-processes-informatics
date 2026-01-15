#include "kichanova_k_increase_contrast/mpi/include/ops_mpi.hpp"

#include <mpi.h>
#include <algorithm>
#include <cstdint>
#include <vector>

#include "kichanova_k_increase_contrast/common/include/common.hpp"

namespace kichanova_k_increase_contrast {

KichanovaKIncreaseContrastMPI::KichanovaKIncreaseContrastMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput().width = in.width;
  GetOutput().height = in.height;
  GetOutput().channels = in.channels;
  GetOutput().pixels.resize(in.pixels.size());
}

bool KichanovaKIncreaseContrastMPI::ValidationImpl() {
  return (GetInput().width > 0) && 
         (GetInput().height > 0) && 
         (GetInput().channels == 3) &&
         (GetInput().pixels.size() == static_cast<size_t>(GetInput().width * GetInput().height * GetInput().channels));
}

bool KichanovaKIncreaseContrastMPI::PreProcessingImpl() {
  return true;
}

bool KichanovaKIncreaseContrastMPI::RunImpl() {
  const auto& input = GetInput();
  auto& output = GetOutput();
  
  const int width = input.width;
  const int height = input.height;
  const int channels = 3;
  
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  int rows_per_process = height / size;
  int remainder = height % size;
  
  int start_row = rank * rows_per_process + std::min(rank, remainder);
  int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);
  int local_rows = end_row - start_row;
  
  uint8_t local_min_r = 255, local_max_r = 0;
  uint8_t local_min_g = 255, local_max_g = 0;
  uint8_t local_min_b = 255, local_max_b = 0;
  
  for (int row = start_row; row < end_row; ++row) {
    for (int col = 0; col < width; ++col) {
      size_t idx = (row * width + col) * channels;
      
      uint8_t r = input.pixels[idx];
      uint8_t g = input.pixels[idx + 1];
      uint8_t b = input.pixels[idx + 2];
      
      if (r < local_min_r) local_min_r = r;
      if (r > local_max_r) local_max_r = r;
      if (g < local_min_g) local_min_g = g;
      if (g > local_max_g) local_max_g = g;
      if (b < local_min_b) local_min_b = b;
      if (b > local_max_b) local_max_b = b;
    }
  }
  
  uint8_t global_min_r, global_max_r;
  uint8_t global_min_g, global_max_g;
  uint8_t global_min_b, global_max_b;
  
  MPI_Allreduce(&local_min_r, &global_min_r, 1, MPI_UINT8_T, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce(&local_max_r, &global_max_r, 1, MPI_UINT8_T, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce(&local_min_g, &global_min_g, 1, MPI_UINT8_T, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce(&local_max_g, &global_max_g, 1, MPI_UINT8_T, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce(&local_min_b, &global_min_b, 1, MPI_UINT8_T, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce(&local_max_b, &global_max_b, 1, MPI_UINT8_T, MPI_MAX, MPI_COMM_WORLD);
  
  float scale_r = 0.0f, scale_g = 0.0f, scale_b = 0.0f;
  
  if (global_max_r > global_min_r) {
    scale_r = 255.0f / (global_max_r - global_min_r);
  }
  
  if (global_max_g > global_min_g) {
    scale_g = 255.0f / (global_max_g - global_min_g);
  }
  
  if (global_max_b > global_min_b) {
    scale_b = 255.0f / (global_max_b - global_min_b);
  }
  
  std::vector<uint8_t> local_output(local_rows * width * channels);
  
  for (int i = 0; i < local_rows; ++i) {
    int global_row = start_row + i;
    for (int col = 0; col < width; ++col) {
      size_t input_idx = (global_row * width + col) * channels;
      size_t output_idx = (i * width + col) * channels;
      
      uint8_t r = input.pixels[input_idx];
      uint8_t g = input.pixels[input_idx + 1];
      uint8_t b = input.pixels[input_idx + 2];
      
      if (global_max_r > global_min_r) {
        float new_r = (r - global_min_r) * scale_r;
        local_output[output_idx] = static_cast<uint8_t>(std::clamp(new_r, 0.0f, 255.0f));
      } else {
        local_output[output_idx] = r;
      }
      
      if (global_max_g > global_min_g) {
        float new_g = (g - global_min_g) * scale_g;
        local_output[output_idx + 1] = static_cast<uint8_t>(std::clamp(new_g, 0.0f, 255.0f));
      } else {
        local_output[output_idx + 1] = g;
      }
      
      if (global_max_b > global_min_b) {
        float new_b = (b - global_min_b) * scale_b;
        local_output[output_idx + 2] = static_cast<uint8_t>(std::clamp(new_b, 0.0f, 255.0f));
      } else {
        local_output[output_idx + 2] = b;
      }
    }
  }
  
  std::vector<int> recv_counts(size);
  std::vector<int> displs(size);
  
  int row_size = width * channels;
  
  for (int i = 0; i < size; ++i) {
    int i_start_row = i * rows_per_process + std::min(i, remainder);
    int i_end_row = i_start_row + rows_per_process + (i < remainder ? 1 : 0);
    int i_rows = i_end_row - i_start_row;
    
    recv_counts[i] = i_rows * row_size;
    displs[i] = i_start_row * row_size;
  }
  
  MPI_Allgatherv(local_output.data(), local_rows * row_size, MPI_UINT8_T, output.pixels.data(),
               recv_counts.data(), displs.data(), MPI_UINT8_T, MPI_COMM_WORLD);
  return true;
}

bool KichanovaKIncreaseContrastMPI::PostProcessingImpl() {
  return true;
}

}  // namespace kichanova_k_increase_contrast