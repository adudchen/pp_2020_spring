// Copyright 2020 Golubev Vladislav

#include <omp.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <utility>
#include <random>
#include "../../../modules/task_2/golubev_v_graham_scan/golubev_v_graham_scan.h"

std::vector<std::pair<double, double> > get_rand_set(std::size_t size) {
  std::mt19937 gen;

  std::vector<std::pair<double, double> > result(size);
  for (std::size_t i = 0; i < size; ++i) {
    result[i] = std::make_pair(gen() % 1000, gen() % 1000);
  }
  return result;
}
std::vector<std::pair<double, double> > generate_points(std::size_t size) {
  std::vector<std::pair<double, double> > result(size);
  result[0] = std::make_pair(0, 0);
  for (std::size_t i = 1; i < size; ++i) {
    result[i] = std::make_pair(i, 10);
  }

  return result;
}

double omp_get_polar_r(const std::pair<double, double>& point) {
  return std::sqrt(point.second * point.second + point.first * point.first);
}

double omp_get_polar_grad(const std::pair<double, double>& point) {
  return std::atan(point.second / point.first);
}

double omp_get_det(const std::pair<double, double>& x,
  const std::pair<double, double>& y, const std::pair<double, double>& z) {
  return (y.first - x.first) * (z.second - x.second) - (z.first - x.first) * (y.second - x.second);
}

std::size_t omp_get_lex_min(std::vector<std::pair<double, double> > v, int num_threads) {
  std::vector<std::size_t> res(num_threads);

#pragma omp parallel num_threads(num_threads)
  {
    int n_thread = omp_get_thread_num();
    int size = v.size();
#pragma omp for
    for (int i = 1; i < size; ++i) {
      if (v[res[n_thread]] > v[i]) {
        res[n_thread] = i;
      }
    }
  }
  std::size_t min_idx = res[0];
  for (std::size_t i = 1; i < res.size(); ++i) {
    if (v[min_idx] > v[res[i]]) {
      min_idx = res[i];
    }
  }
  return min_idx;
}

void mp_sort(std::vector<std::pair<double, double> >::iterator begin,
            std::vector<std::pair<double, double> >::iterator end, int num_threads) {
  int st = std::log2(num_threads);
  num_threads = std::pow(2, st);

#pragma omp parallel num_threads(num_threads)
  {
    int n_thread = omp_get_thread_num();
    int step = (end - begin) / num_threads;

    auto left = begin + n_thread * step;
    auto right = left + step;

    std::sort(left, right);


    int log = std::log2(num_threads);
    int h = 2;
    int sorted_size;

    while (log != 0) {
      sorted_size = step;
      step += step;

#pragma omp barrier

      if (n_thread < num_threads / h) {
        merge(begin + step * n_thread,
          begin + n_thread * step + sorted_size,
          begin + n_thread * step + step);
      }
      --log;
      h *= 2;
    }
  }

  int ostatok = (end - begin) % num_threads;
  if (ostatok != 0) {
    std::sort(begin + (end - begin) / num_threads * num_threads, end);
    merge(begin, begin + (end - begin) / num_threads * num_threads, end);
  }
}

void merge(std::vector<std::pair<double, double> >::iterator left,
  std::vector<std::pair<double, double> >::iterator mid,
  std::vector<std::pair<double, double> >::iterator right) {
  auto lidx = left;
  auto ridx = mid;
  std::size_t idx = 0;
  std::vector<std::pair<double, double> > tmp(right - left);

  while (lidx != mid || ridx != right) {
    if (*lidx < *ridx) {
      tmp[idx++] = std::move(*lidx);
      lidx++;
    } else {
      tmp[idx++] = std::move(*ridx);
      ridx++;
    }

    if (lidx == mid) {
      std::copy(std::make_move_iterator(ridx),
        std::make_move_iterator(right),
        &tmp[idx]);
      break;
    }

    if (ridx == right) {
      std::copy(std::make_move_iterator(lidx),
        std::make_move_iterator(mid),
        &tmp[idx]);
      break;
    }
  }
  std::copy(std::make_move_iterator(tmp.begin()),
    std::make_move_iterator(tmp.end()), left);
}


/*void omp_sort(std::vector<double>::iterator first, std::vector<double>::iterator last) {
  auto i = first;
  auto j = last - 1;
  double pivot = *(first + (last - first) / 2);

  do {
    while (*i < pivot)
      i++;
    while (*j > pivot)
      j--;
    if (i <= j) {
      if (i < j) {
        std::swap(*i, *j);
      }
      i++;
      j--;
    }
  } while (i <= j);

  if (i < last)
    omp_sort(i, last);
  if (first < j)
    omp_sort(first, j + 1);
}*/
