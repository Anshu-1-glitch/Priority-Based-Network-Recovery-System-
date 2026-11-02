#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <limits>
using namespace std;

const int INF = numeric_limits<int>::max();
struct Edge { int to, weight; };
struct Station { string name, priority; bool restored = false; };

class Network {
    vector<Station> stations;
    vector<vector<Edge>> adj;
    int rankPriority(const string& p) const {
        if (p == "Critical") return 3;
        if (p == "Semi-critical") return 2;
        return 1;
    }
public:
    void addStation(string name, string priority) {
        stations.push_back({name, priority, false});
        adj.push_back({});
    }
    void addConnection(int u, int v, int w) {
        if (u < 0 || v < 0 || u >= size() || v >= size() || w <= 0) return;
        adj[u].push_back({v,w}); adj[v].push_back({u,w});
    }
    int size() const { return (int)stations.size(); }
    void display() const {
        cout << "\n========== POWER NETWORK ==========\n";
        for (int i=0; i<size(); ++i) {
            cout << i+1 << ". " << stations[i].name << " | " << stations[i].priority
                 << " | Status: " << (stations[i].restored ? "Restored" : "Pending") << "\n   Connected to: ";
            for (const Edge& e : adj[i]) cout << stations[e.to].name << "(" << e.weight << ") ";
            cout << "\n";
        }
    }
    int importantNeighbours(int u) const {
        int count=0;
        for (const Edge& e : adj[u]) if (rankPriority(stations[e.to].priority) >= 2) ++count;
        return count;
    }
    int estimatedTime(int u) const {
        int best=INF;
        for (const Edge& e : adj[u]) best=min(best,e.weight);
        return best;
    }
    void repairSchedule() const {
        vector<int> order;
        for (int i=0;i<size();++i) if (!stations[i].restored) order.push_back(i);
        sort(order.begin(),order.end(),[&](int a,int b) {
            int pa=rankPriority(stations[a].priority), pb=rankPriority(stations[b].priority);
            if (pa!=pb) return pa>pb;
            int na=importantNeighbours(a), nb=importantNeighbours(b);
            if (na!=nb) return na>nb;
            return estimatedTime(a)<estimatedTime(b);
        });
        cout << "\n======= SUGGESTED REPAIR SCHEDULE =======\n";
        if (order.empty()) { cout << "All stations are restored.\n"; return; }
        int pos=1;
        for (int id:order) {
            cout << pos++ << ". " << stations[id].name << " | " << stations[id].priority
                 << " | Important neighbours: " << importantNeighbours(id) << " | Estimated time: ";
            int t=estimatedTime(id);
            if (t==INF) cout << "Not available\n"; else cout << t << " units\n";
        }
    }
    void dijkstra(int source) const {
        if (source<0 || source>=size()) { cout << "Invalid station number.\n"; return; }
        vector<int> dist(size(),INF);
        priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>> pq;
        dist[source]=0; pq.push({0,source});
        while(!pq.empty()) {
            auto [d,u]=pq.top(); pq.pop();
            if(d!=dist[u]) continue;
            for(const Edge& e:adj[u]) if(dist[u]!=INF && dist[u]+e.weight<dist[e.to]) {
                dist[e.to]=dist[u]+e.weight; pq.push({dist[e.to],e.to});
            }
        }
        cout << "\n======= SHORTEST RESTORATION PATHS =======\nStarting station: " << stations[source].name << "\n";
        for(int i=0;i<size();++i) {
            cout << stations[i].name << " : ";
            if(dist[i]==INF) cout << "Unreachable\n"; else cout << dist[i] << " units\n";
        }
    }
    void primMST() const {
        if(size()==0) { cout << "Network is empty.\n"; return; }
        vector<bool> used(size(),false);
        vector<int> key(size(),INF), parent(size(),-1);
        priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>> pq;
        key[0]=0; pq.push({0,0}); int total=0, edgesUsed=0;
        while(!pq.empty()) {
            auto [w,u]=pq.top(); pq.pop();
            if(used[u]) continue;
            used[u]=true; total+=w;
            for(const Edge& e:adj[u]) if(!used[e.to] && e.weight<key[e.to]) {
                key[e.to]=e.weight; parent[e.to]=u; pq.push({key[e.to],e.to});
            }
        }
        cout << "\n======= MINIMUM SPANNING TREE (PRIM) =======\n";
        for(int i=1;i<size();++i) if(parent[i]!=-1) {
            cout << stations[parent[i]].name << " -- " << stations[i].name << " : " << key[i] << "\n";
            ++edgesUsed;
        }
        if(edgesUsed!=size()-1) cout << "Warning: Network is disconnected; all stations cannot be connected in one spanning tree.\n";
        cout << "Total cost of selected component(s): " << total << "\n";
    }
    void markRestored(int number) {
        int id=number-1;
        if(id<0 || id>=size()) { cout << "Invalid station number.\n"; return; }
        stations[id].restored=true; cout << stations[id].name << " marked as restored.\n";
    }
    void runTests() const {
        cout << "\n======= BASIC UNIT TESTS =======\n";
        // Test Dijkstra on A-B=5, A-C=10, B-C=2. Expected distances: 0, 5, 7.
        vector<vector<Edge>> g(3);
        auto add=[&](int a,int b,int w){g[a].push_back({b,w});g[b].push_back({a,w});};
        add(0,1,5); add(0,2,10); add(1,2,2);
        vector<int> d(3,INF); priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>> q;
        d[0]=0; q.push({0,0});
        while(!q.empty()) { auto [du,u]=q.top();q.pop();if(du!=d[u])continue;
            for(auto e:g[u]) if(d[u]+e.weight<d[e.to]) {d[e.to]=d[u]+e.weight;q.push({d[e.to],e.to});}}
        bool t1=(d[0]==0 && d[1]==5 && d[2]==7);
        bool t2=(rankPriority("Critical")>rankPriority("Normal"));
        bool t3=(size()==10);
        cout << "Test 1 - Dijkstra distances: " << (t1?"PASS":"FAIL") << "\n";
        cout << "Test 2 - Critical priority above Normal: " << (t2?"PASS":"FAIL") << "\n";
        cout << "Test 3 - Sample network has 10 stations: " << (t3?"PASS":"FAIL") << "\n";
        cout << ((t1&&t2&&t3)?"All basic tests passed.\n":"Some tests failed.\n");
    }
};

void createSampleNetwork(Network& n) {
    n.addStation("Main Grid","Critical");
    n.addStation("Hospital","Critical");
    n.addStation("Water Plant","Critical");
    n.addStation("Communication","Semi-critical");
    n.addStation("Residential A","Normal");
    n.addStation("Residential B","Normal");
    n.addStation("Emergency Centre","Critical");
    n.addStation("Substation G","Semi-critical");
    n.addStation("Residential C","Normal");
    n.addStation("Substation J","Semi-critical");
    int edges[][3]={{0,1,4},{0,2,3},{0,3,5},{0,4,8},{1,2,2},{1,6,3},{2,3,4},{2,7,4},{3,4,3},{3,7,2},{4,5,2},{4,8,4},{5,6,5},{5,8,3},{6,7,2},{6,9,6},{7,8,3},{7,9,4},{8,9,2},{1,9,7}};
    for(auto& e:edges) n.addConnection(e[0],e[1],e[2]);
}

int main() {
    Network network; createSampleNetwork(network); int choice;
    do {
        cout << "\n========================================\n"
             << " PRIORITY BASED NETWORK RECOVERY SYSTEM\n"
             << "========================================\n"
             << "1. Display sample network\n"
             << "2. Generate priority-based repair schedule\n"
             << "3. Find shortest paths (Dijkstra)\n"
             << "4. Generate minimum spanning tree (Prim)\n"
             << "5. Mark a station as restored\n"
             << "6. Run basic unit tests\n"
             << "0. Exit\nEnter your choice: ";
        if(!(cin>>choice)) { cout << "Please enter a valid number.\n"; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); continue; }
        switch(choice) {
            case 1: network.display(); break;
            case 2: network.repairSchedule(); break;
            case 3: { int s; cout << "Enter starting station number (1-" << network.size() << "): "; cin>>s; network.dijkstra(s-1); break; }
            case 4: network.primMST(); break;
            case 5: { int s; cout << "Enter station number to mark restored (1-" << network.size() << "): "; cin>>s; network.markRestored(s); break; }
            case 6: network.runTests(); break;
            case 0: cout << "Exiting the system. Thank you!\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while(choice!=0);
    return 0;
}