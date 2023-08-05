//
// Created by sandaruwan on 4/29/23.
//

#include "JasmineGraphCsvStore.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../nativestore/RelationBlock.h"
#include "../../util/logger/Logger.h"
#include "../../nativestore/CtypesLibrary.h"
#include "../../frontend/JasmineGraphFrontEnd.h"

Logger csvStore_logger;
pthread_mutex_t lockCsv;

JasmineGraphCsvStore::JasmineGraphCsvStore(unsigned int graphID, unsigned int partitionID) {
    Utils utils;
    this->graphID = graphID;
    this->partitionID = partitionID;
    std::string initial_timestamp = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.initial_timestamp");
    this->initial_timestamp = std::stoi(initial_timestamp);
    std::string last_timestamp = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.last_timestamp");
    this->last_timestamp = std::stoi(last_timestamp);
    std::string training_batch_size = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.training_batch_size");
    this->training_batch_size = std::stoi(training_batch_size);
    std::string testing_batch_size = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.testing_batch_size");
    this->testing_batch_size = std::stoi(testing_batch_size);
    std::string current_timestamp = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.current_timestamp");
    this->current_timestamp = std::stoi(current_timestamp);
    this->training_batch = std::vector<json>();
    this->test_batch = std::vector<json>();
    this->nodes_map = std::unordered_set<std::string>();
    std::string graphIdentifier = graphID + "_" + partitionID;
};

std::pair<std::string, std::string> JasmineGraphCsvStore::getIDs(std::string edgeString) {
    try {
        auto edgeJson = json::parse(edgeString);
        if (edgeJson.contains("graphIdentifier")) {
            auto graphIdentifier = edgeJson["graphIdentifier"];
            std::string graphId = std::string(graphIdentifier["graphId"]);
            std::string pId = to_string(graphIdentifier["pid"]);
            return {graphId, pId};
        }


    } catch (const std::exception &) {
        csvStore_logger.log(
                "Error while processing node ID data = " + edgeString +
                "Could be due to JSON parsing error or error while persisting the data to disk",
                "error");
    }
}

std::string JasmineGraphCsvStore::saveCsvData(std::string edgeString) {
    try {
        Utils utils;
        pthread_mutex_lock(&lockCsv);
        auto edgeJson = json::parse(edgeString);
//        auto graphIdentifierJson = edgeJson["graphIdentifier"];
        std::string timestamp = edgeJson["properties"]["timestamp"];
        float a_float = std::stof(timestamp);
        unsigned int a_int = static_cast<int>(a_float);
        unsigned int start = this->current_timestamp;

        if (start != this->initial_timestamp) {
            unsigned int end = this->current_timestamp + this->testing_batch_size;
            if (start <= a_int and a_int < end) {
                this->training_batch.push_back(edgeJson);
            } else {
                // Write the JSON objects to a CSV file
                std::string saving_location = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.saving_location");
                std::string dataset_name = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.name");
                std::string partitionCount = utils.getJasmineGraphProperty("org.jasminegraph.server.streaming.npartitions");

                std::ofstream test_edge_file( saving_location + dataset_name + "_" + partitionCount + "_" +
                                   std::to_string(this->partitionID) + "/" + std::to_string(this->batch_number) +
                                   "_test_batch_edges.csv");
                std::ofstream test_node_file(saving_location + dataset_name + "_" + partitionCount + "_" +
                                   std::to_string(this->partitionID) + "/" + std::to_string(this->batch_number) +
                                   "_test_batch_nodes.csv");
                for (const auto &j: this->training_batch) {

                    auto sourceJson = j["source"];
                    std::string source_id = sourceJson["id"];
                    std::string source_properties = sourceJson["properties"]["parameters"];

                    auto destinationJson = j["destination"];
                    std::string destination_id = destinationJson["id"];
                    std::string destination_properties = destinationJson["properties"]["parameters"];

                    if(this->nodes_map.find(source_id) == this->nodes_map.end()){
                        test_node_file << source_id << "," << source_properties << "\n";
                        nodes_map.insert(source_id);
                    }
                    if(this->nodes_map.find(destination_id) == this->nodes_map.end()){
                        test_node_file << destination_id << "," << destination_properties << "\n";
                        nodes_map.insert(destination_id);
                    }

                    test_edge_file << source_id << "," << destination_id << "\n";
                }
                test_edge_file.close();
                test_node_file.close();
                this->batch_number++;
                this->current_timestamp = this->current_timestamp + this->testing_batch_size;
                this->training_batch.clear();
                this->nodes_map.clear();
                this->training_batch.push_back(edgeJson);
            }
        } else {
            unsigned int end = this->current_timestamp + this->training_batch_size;
            if (start <= a_int and a_int < end) {
                this->training_batch.push_back(edgeJson);
            } else {
                std::string saving_location = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.saving_location");
                std::string dataset_name = utils.getJasmineGraphProperty("org.jasminegraph.csvstore.dataset.name");
                std::string partitionCount = utils.getJasmineGraphProperty("org.jasminegraph.server.streaming.npartitions");

                std::ofstream training_edge_file(saving_location + dataset_name + "_" + partitionCount + "_" +
                                                 std::to_string(this->partitionID) + "/" + std::to_string(this->batch_number) +
                                   "_training_batch_edges.csv");
                std::ofstream training_node_file(saving_location + dataset_name + "_" + partitionCount + "_" +
                                                 std::to_string(this->partitionID) + "/" + std::to_string(this->batch_number) +
                                   "_training_batch_nodes.csv");
                for (const auto &j: this->training_batch) {
                    auto sourceJson = j["source"];
                    std::string source_id = sourceJson["id"];
                    std::string source_properties = sourceJson["properties"]["parameters"];

                    auto destinationJson = j["destination"];
                    std::string destination_id = destinationJson["id"];
                    std::string destination_properties = destinationJson["properties"]["parameters"];

                    if(this->nodes_map.find(source_id) == this->nodes_map.end()){
                        training_node_file << source_id << "," << source_properties << "\n";
                        nodes_map.insert(source_id);
                    }
                    if(this->nodes_map.find(destination_id) == this->nodes_map.end()){
                        training_node_file << destination_id << "," << destination_properties << "\n";
                        nodes_map.insert(destination_id);
                    }
                    training_edge_file << source_id << "," << destination_id << "\n";
                }
                training_edge_file.close();
                training_node_file.close();
                this->batch_number++;
                this->current_timestamp = this->current_timestamp + this->training_batch_size;
                this->training_batch.clear();
                this->nodes_map.clear();
                this->training_batch.push_back(edgeJson);
            }
        }
        pthread_mutex_unlock(&lockCsv);

    }
    catch (const std::exception &) {
        csvStore_logger.log(
                "Error while processing all data = " + edgeString +
                "Could be due to JSON parsing error or error while persisting the data to disk",
                "error");
        csvStore_logger.log("Error malformed JSON attributes!", "error");
    }
}