/////////////////////////////////////////////////////////////
//           Author: Chun-Sheng Wu, Jinson                 //
/////////////////////////////////////////////////////////////

#include "time.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include <fstream>
#include <unordered_map>
#include <cstdio>
typedef unsigned long long int ull;
//typedef long long int ull;

using namespace std;

// reference cache struct
struct cache_ {
    ull tag;
    ull index;
    ull offset;
};

// block inside the cache
class block{
    public:
        block(ull block_tag): valid(false), tag(block_tag){}
        ~block(){}
    
        bool valid;
        ull tag;
};

// cache_set
class cache_set{
    public:
        cache_set(int num_block, int block_size): num(num_block), size(block_size), block_set(num_block, block(0)){}
        ~cache_set(){}

        int num, size;
        vector<block> block_set;

        unordered_map<int, int> ma;     // store reference keys
        list<int> usedblock;            // list storing the used blocks
};

// main cache
class cache {
    public:
        cache(char replacement, int cache_size, int block_size, int set_size, int associativity, int num_set, cache_ cache_):
            repl(replacement), csize(cache_size), bsize(block_size), ssize(set_size), as(associativity), ind(num_set),
            r_acc(0), r_hit(0), w_acc(0), w_hit(0), sets(ind, cache_set(as, bsize)), cacheline(cache_){}
        
        ~cache(){}

        char repl;                              // replacement policy
        int csize, bsize, ssize, as, ind;       // cache properties
        ull r_acc, r_hit, w_acc, w_hit;         // recording

        vector<cache_set> sets;                 // cache_set
        cache_ cacheline;                       // reference cache
};

// check hits
// if hits, implement LRU
bool addr_valid(cache& target_cache, const cache_& addrline){    
    for(int i = 0; i < target_cache.as; i++){
        // if hits
        if(((target_cache.sets[addrline.index]).block_set[i].valid) && ((target_cache.sets[addrline.index]).block_set[i].tag == addrline.tag)){
            // check replacement policy, if it is random, jump out of the loop
            if(target_cache.repl == 'l'){
                auto iter = target_cache.sets[addrline.index].ma.find(i);
                // if data presents in cache, erase it from current point and push_back inside ma
                if(iter != (target_cache.sets[addrline.index].ma.end())){
                    int idx = iter->second;

                    auto list = find((target_cache.sets[addrline.index].usedblock).begin(),
                                (target_cache.sets[addrline.index].usedblock).end(), i);
                    // if data presents in cache, erase it from current point and push_back inside used blocks
                    if(list != (target_cache.sets[addrline.index].usedblock).end()){
                        (target_cache.sets[addrline.index].usedblock).erase(list);
                    }

                    target_cache.sets[addrline.index].ma.erase(iter);
                    int idx_ = (target_cache.sets[addrline.index].usedblock).size();
                    target_cache.sets[addrline.index].usedblock.push_back(i);
                    target_cache.sets[addrline.index].ma.insert(make_pair(i, idx_));
                }
            }
            return true;
        }
    }
    return false;
}

// cache data replacement while miss
void replace(cache& target_cache, const cache_& addr){
    int as = target_cache.as;
    // make block in this assoc to be accessible (hit)
    for(int i = 0; i < target_cache.as; i++){
        if(target_cache.sets[addr.index].block_set[i].valid == false){
            target_cache.sets[addr.index].block_set[i].valid = true;
            target_cache.sets[addr.index].block_set[i].tag = addr.tag;

            int idx = (target_cache.sets[addr.index].usedblock).size();
            target_cache.sets[addr.index].usedblock.push_back(i);
            target_cache.sets[addr.index].ma.insert(make_pair(i, idx));
        }
    }

    // update ma & used blocks (almost same as addr_valid function)
    if(target_cache.repl == 'l'){
        int idx = (target_cache.sets[addr.index].usedblock).front();
        (target_cache.sets[addr.index].usedblock).erase((target_cache.sets[addr.index].usedblock).begin());

        auto iter = (target_cache.sets[addr.index].ma).find(idx);
        if(iter != (target_cache.sets[addr.index].ma).end()){
            (target_cache.sets[addr.index].ma).erase(iter);

            int idx_ = (target_cache.sets[addr.index].usedblock).size();
            target_cache.sets[addr.index].usedblock.push_back(idx);
            target_cache.sets[addr.index].ma.insert(make_pair(idx, idx_));
            target_cache.sets[addr.index].block_set[idx].tag = addr.tag;
        }
    }
    // random access => access random block
    else if(target_cache.repl == 'r'){
        srand(time(NULL));
        if(target_cache.as > 1) --as;
        int repl_idx = rand() % as;
        target_cache.sets[addr.index].block_set[repl_idx].tag = addr.tag;
    }
    else cout << "Errors on Replacement Declaration!" << endl;
}

// data prefetching to cache
void fetching(const cache_& c, ull addr, cache_& addr_){
    ull index = 0;
    ull offset = 0;
    //cout << "c.tag " << c.tag << endl;
    //cout << "c.index " << c.index << endl;
    //cout << "c.offset " << c.offset << endl;
    addr_.tag = (addr >> (64-c.tag)) & 0xFFFFFF;
    index = addr << c.tag;
    addr_.index = index >> (c.tag + c.offset);
    offset = addr << (64-c.offset);
    addr_.offset = offset >> (64-c.offset); 
    //cout << "tag " << addr_.tag << endl;
    //cout << "index " << addr_.index << endl;
    //cout << "offset " << addr_.offset << endl;
}

// fetching instructions
bool ins(const string& str, char& rw, ull& addr){
    if(!str.empty()){
        // fetching r/w instructions
        if(str.find('r') != string::npos) rw = 'r';
        else if(str.find('w') != string::npos) rw = 'w';
        else return false;

        // putting address into a ull
        string str_ = str.substr(2);
        istringstream iss(str_);
        iss >> hex >> addr;
        return true;
    }
    else{
        cout << "Errors on Input Loading!" << endl;
        return false;
    }
}

int main(int argc, char *argv[]){
    // declare variables
    int cap, as, bsize;
    char repl;

    // commands input
    if(argc != 5) cout << "Input Error!!" << endl;
    else{
        cap = stoi(argv[1]);        // cap = capacity
        as = stoi(argv[2]);         // as = associativity
        bsize = stoi(argv[3]);      // bsize = block size
        repl = *argv[4];            // repl = replacement policy
    }

    int ssize = as * bsize;
    int ind = (cap * 1024) / ssize;

    cache_ cacheline;
    cacheline.tag = (ull)(64 - log2(ind) - log2(bsize));        // tag = total_bits - index - offset
    cacheline.index = (ull)log2(ind);                           // index = (cap(KB) / assoc) / block_size
    cacheline.offset = (ull)log2(bsize);                        // offset = block_size

    cache target_cache = cache(repl, cap, bsize, ssize, as, ind, cacheline);

    string str;
    int cnt = 0;
    while(getline(cin, str)){
        ++cnt;
        char rw;                    // define read or write
        ull addr = 0;               // current access addr
        cache_ addr_;               // reference cache (addr)
        
        if(ins(str, rw, addr)) fetching(cacheline, addr, addr_);        // if the instruction is able to conduct, doing data fetching
        else{
            cout << "Errors on fetching instructions!" << endl;
            return 0;
        }
        if(rw == 'r'){
            target_cache.r_acc ++;
            if(addr_valid(target_cache, addr_)) target_cache.r_hit ++;  // read hit
            else replace(target_cache, addr_);                          // read miss
        }
        else if(rw == 'w'){
            target_cache.w_acc ++;
            if(addr_valid(target_cache, addr_)) target_cache.w_hit ++;  // write hit
            else replace(target_cache, addr_);                          // write miss
        }
        else{
            cout << "Error!" << endl;
            return 0;
        }
    }
    ull r_miss = target_cache.r_acc - target_cache.r_hit;
    ull w_miss = target_cache.w_acc - target_cache.w_hit;
    double r_miss_per = 100*(double)r_miss / (double)(target_cache.r_acc);
    double w_miss_per = 100*(double)w_miss / (double)(target_cache.w_acc);
    ull total_miss = r_miss + w_miss;
    double total_miss_per = 100*(double)total_miss / (double)(target_cache.r_acc + target_cache.w_acc);

    /*cout << "total miss: " << total_miss << endl 
        << "total miss percentage: " << total_miss_per << "%" << endl
        << "read miss: " << r_miss << endl
        << "read miss percentage: " << r_miss_per << "%" << endl
        << "write miss: " << w_miss << endl
        << "write miss percentage: " << w_miss_per << "%" << endl;*/

    cout << total_miss << " " << total_miss_per << "% " << r_miss << " " << r_miss_per << "% " << w_miss << " " << w_miss_per << "%" << endl;
}



