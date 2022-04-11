#pragma once

#include "ContestIO.h"

class ContestCalculate
{
public:
  ContestCalculate()
  {
    // 处理输入
    io.handle_contest_input();
    // 对于所有时间点，复制一份边缘节点带宽，下面作为剩余带宽的记录
    sb_alltime.resize(io.demand.size(), io.site_bandwidth);
  }

  void brute_force()
  {

    return;
  }

  /* 计分函数 */
  int calculate_94_score()
  {
    int pos_94 = 0.05 * res.size();
    std::vector<int> edge_score_94(io.edge_names.size());
    std::vector<std::vector<std::pair<int, int>>> edge_sort_score(
        io.edge_names.size(), std::vector<std::pair<int, int>>(res.size()));

    for (int time = 0; time < res.size(); time++)
    {
      for (int edge_idx = 0; edge_idx < io.edge_names.size(); edge_idx++)
      {
        int edge_dist = io.site_bandwidth[edge_idx] - sb_alltime[time][edge_idx];
        edge_sort_score[edge_idx][time] = {edge_dist, time};
      }
    }
    double sum = 0;
    for (int edge_idx = 0; edge_idx < io.edge_names.size(); edge_idx++)
    {
      auto edge_score = edge_sort_score[edge_idx];
      std::sort(edge_score.begin(), edge_score.end(), std::greater<std::pair<int, int>>());
      edge_score_94[edge_idx] = edge_score[pos_94].first;
      if (edge_score_94[edge_idx] == 0)
      {
        sum += 0;
      }
      else if (edge_score_94[edge_idx] < io.base_cost)
      {
        sum += io.base_cost;
      }
      else
      {
        int over = edge_score_94[edge_idx] - io.base_cost;
        sum += io.base_cost;
        sum += double(over * over) / io.site_bandwidth[edge_idx];
      }
      sum += 0.5;
      // sum += edge_score_94[edge_idx];
      // std::cout << "edge_idx: " << edge_idx << " score: " << edge_score[pos_94].first << "
      // time: " << edge_score[pos_94].second << std::endl;
    }

    return sum;
  }

  /* 处理输出 */
  void handle_output()
  {
    io.handle_output(res);
  }

public:
  ContestIO io;
  std::vector<std::vector<int>> sb_alltime;
  // 时间 / 客户端 / 边缘节点 / 对当前客户端那些流进行了分配
  std::vector<std::vector<std::vector<std::vector<std::string>>>> res;
};
