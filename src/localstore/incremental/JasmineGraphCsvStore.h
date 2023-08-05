//
// Created by sandaruwan on 4/29/23.
//

#include <string>
#include <nlohmann/json.hpp>
#include <unordered_set>

using json = nlohmann::json;

#ifndef JASMINEGRAPH_JASMINEGRAPHCSVSTORE_H
#define JASMINEGRAPH_JASMINEGRAPHCSVSTORE_H


class JasmineGraphCsvStore
{
public:
    unsigned int graphID = 0;
    unsigned int partitionID = 0;
    unsigned int initial_timestamp;
    unsigned int last_timestamp ;
    unsigned int testing_batch_size ;
    unsigned int training_batch_size ;
    unsigned int current_timestamp;
//    std::vector<json> batch;
    std::vector<json> training_batch;
    std::vector<json> test_batch;
    std::unordered_set<std::string> nodes_map;
    unsigned int batch_number = 0;
    static const unsigned long MILLISECONDS = 3600*24 ;
    static const unsigned long SECONDS ;
    JasmineGraphCsvStore(unsigned int graphID , unsigned int partitionID);

    static std::pair<std::string, std::string> getIDs(std::string edgeString );
    std::string saveCsvData(std::string edgeString);


    std::string addNodeFromString(std::string edgeString);
    std::string addEdgeFromString(std::string edgeString);
//    static std::pair<std::string, std::string> getIDs(std::string edgeString );


    std::string addGraphEdgeFromString(std::string edgeString);

    std::string print_node_index();

//    std::unordered_map<std::string,  NodeManager*> nodeManagerIndex;
};



#endif //JASMINEGRAPH_JASMINEGRAPHCSVSTORE_H
