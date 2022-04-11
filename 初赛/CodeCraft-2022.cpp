#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

// 提交的时候要注释掉这个宏
// #define DEBUG

void read_csv(std::ifstream& fp,
              std::vector<std::vector<std::string>>& output) {
    std::string line;
    int cur_row = -1;
    while (getline(fp, line)) {
        cur_row++;
        if (line.size() && line[line.size() - 1] == '\r') {
            line = line.substr(0, line.size() - 1);
        }
        std::string elem;
        std::istringstream readstr(line);
        //将一行数据按'，'分割
        std::vector<std::string> row;
        while (getline(readstr, elem, ',')) {
            row.push_back(elem);
        }
        output.emplace_back(std::move(row));
    }
}

void handle_dm(std::vector<std::vector<std::string>>& csv_demand,
               std::vector<std::vector<int>>& demand) {
    size_t csv_row_size = csv_demand.size();
    if (csv_row_size < 1) return;
    size_t csv_col_size = csv_demand[0].size();

    for (size_t i = 1; i < csv_row_size; i++) {
        std::vector<int> row;
        for (size_t j = 1; j < csv_col_size; j++) {
            row.push_back(stoi(csv_demand[i][j]));
        }
        demand.emplace_back(std::move(row));
    }
}

void handle_qos_filter(std::vector<std::vector<std::string>>& csv_qos,
                       std::vector<std::vector<int>>& qos,
                       std::vector<int>& edge_dist_num,
                       int qos_constrain, std::vector<std::vector<int>>& qos_bitmap) {
    size_t csv_row_size = csv_qos.size();
    if (csv_row_size < 1) return;
    size_t csv_col_size = csv_qos[0].size();

    // qos.reserve(csv_row_size - 1);
    // edge_dist_num.resize(csv_row_size - 1, 0);
    // 位图初始化为0
    qos_bitmap.resize(csv_row_size - 1, std::vector<int>(csv_col_size - 1, 0));

    for (size_t i = 1; i < csv_row_size; i++) {
        // std::vector<int> row;
        for (size_t j = 1; j < csv_col_size; j++) {
            if (stoi(csv_qos[i][j]) < qos_constrain) {
                // row.push_back(j-1);
                // edge_dist_num[i-1]++;
                qos_bitmap[i - 1][j - 1] = 1;
            }
        }
        // qos.emplace_back(std::move(row));
    }
}

void handle_sb(std::vector<std::vector<std::string>>& csv_sb,
               std::vector<int>& site_bandwidth) {
    size_t csv_row_size = csv_sb.size();
    for (size_t i = 1; i < csv_row_size; i++) {
        site_bandwidth.push_back(stoi(csv_sb[i][1]));
    }
}

void handle_names(std::vector<std::vector<std::string>>& csv_qos,
                  std::vector<std::string> &client_names,
                  std::vector<std::string> &edge_names) {
    size_t csv_row_size = csv_qos.size();
    if (csv_row_size < 1) return;
    size_t csv_col_size = csv_qos[0].size();

    edge_names.reserve(csv_row_size - 1);
    client_names.reserve(csv_col_size - 1);

    for (size_t i = 1; i < csv_row_size; i++) {
        edge_names.push_back(csv_qos[i][0]);
    }
    for (size_t i = 1; i < csv_col_size; i++) {
        client_names.push_back(csv_qos[0][i]);
    }
}

void handle_input(std::vector<std::string>& client_names,
                  std::vector<std::string>& edge_names,
                  std::vector<std::vector<int>>& demand,
                  std::vector<std::vector<int>>& qos,
                  std::vector<int>& edge_dist_num,
                  std::vector<int>& site_bandwidth,
                  int& qos_constrain, std::vector<std::vector<int>>& qos_bitmap) {
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
    getline(fp_cfg, cfg_line);  // 获取第二行
    std::istringstream cfg_line_iss(cfg_line);
    getline(cfg_line_iss, elem, '=');
    getline(cfg_line_iss, elem, '=');  // 获取qos_constrain=v的v
    qos_constrain = atoi(elem.c_str());
    std::cout << "qos_constrain=" << qos_constrain << std::endl;

    // demand
    std::vector<std::vector<std::string>> csv_demand;
    read_csv(fp_dm, csv_demand);
    handle_dm(csv_demand, demand);

    // qos
    std::vector<std::vector<std::string>> csv_qos;
    read_csv(fp_qos, csv_qos);
    handle_qos_filter(csv_qos, qos, edge_dist_num, qos_constrain, qos_bitmap);

    // names
    handle_names(csv_qos, client_names, edge_names);

    // site_bandwidth
    std::vector<std::vector<std::string>> csv_sb;
    read_csv(fp_sb, csv_sb);
    handle_sb(csv_sb, site_bandwidth);
}

void try1(std::vector<std::string>& client_names, std::vector<std::string>& edge_names,
             std::vector<std::vector<int>>& demand, std::vector<std::vector<int>>& qos,
             std::vector<int>& edge_dist_num, std::vector<int>& site_bandwidth,
             int qos_constrain, std::vector<std::vector<std::vector<int>>>& res,
             std::vector<std::vector<int>>& qos_bitmap) {

    res.resize(demand.size(), std::vector<std::vector<int>>(client_names.size(),
        std::vector<int>(edge_names.size(), 0)));  // 时间 client序号 edge序号 分配数量
    int five_percent_time = demand.size() * 0.05 - 1;
    std::vector<std::pair<int, int>> sort_time;  // {某时刻总需求量, 某时刻idx}
    std::vector<std::pair<int, int>> sort_sb;  // {某边缘节点带宽, 某边缘节点idx}

    // 统计sort_time
    int idx = 0;
    for (auto &row : demand) {
        int sumrow = 0;
        for(auto num : row) sumrow += num;
        sort_time.push_back({sumrow, idx++});
    }
    // 按时刻总需求量排序，从大到小
    std::sort(sort_time.begin(), sort_time.end(), std::greater<std::pair<int, int>>());
    // 统计sort_sb
    int bw_idx = 0;
    for (auto &bw : site_bandwidth) {
        sort_sb.push_back({bw, bw_idx++});
    }
    // 按边缘节点带宽排序，从小到大
    std::sort(sort_sb.begin(), sort_sb.end());

    // step 1：处理总需求最大的前 5% 时刻，优先分配带宽小的边缘节点
    for (int i = 0; i < five_percent_time; i++) {
        auto sort_sb_copy = sort_sb;
        auto time_idx = sort_time[i].second;
        // 依次处理每个客户的请求
        for (int cli_idx = 0; cli_idx < demand[time_idx].size(); cli_idx++) {
            // 尝试用小带宽边缘节点来满足客户
            for (int j = 0; j < sort_sb_copy.size(); j++) {
                int &edge_bw = sort_sb_copy[j].first;
                int edge_idx = sort_sb_copy[j].second;
                // 检查该边缘节点是否满足qos，且是否还有剩余带宽
                if (qos_bitmap[edge_idx][cli_idx] && edge_bw) {
                    if (demand[time_idx][cli_idx] <= edge_bw) {  // 边缘节点带宽足够
                        edge_bw -= demand[time_idx][cli_idx];
                        res[time_idx][cli_idx][edge_idx] += demand[time_idx][cli_idx];  // 记录res
                        break;
                    } else {  // 边缘节点带宽不足，把带宽全部分配给用户，继续请求下一个边缘节点
                        demand[time_idx][cli_idx] -= edge_bw;
                        res[time_idx][cli_idx][edge_idx] += edge_bw;  // 记录res
                        edge_bw = 0;
                        continue;
                    }
                }
            }
        }
    }

    // step 2：处理总需求后 95% 时刻，尽可能不分配在第一阶段被用过的边缘节点，方案是
    // 尝试用  “大”  带宽边缘节点来满足客户
    std::sort(sort_sb.begin(), sort_sb.end(), std::greater<std::pair<int, int>>());
    for (int i = five_percent_time; i < demand.size(); i++) {
        auto sort_sb_copy = sort_sb;
        auto time_idx = sort_time[i].second;
        // 依次处理每个客户的请求
        for (int cli_idx = 0; cli_idx < demand[time_idx].size(); cli_idx++) {
            // 尝试用  “大”  带宽边缘节点来满足客户
            for (int j = 0; j < sort_sb_copy.size(); j++) {
                int &edge_bw = sort_sb_copy[j].first;
                int edge_idx = sort_sb_copy[j].second;
                // 检查该边缘节点是否满足qos，且是否还有剩余带宽
                if (qos_bitmap[edge_idx][cli_idx] && edge_bw) {
                    if (demand[time_idx][cli_idx] <= edge_bw) {  // 边缘节点带宽足够
                        edge_bw -= demand[time_idx][cli_idx];
                        res[time_idx][cli_idx][edge_idx] += demand[time_idx][cli_idx];  // 记录res
                        break;
                    } else {  // 边缘节点带宽不足，把带宽全部分配给用户，继续请求下一个边缘节点
                        demand[time_idx][cli_idx] -= edge_bw;
                        res[time_idx][cli_idx][edge_idx] += edge_bw;  // 记录res
                        edge_bw = 0;
                        continue;
                    }
                }
            }
        }
    }
    return;
}

void try2(std::vector<std::string>& client_names, std::vector<std::string>& edge_names,
             std::vector<std::vector<int>>& demand, std::vector<std::vector<int>>& qos,
             std::vector<int>& edge_dist_num, std::vector<int>& site_bandwidth,
             int qos_constrain, std::vector<std::vector<std::vector<int>>>& res,
             std::vector<std::vector<int>>& qos_bitmap) {
    
    const int cli_size = client_names.size();
    const int edge_size = edge_names.size();
    res.resize(demand.size(), std::vector<std::vector<int>>(cli_size,
        std::vector<int>(edge_size, 0)));  // 时间 client序号 edge序号 分配数量
    int five_percent_time = demand.size() * 0.05 - 1;
    std::vector<std::pair<int, int>> sort_time;  // {某时刻总需求量, 某时刻idx}
    std::vector<std::pair<int, int>> sort_sb;  // {某边缘节点带宽, 某边缘节点idx}
    std::vector<int> step1_used_edge_idxs(edge_size, 0);  // 前 5% 时刻用到的边缘节点idx

    // 统计sort_time
    int idx = 0;
    for (auto &row : demand) {
        int sumrow = 0;
        for(auto num : row) sumrow += num;
        sort_time.push_back({sumrow, idx++});
    }
    // 按时刻总需求量排序，从大到小
    std::sort(sort_time.begin(), sort_time.end(), std::greater<std::pair<int, int>>());
    // 统计sort_sb
    int bw_idx = 0;
    for (auto &bw : site_bandwidth) {
        sort_sb.push_back({bw, bw_idx++});
    }
    // 按边缘节点带宽排序，从小到大
    std::sort(sort_sb.begin(), sort_sb.end());

    // step 1：处理总需求最大的前 5% 时刻，优先分配带宽小的边缘节点
    for (int i = 0; i < five_percent_time; i++) {
        auto sort_sb_copy = sort_sb;
        auto time_idx = sort_time[i].second;
        // 依次处理每个客户的请求
        for (int cli_idx = 0; cli_idx < demand[time_idx].size(); cli_idx++) {
            // 尝试用小带宽边缘节点来满足客户
            for (int j = 0; j < sort_sb_copy.size(); j++) {
                int &edge_bw = sort_sb_copy[j].first;
                int edge_idx = sort_sb_copy[j].second;
                // 检查该边缘节点是否满足qos，且是否还有剩余带宽
                if (qos_bitmap[edge_idx][cli_idx] && edge_bw) {
                    step1_used_edge_idxs[edge_idx] = 1;  // 记录前 5% 时刻用到的边缘节点idx
                    if (demand[time_idx][cli_idx] <= edge_bw) {  // 边缘节点带宽足够
                        edge_bw -= demand[time_idx][cli_idx];
                        res[time_idx][cli_idx][edge_idx] += demand[time_idx][cli_idx];  // 记录res
                        break;
                    } else {  // 边缘节点带宽不足，把带宽全部分配给用户，继续请求下一个边缘节点
                        demand[time_idx][cli_idx] -= edge_bw;
                        res[time_idx][cli_idx][edge_idx] += edge_bw;  // 记录res
                        edge_bw = 0;
                        continue;
                    }
                }
            }
        }
    }

    // step 2：处理总需求后 95% 时刻，不使用在第一阶段被用过的边缘节点
    // 统计客户可以使用的边缘节点
    std::vector<std::vector<int>> cli_edge_useful;
    for (int cli_idx = 0; cli_idx < cli_size; cli_idx++) {
        std::vector<int> row;
        for (int edge_idx = 0; edge_idx < edge_size; edge_idx++) {
            if (qos_bitmap[edge_idx][cli_idx] && !step1_used_edge_idxs[edge_idx])
                row.push_back(edge_idx);
        }
        cli_edge_useful.emplace_back(std::move(row));
    }
    for (int i = five_percent_time; i < demand.size(); i++) {
        auto time_idx = sort_time[i].second;
        auto site_bandwidth_copy = site_bandwidth;
        // 依次处理每个客户的请求
        for (int cli_idx = 0; cli_idx < cli_size; cli_idx++) {
            int slice = 1000;
            std::vector<int> &edge_useful = cli_edge_useful[cli_idx];
            int edge_useful_cnt = edge_useful.size();
            while (demand[time_idx][cli_idx] > 0) {
                // 循环消费第一阶段未用的边缘节点
                for (int j = 0; demand[time_idx][cli_idx] > 0, j < edge_useful_cnt; j++) {
                    if (demand[time_idx][cli_idx] < slice) {
                        slice = demand[time_idx][cli_idx];
                    }
                    int edge_idx = edge_useful[j];
                    if (site_bandwidth_copy[edge_idx]) {
                        if (slice <= site_bandwidth_copy[edge_idx]) {  // 边缘节点带宽足够
                            demand[time_idx][cli_idx] -= slice;
                            res[time_idx][cli_idx][edge_idx] += slice;  // 记录res
                            site_bandwidth_copy[edge_idx] -= slice;
                        } else {  // 边缘节点带宽不足，把带宽全部分配给用户，继续请求下一个边缘节点
                            demand[time_idx][cli_idx] -= site_bandwidth_copy[edge_idx];
                            res[time_idx][cli_idx][edge_idx] += site_bandwidth_copy[edge_idx];  // 记录res
                            site_bandwidth_copy[edge_idx] = 0;
                        }
                    }
                }
            }
        }
    }
    return;
}

void handle_output(std::vector<std::vector<std::vector<int>>>& res,
                   std::vector<std::string>& client_names, std::vector<std::string>& edge_names) {
    #ifdef DEBUG
    std::ofstream fout("./output/solution.txt", std::ios_base::trunc);
    #else
    std::ofstream fout("/output/solution.txt", std::ios_base::trunc);
    #endif
    for (size_t time = 0; time < res.size(); time++) {
        auto& output_time = res[time];
        for (size_t cli_idx = 0; cli_idx < output_time.size(); cli_idx++) {
            fout << client_names[cli_idx] << ":";
            bool is_first = true;
            for (size_t edge_idx = 0; edge_idx < output_time[cli_idx].size(); edge_idx++) {
                if (output_time[cli_idx][edge_idx] != 0) {
                    if (!is_first)
                        fout << ",";
                    else
                        is_first = false;
                    fout << "<" << edge_names[edge_idx] << "," << output_time[cli_idx][edge_idx]
                         << ">";
                }
            }
            fout << std::endl;
        }
    }
    return;
}

int main() {
    std::vector<std::string> client_names;  // 客户节点名字
    std::vector<std::string> edge_names;  // 边缘节点名字
    std::vector<std::vector<int>> demand;  // 带宽需求。行代表时间，列代表客户节点
    std::vector<std::vector<int>> qos;  // 每行:满足qos的client_idx
    std::vector<std::vector<int>> qos_bitmap;  // 1表示满足qos
    std::vector<int> edge_dist_num;  // 每个edge可以分发的节点数
    std::vector<int> site_bandwidth;  // 边缘节点带宽
    int qos_constrain;  // qos限制

    handle_input(client_names, edge_names, demand, qos, edge_dist_num,
                 site_bandwidth, qos_constrain, qos_bitmap);

    std::vector<std::vector<std::vector<int>>> res_for_print;
    try2(
        client_names, edge_names, demand, qos, edge_dist_num,
        site_bandwidth, qos_constrain, res_for_print, qos_bitmap);

    handle_output(res_for_print, client_names, edge_names);
    return 0;
}