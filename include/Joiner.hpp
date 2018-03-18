#pragma once
#include <vector>
#include <cstdint>
#include <set>
#include "Operators.hpp"
#include "Relation.hpp"
#include "Parser.hpp"
#include <condition_variable>
#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>

//---------------------------------------------------------------------------

class Operator;
//class Checksum;

class Joiner {
    friend Checksum;
    /// Add scan to query
    std::shared_ptr<Operator> addScan(std::set<unsigned>& usedRelations,SelectInfo& info,QueryInfo& query);
 
    boost::asio::io_service ioService;
    boost::thread_group threadPool;
    boost::asio::io_service::work work;
    
    int pendingAsyncJoin = 0;
    int nextQueryIndex = 0;
    std::vector<std::vector<uint64_t>> asyncResults; //checksums
    std::vector<std::shared_ptr<Checksum>> asyncJoins; 
    std::condition_variable cvAsync;
    std::mutex cvAsyncMt;
    
    public:
    Joiner(int threadNum) : work(ioService) {
        for (int i=0; i<threadNum; i++) {
            threadPool.create_thread(
                boost::bind(&boost::asio::io_service::run, &ioService)
                );
        }        
    }
    /// The relations that might be joined
    std::vector<Relation> relations;
    /// Add relation
    void addRelation(const char* fileName);
    /// Get relation
    Relation& getRelation(unsigned id);
    /// Joins a given set of relations
    std::string join(QueryInfo& i, bool async=true);
    /// wait for async joins
    void waitAsyncJoins();
    /// return parsed asyncResults 
    std::vector<std::string> getAsyncJoinResults();
	/// print asyncJoin infos
	void printAsyncJoinInfo();

    ~Joiner() {
        ioService.stop();
        threadPool.join_all();
    }
    
};
//---------------------------------------------------------------------------
