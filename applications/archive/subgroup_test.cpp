#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "derecho/derecho.h"

using derecho::RawObject;
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;

template <typename InputIt, typename OutputIt, typename Elem>
OutputIt unordered_intersection(InputIt first, InputIt last, std::unordered_set<Elem> filter, OutputIt output) {
    while(first != last) {
        if(filter.count(*first) > 0) {
            *output++ = *first;
        }
        first++;
    }
    return output;
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        cout << "Error: Expected number of nodes in experiment as the first argument." << endl;
        return -1;
    }
    uint32_t num_nodes = std::stoi(argv[1]);

    int num_messages = 100;

    auto stability_callback = [&num_messages](
                                      uint32_t subgroup_num, uint32_t sender_id, long long int index, char* buf,
                                      long long int msg_size) {
        if(index == num_messages - 1) {
            cout << "Received the last message in subgroup " << subgroup_num << " from sender " << sender_id << endl;
            cout << "The last message is: " << endl;
            cout << buf << endl;
        }
        cout << "In stability callback; sender = " << sender_id
             << ", index = " << index << endl;
    };

    derecho::CallbackSet callbacks{stability_callback};

    //Assuming there will be a total of up to 9 nodes, define 3 subgroups with 3 nodes each
    //Also assumes that the node IDs will be 0-8 (which they always are in our experiments)
    std::unordered_set<node_id_t> group_0_members{0, 1, 2};
    std::unordered_set<node_id_t> group_1_members{3, 4, 5};
    std::unordered_set<node_id_t> group_2_members{6, 7, 8};
    derecho::SubgroupInfo subgroup_info{
            {{std::type_index(typeid(RawObject)),
              [group_0_members, group_1_members, group_2_members](const derecho::View& curr_view, int& next_unassigned_rank) {
            std::vector<node_id_t> subgroup_0_members;
            std::vector<node_id_t> subgroup_1_members;
            std::vector<node_id_t> subgroup_2_members;
	    unordered_intersection(curr_view.members.begin(), curr_view.members.end(),
                                   group_0_members, std::back_inserter(subgroup_0_members));
	    unordered_intersection(curr_view.members.begin(), curr_view.members.end(),
                                   group_1_members, std::back_inserter(subgroup_1_members));
	    unordered_intersection(curr_view.members.begin(), curr_view.members.end(),
                                   group_2_members, std::back_inserter(subgroup_2_members));
            derecho::subgroup_shard_layout_t subgroup_vector(3);
            std::vector<int> subgroup_0_senders(subgroup_0_members.size());
            if(subgroup_0_senders.size()) {
                subgroup_0_senders[0] = 1;
            }
            subgroup_vector[0].emplace_back(curr_view.make_subview(subgroup_0_members, derecho::Mode::ORDERED, subgroup_0_senders));
            subgroup_vector[1].emplace_back(curr_view.make_subview(subgroup_1_members)); // ,subgroup_1_senders
            subgroup_vector[2].emplace_back(curr_view.make_subview(subgroup_2_members)); // ,subgroup_2_senders
            next_unassigned_rank = std::max(next_unassigned_rank, 9);
            return subgroup_vector; }}},
            {std::type_index(typeid(RawObject))}};

    derecho::Group<> managed_group(callbacks, subgroup_info);

    cout << "Finished constructing/joining ManagedGroup" << endl;

    while(managed_group.get_members().size() < num_nodes) {
    }

    uint32_t my_subgroup_num;
    const node_id_t node_id = derecho::getConfUInt32(CONF_DERECHO_LOCAL_ID);
    if(node_id < 3) {
        my_subgroup_num = 0;
    } else if(node_id >= 3 && node_id < 6) {
        my_subgroup_num = 1;
    } else {
        my_subgroup_num = 2;
    }
    // all are senders except node id's 1 and 2 in shard 0 and nodes >= 9
    if(node_id != 1 && node_id != 2 && node_id <= 8) {
        for(int i = 0; i < num_messages; ++i) {
            // random message size between 1 and 100
            long long unsigned int max_msg_size = derecho::getConfUInt64(CONF_DERECHO_MAX_PAYLOAD_SIZE);
            unsigned int msg_size = (rand() % 7 + 2) * (max_msg_size / 10);
            derecho::RawSubgroup& subgroup_handle = managed_group.get_subgroup<RawObject>(my_subgroup_num);
            char* buf = subgroup_handle.get_sendbuffer_ptr(msg_size);
            while(!buf) {
                buf = subgroup_handle.get_sendbuffer_ptr(msg_size);
            }
            for(unsigned int k = 0; k < msg_size; ++k) {
                buf[k] = 'a' + (rand() % 26);
            }
            buf[msg_size - 1] = 0;
            subgroup_handle.send();
        }
    }
    // everything that follows is rendered irrelevant
    while(true) {
    }

    cout << "Done" << endl;
    managed_group.barrier_sync();

    managed_group.leave();

    cout << "Finished destroying managed_group" << endl;
}
