# Отчёт по лабораторной работе
# Повышение контраста

- Student: Кичанова Ксения Константиновна, group 3823Б1ФИ3
- Technology: SEQ | MPI
- Variant: 23

## 1. Introduction
Повышение контраста изображения — базовая операция в обработке изображений, направленная на увеличение разницы между яркостными характеристиками объектов. Данная задача относится к классу Data Parallel, так как независимые операции выполняются над разными частями данных (пикселями). Ожидается, что использование технологии MPI позволит добиться ускорения обработки за счет распределения вычислительной нагрузки между несколькими процессами, особенно для изображений большого размера.

## 2. Problem Statement
Необходимо применить алгоритм линейного контрастирования к RGB-изображению.
Input: Структура Image, содержащая одномерный массив пикселей (std::vector<uint8_t> pixels) и метаданные (ширина, высота, количество каналов). Для задачи используется 3 канала (RGB).
Output: Структура Image того же размера с увеличенным контрастом.

## 3. Baseline Algorithm (Sequential)
Базовый последовательный алгоритм состоит из двух проходов по всем пикселям изображения:
Проход 1: Поиск глобальных минимальных и максимальных значений для каждого из трёх цветовых каналов (R, G, B).
Проход 2: Применение формулы линейного контрастирования к каждому пикселю и каждому каналу с использованием найденных глобальных min и max.
Вычислительная сложность: O(N)

## 4. Parallelization Scheme
Изображение разделяется по горизонтальным строкам между доступными процессами MPI. Каждый процесс получает для обработки свой сегмент изображения. Каждый процесс находит минимальные и максимальные значения яркости для каждого канала (R, G, B) в своём сегменте. С помощью операций MPI_Allreduce с операциями MPI_MIN и MPI_MAX находятся глобальные минимумы и максимумы для каждого канала. Каждый процесс независимо применяет формулу линейного контрастирования ко всем пикселям своего сегмента, используя вычисленные глобальные значения.С помощью MPI_Allgatherv все процессы обмениваются обработанными сегментами, и каждый процесс получает полное итоговое изображение.

## 5. Implementation Details
- common.hpp - общие типы данных и структура Image для представления изображения
- ops_seq - последовательная реализация алгоритма
- ops_mpi - параллельная MPI-реализация алгоритма
- tests - functional для проверки корректности и performance для замера скорости.

## 6. Experimental Setup
- Аппаратное обеспечение: Intel® Core™ Ultra 5 225U × 14 (12 ядер, 14 логических процессоров, базовая скорость 1,5 ГГц)
- ОЗУ — 32 ГБ 
- Операционная система: Ubuntu 24.04.3 LTS
- Компилятор: g++
- Тип сборки: Release

## 7. Results and Discussion

### 7.1 Correctness
Корректность реализации проверялась набором функциональных тестов: маленькое однородное изображение (4×4 пикселя), изображение с градиентом (8×8 пикселей), краевой случай для MPI (1×3 пикселя), реальное изображение из файла.
Все 8 тестов (4 для MPI и 4 для последовательной версии) успешно пройдены. Результаты параллельной реализации полностью совпали с результатами последовательной.

### 7.2 Performance

pipeline:

| Mode        | Count | Time, s   | Speedup | Efficiency  |
|-------------|-------|-----------|---------|-------------|
| seq         | 1     | 0.14279   | 1.00    | N/A         |
| omp         | 2     | 0.06080   | 2.35    | 117.5%      |
| omp         | 4     | 0.08319   | 1.72    | 43.0%       |
| omp         | 8     | 0.06008   | 2.37    | 29.6%       |

task_run:

| Mode        | Count | Time, s   | Speedup | Efficiency  |
|-------------|-------|-----------|---------|-------------|
| seq         | 1     | 0.14272   | 1.00    | N/A         |
| omp         | 2     | 0.05705   | 2.50    | 125.0%      |
| omp         | 4     | 0.07922   | 1.80    | 45.0%       |
| omp         | 8     | 0.06228   | 2.29    | 28.6%       |


## 8. Conclusions
MPI реализация демонстрирует положительное ускорение на всех конфигурациях. Максимальное ускорение 2.50 достигнуто в режиме task_run на 2 процессах. На 4 процессах наблюдается снижение ускорения по сравнению с 2 процессами, что может быть связано с увеличением накладных расходов на коммуникацию. Эффективность параллелизации снижается с ростом числа процессов из-за накладных расходов MPI.

## 9. References
1. Лекции и практики курса "Параллельное программирование для кластерных систем"
2. Документация по MPI (стандарт MPI-3.1).
3. Лекции и практики курса "Компьюьтерная графика"

## Appendix
ops_seq.cpp:

bool KichanovaKIncreaseContrastSEQ::RunImpl() {
  const auto& input = GetInput();
  auto& output = GetOutput();
  
  const int width = input.width;
  const int height = input.height;
  const int channels = 3;
  const size_t total_pixels = width * height;
  
  uint8_t min_r = 255, max_r = 0;
  uint8_t min_g = 255, max_g = 0;
  uint8_t min_b = 255, max_b = 0;
  
  for (size_t i = 0; i < total_pixels; ++i) {
    size_t idx = i * channels;
    
    uint8_t r = input.pixels[idx];
    uint8_t g = input.pixels[idx + 1];
    uint8_t b = input.pixels[idx + 2];
    
    if (r < min_r) min_r = r;
    if (r > max_r) max_r = r;
    if (g < min_g) min_g = g;
    if (g > max_g) max_g = g;
    if (b < min_b) min_b = b;
    if (b > max_b) max_b = b;
  }
  
  float scale_r = 0.0f, scale_g = 0.0f, scale_b = 0.0f;
  
  if (max_r > min_r) {
    scale_r = 255.0f / (max_r - min_r);
  }
  
  if (max_g > min_g) {
    scale_g = 255.0f / (max_g - min_g);
  }
  
  if (max_b > min_b) {
    scale_b = 255.0f / (max_b - min_b);
  }
  
  for (size_t i = 0; i < total_pixels; ++i) {
    size_t idx = i * channels;
    
    uint8_t r = input.pixels[idx];
    uint8_t g = input.pixels[idx + 1];
    uint8_t b = input.pixels[idx + 2];
    
    if (max_r > min_r) {
      float new_r = (r - min_r) * scale_r;
      output.pixels[idx] = static_cast<uint8_t>(std::clamp(new_r, 0.0f, 255.0f));
    } else {
      output.pixels[idx] = r;
    }
    
    if (max_g > min_g) {
      float new_g = (g - min_g) * scale_g;
      output.pixels[idx + 1] = static_cast<uint8_t>(std::clamp(new_g, 0.0f, 255.0f));
    } else {
      output.pixels[idx + 1] = g;
    }
    
    if (max_b > min_b) {
      float new_b = (b - min_b) * scale_b;
      output.pixels[idx + 2] = static_cast<uint8_t>(std::clamp(new_b, 0.0f, 255.0f));
    } else {
      output.pixels[idx + 2] = b;
    }
  }
  
  return true;
}

ops_mpi.cpp:

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