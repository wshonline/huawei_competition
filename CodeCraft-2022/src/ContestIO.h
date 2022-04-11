#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_set>

class ContestIO
{
public:
    void read_csv(std::ifstream &fp,
                  std::vector<std::vector<std::string>> &output)
    {
        std::string line;
        int cur_row = -1;
        while (getline(fp, line))
        {
            cur_row++;
            if (line.size() && line[line.size() - 1] == '\r')
            {
                line = line.substr(0, line.size() - 1);
            }
            std::string elem;
            std::istringstream readstr(line);
            //将一行数据按'，'分割
            std::vector<std::string> row;
            while (getline(readstr, elem, ','))
            {
                row.push_back(elem);
            }
            output.emplace_back(std::move(row));
        }
    }

    void handle_dm(std::vector<std::vector<std::string>> &csv_demand,
                   std::vector<std::vector<std::vector<std::pair<int, std::string>>>> demand)
    {
        size_t csv_row_size = csv_demand.size();
        if (csv_row_size < 1)
            return;
        size_t csv_col_size = csv_demand[0].size();
        size_t cli_size = csv_col_size - 2;

        for (size_t i = 1; i < csv_row_size; i++)
        {
            std::string time = csv_demand[i][0];
            std::vector<std::vector<std::pair<int, std::string>>> row(cli_size); // 客户节点，<需求量，流名称>
            for (; i < csv_row_size && csv_demand[i][0] == time; i++)
            {
                std::string stream_id = csv_demand[i][1];
                for (size_t j = 0; j < cli_size; j++)
                {
                    row[j].push_back({stoi(csv_demand[i][j + 2]), stream_id});
                }
            }
            demand.emplace_back(std::move(row));
        }
    }

    void handle_qos_filter(std::vector<std::vector<std::string>> &csv_qos,
                           std::vector<int> &edge_dist_num,
                           int qos_constrain, std::vector<std::vector<int>> &qos_bitmap)
    {
        size_t csv_row_size = csv_qos.size();
        if (csv_row_size < 1)
            return;
        size_t csv_col_size = csv_qos[0].size();

        edge_dist_num.resize(csv_row_size - 1, 0);
        qos_bitmap.resize(csv_row_size - 1); // 位图初始化为0
        edge_dist_clients.resize(csv_row_size - 1);

        for (size_t i = 1; i < csv_row_size; i++)
        {
            for (size_t j = 1; j < csv_col_size; j++)
            {
                if (stoi(csv_qos[i][j]) < qos_constrain)
                {
                    edge_dist_num[i - 1]++;
                    qos_bitmap[i - 1][j - 1] = 1;
                    edge_dist_clients[i].push_back(j - 1);
                }
            }
        }
    }

    void handle_sb(std::vector<std::vector<std::string>> &csv_sb,
                   std::vector<int> &site_bandwidth)
    {
        size_t csv_row_size = csv_sb.size();
        for (size_t i = 1; i < csv_row_size; i++)
        {
            site_bandwidth.push_back(stoi(csv_sb[i][1]));
        }
    }

    void handle_names(std::vector<std::vector<std::string>> &csv_qos,
                      std::vector<std::string> &client_names,
                      std::vector<std::string> &edge_names)
    {
        size_t csv_row_size = csv_qos.size();
        if (csv_row_size < 1)
            return;
        size_t csv_col_size = csv_qos[0].size();

        edge_names.reserve(csv_row_size - 1);
        client_names.reserve(csv_col_size - 1);

        for (size_t i = 1; i < csv_row_size; i++)
        {
            edge_names.push_back(csv_qos[i][0]);
        }
        for (size_t i = 1; i < csv_col_size; i++)
        {
            client_names.push_back(csv_qos[0][i]);
        }
    }

    void handle_output(std::vector<std::vector<std::vector<std::vector<std::string>>>> &res)
    {
#ifdef DEBUG
        std::ofstream fout("./output/solution.txt", std::ios_base::trunc);
#else
        std::ofstream fout("/output/solution.txt", std::ios_base::trunc);
#endif
        for (size_t time = 0; time < res.size(); time++)
        {
            auto &output_time = res[time];
            for (size_t cli_idx = 0; cli_idx < output_time.size(); cli_idx++)
            {
                fout << client_names[cli_idx] << ":";
                bool is_first = true;
                for (size_t edge_idx = 0; edge_idx < output_time[cli_idx].size(); edge_idx++)
                {
                    if (output_time[cli_idx][edge_idx].size() > 0)
                    {
                        if (!is_first)
                            fout << ",";
                        else
                            is_first = false;
                        fout << "<" << edge_names[edge_idx];
                        for (int i = 0; i < output_time[cli_idx][edge_idx].size(); i++)
                        {
                            fout << ",";
                            fout << output_time[cli_idx][edge_idx][i];
                        }
                        fout << ">";
                    }
                }
                fout << std::endl;
            }
        }
        fout.close();
        return;
    }

    void handle_contest_input()
    {
#ifdef DEBUG
        std::ifstream fp_cfg("./data/config.ini");
        std::ifstream fp_dm("./data/demand.csv");
        std::ifstream fp_qos("./data/qos.csv");
        std::ifstream fp_sb("./data/site_bandwidth.csv");
#else
        std::ifstream fp_cfg("/data/config.ini");
        std::ifstream fp_dm("/data/demand.csv");
        std::ifstream fp_qos("/data/qos.csv");
        std::ifstream fp_sb("/data/site_bandwidth.csv");
#endif

        // config
        std::string cfg_line, elem;
        getline(fp_cfg, cfg_line);
        getline(fp_cfg, cfg_line); // 获取第二行
        std::istringstream cfg_line_iss(cfg_line);
        getline(cfg_line_iss, elem, '=');
        getline(cfg_line_iss, elem, '='); // 获取qos_constrain=v的v
        qos_constrain = atoi(elem.c_str());
        std::cout << "qos_constrain=" << qos_constrain << std::endl;

        // demand
        std::vector<std::vector<std::string>> csv_demand;
        read_csv(fp_dm, csv_demand);
        handle_dm(csv_demand, demand);

        // qos
        std::vector<std::vector<std::string>> csv_qos;
        read_csv(fp_qos, csv_qos);
        handle_qos_filter(csv_qos, edge_dist_num, qos_constrain, qos_bitmap);

        // names
        handle_names(csv_qos, client_names, edge_names);

        // site_bandwidth
        std::vector<std::vector<std::string>> csv_sb;
        read_csv(fp_sb, csv_sb);
        handle_sb(csv_sb, site_bandwidth);
    }

    int qos_constrain; // qos限制
    int base_cost;
    std::vector<std::string> client_names;                                     // 客户节点名字
    std::vector<std::string> edge_names;                                       // 边缘节点名字
    std::vector<std::vector<std::vector<std::pair<int, std::string>>>> demand; // 带宽需求。外代表时间，中代表客户节点，内代表<需求量，流名称>
    std::vector<std::vector<int>> qos_bitmap;                                  // 1表示满足qos，每行:client_idx，每列:edge_idx
    std::vector<int> edge_dist_num;                                            // 每个edge可以分发的节点数
    std::vector<std::vector<int>> edge_dist_clients;                           // 每个edge可以分发的客户节点
    std::vector<int> site_bandwidth;                                           // 边缘节点带宽
};