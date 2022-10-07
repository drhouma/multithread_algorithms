/**
 * @file    winograd.cpp
 * @authors telvina<telvina<at>student.21-school.ru>
 * @authors sreanna<sreanna<at>student.21-school.ru>
 * @authors jmadie<jmadie<at>student.21-school.ru>
 * @date    02/10/22
 */

#include "winograd.h"

#include <functional>

namespace s21 {

auto Winograd::run(RunType type) -> Matrix<double> {
  Matrix<double> result{m_first.rows(), m_second.cols()};
  preprocessing();

  switch (type) {
    case RunType::SINGLE: {
      runOneThread(result);
      break;
    }
    case RunType::MULTITHREADING_CLASSIC: {
      runMultiplyThreads(result);
      break;
    }
    case RunType::MULTITHREADING_NUMBER: {
      runMultiplyThreads(result, m_threadNumber);
      break;
    }
    case RunType::MULTITHREADING_CONVEYOR: {
      runtMultiplyThreadsConveyor(result);
      break;
    }
    default:
      throw std::invalid_argument{"Invalid type of run"};
  }

  return result;
}

auto Winograd::runOneThread(Matrix<double>& result) -> void {
  for (int i{0}; i < m_first.rows(); ++i) {
    calculateRowFactor(i);
  }
  for (int i{0}; i < m_second.cols(); ++i) {
    calculateColFactor(i);
  }
  calculateResultMatrix(result);
  oddMatrixProcessing(result);
}

auto Winograd::runMultiplyThreads(Matrix<double>& result,
                                  std::uint32_t threadNumber) -> void {
  m_threads.resize(2);

  m_threads[0] = std::move(std::thread{[this]() {
    for (int i{0}; i < m_first.rows(); ++i) {
      calculateRowFactor(i);
    }
  }});
  m_threads[1] = std::move(std::thread{[this]() {
    for (int i{0}; i < m_second.cols(); ++i) {
      calculateColFactor(i);
    }
  }});

  m_threads[0].join();
  m_threads[1].join();

  m_threads.resize(threadNumber);

  auto job = [this, &result](int i) { calculateRowValues(result, i); };

  for (int i{0}; i < m_first.rows(); ++i) {
    if (m_threads[i % m_threads.size()].joinable()) {
      m_threads[i % m_threads.size()].join();
    }
    m_threads[i % m_threads.size()] = std::move(std::thread{job, i});
  }
  for (auto& thread : m_threads) {
    if (thread.joinable()) thread.join();
  }
  oddMatrixProcessing(result);
}

auto Winograd::runtMultiplyThreadsConveyor(Matrix<double>& result) -> void {
  m_threads.resize(3);

  auto job1 = [this](int i) { calculateRowFactor(i); };
  auto job2 = [this](int i) { calculateColFactor(i); };
  auto job3 = [this, &result](int i) { calculateRowValues(result, i); };

  for (int i{0}; i < m_first.rows(); ++i) {
    m_threads[0] = std::move(std::thread{job1, i});
    if (i < m_second.cols()) {
      m_threads[1] = std::move(std::thread{job2, i});
    }
    m_threads[2] = std::move(std::thread{job3, i});
    for (auto& thread : m_threads) {
      if (thread.joinable()) thread.join();
    }
  }
  oddMatrixProcessing(result);
}

auto Winograd::preprocessing() noexcept -> void {
  m_halfSize = m_first.cols() / 2;
  m_rowFactor.clear();
  m_colFactor.clear();
  m_rowFactor.resize(static_cast<size_t>(m_first.rows()));
  m_colFactor.resize(static_cast<size_t>(m_second.cols()));
}

auto Winograd::setMatrices(const Matrix<double>& first,
                           const Matrix<double>& second) -> void {
  if (first.cols() != second.rows()) {
    throw std::invalid_argument{"Cols from 1st matrix != rows from 2nd matrix"};
  }
  if (m_first != first) {
    m_first = first;
  }
  if (m_second != second) {
    m_second = second;
  }
}

auto Winograd::calculateRowFactor(const int i) -> void {
  m_rowFactor[i] = m_first(i, 0) * m_first(i, 1);
  for (int j{1}; j < m_halfSize; ++j)
    m_rowFactor[i] += m_first(i, 2 * j) * m_first(i, 2 * j + 1);
}

auto Winograd::calculateColFactor(const int i) -> void {
  m_colFactor[i] = m_second(0, i) * m_second(1, i);
  for (int j{1}; j < m_halfSize; ++j)
    m_colFactor[i] += m_second(2 * j, i) * m_second(2 * j + 1, i);
}

auto Winograd::calculateResultMatrix(Matrix<double>& result) -> void {
  for (int i{0}; i < m_first.rows(); ++i) {
    calculateRowValues(result, i);
  }
}

auto Winograd::calculateRowValues(Matrix<double>& result, const int i) -> void {
  for (int j{0}; j < m_second.cols(); ++j) {
    result(i, j) = -m_rowFactor[i] - m_colFactor[j];
    for (int k{0}; k < m_halfSize; ++k) {
      result(i, j) += (m_first(i, 2 * k) + m_second(2 * k + 1, j)) *
                      (m_first(i, 2 * k + 1) + m_second(2 * k, j));
    }
  }
}

auto Winograd::oddMatrixProcessing(Matrix<double>& result) -> void {
  if (m_halfSize % 2) {
    for (int i{0}; i < m_first.rows(); ++i) {
      for (int j{0}; j < m_second.cols(); ++j) {
        result(i, j) +=
            m_first(i, m_first.cols() - 1) * m_second(m_first.cols() - 1, j);
      }
    }
  }
}

auto Winograd::setThreadsNumber(const std::uint32_t number) -> void {
  if (number != m_threadNumber) {
    m_threadNumber = number;
  }
}

};  // namespace s21