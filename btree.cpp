#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <windows.h>
#include <psapi.h>

using namespace std;
using hrc = chrono::high_resolution_clock;

size_t getMemoryUsageKB() {
    PROCESS_MEMORY_COUNTERS c;
    GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof(c));
    return c.WorkingSetSize / 1024;
}

void logMem(const string& tag) {
    cout << "[LOG] " << tag << " memory: " << getMemoryUsageKB() << " KB\n";
}

string trim(const string& s){
    size_t a=s.find_first_not_of(" \t\r\n"), b=s.find_last_not_of(" \t\r\n");
    return a==string::npos ? "" : s.substr(a,b-a+1);
}

long long now(){
    return chrono::duration_cast<chrono::microseconds>(hrc::now().time_since_epoch()).count();
}

void logUS(long long us,const string& tag){
    cout << "[LOG] " << tag << " time  : " << us << " us\n";
}

struct Passenger{
    string id,name,gender,nation,passport;
    int age{};
    string flight,airline,depAir,arrAir,depTime,arrTime,seat,seClass;
};

void print(const Passenger& p){
    cout<<"--------------------------------\nID : "<<p.id<<"\nName : "<<p.name<<"\nGender : "<<p.gender
        <<"\nAge : "<<p.age<<"\nNation : "<<p.nation<<"\nPassport : "<<p.passport<<"\nFlight : "<<p.flight
        <<"\nAirline : "<<p.airline<<"\nDepAir : "<<p.depAir<<"\nArrAir : "<<p.arrAir<<"\nDepTime : "<<p.depTime
        <<"\nArrTime : "<<p.arrTime<<"\nSeat : "<<p.seat<<"\nClass : "<<p.seClass<<"\n";
}

const int ORDER = 4;

struct Node{
    bool leaf;
    vector<string> keys;
    vector<Passenger> rec;
    vector<Node*> child;
    Node *par, *next;
    Node(bool l): leaf(l), par(nullptr), next(nullptr) {}
};

class BPTree{

    Node* root;
    Node* leafOf(const string& k) const {
        Node* n = root;
        while (!n->leaf) {
            int i=0;
            while (i<n->keys.size() && k > n->keys[i]) i++;
            n = n->child[i];
        }
        return n;

    }

    void insParent(Node* left, const string& k, Node* right) {
        if (left == root) {
            Node* r = new Node(false);
            r->keys = {k};
            r->child = {left, right};
            left->par = right->par = r;
            root = r;
            return;
        }
        Node* p = left->par;
        int idx=0;
        while (p->child[idx] != left) idx++;
        p->keys.insert(p->keys.begin()+idx, k);
        p->child.insert(p->child.begin()+idx+1, right);
        right->par = p;
        if (p->keys.size() < ORDER) return;

        int mid = (ORDER+1)/2;
        string up = p->keys[mid];
        Node* n2 = new Node(false);
        n2->keys.assign(p->keys.begin()+mid+1, p->keys.end());
        n2->child.assign(p->child.begin()+mid+1, p->child.end());
        for (Node* c : n2->child) c->par = n2;
        p->keys.resize(mid);
        p->child.resize(mid+1);
        insParent(p, up, n2);
    }

public:
    BPTree() { root = new Node(true); }
    Node* getRoot() const { return root; }
    void insert(const Passenger& p) {
        Node* l = leafOf(p.id);
        auto it = lower_bound(l->keys.begin(), l->keys.end(), p.id);
        int pos = it - l->keys.begin();
        l->keys.insert(it, p.id);
        l->rec.insert(l->rec.begin()+pos, p);
        if (l->keys.size() < ORDER) return;

        int cut = (ORDER+1)/2;
        Node* n2 = new Node(true);
        n2->keys.assign(l->keys.begin()+cut, l->keys.end());
        n2->rec.assign(l->rec.begin()+cut, l->rec.end());
        l->keys.resize(cut);
        l->rec.resize(cut);
        n2->next = l->next;
        l->next = n2;
        n2->par = l->par;
        insParent(l, n2->keys[0], n2);
    }

    Passenger* find(const string& id) const {
        Node* l = leafOf(id);
        for (int i=0; i<l->keys.size(); i++)
            if (l->keys[i] == id)
                return const_cast<Passenger*>(&l->rec[i]);
        return nullptr;
    }

    void update(const Passenger& p){
        Passenger* q = find(p.id);
        if (q) *q = p;
    }

    void erase(const string& id){
        Node* l = leafOf(id);
        for (int i=0; i<l->keys.size(); i++)
            if (l->keys[i] == id) {
                l->keys.erase(l->keys.begin()+i);
                l->rec.erase(l->rec.begin()+i);
                break;
            }
    }

    vector<Passenger*> range(const string& lo, const string& hi) const {
        vector<Passenger*> v;
        Node* l = leafOf(lo);
        while (l) {
            for (int i=0; i<l->keys.size(); i++) {
                if (l->keys[i] > hi) return v;
                if (l->keys[i] >= lo)
                    v.push_back(const_cast<Passenger*>(&l->rec[i]));
            }
            l = l->next;
        }
        return v;
    }
};

vector<string> splitCSV(const string& line){
    vector<string> v;
    string cur; bool q = false;
    for (char c : line) {
        if (c == '"') q = !q;
        else if (c == ',' && !q) {
            v.push_back(cur);
            cur.clear();
        } else cur.push_back(c);
    }
    v.push_back(cur);
    return v;
}

void load(const string& file, BPTree& t){
    ifstream f(file);
    if (!f.is_open()) {
        cerr << "Failed to open file: " << file << endl;
        return;
    }

    string l;
    getline(f, l); // skip header

    int lineCount = 0;
    while (getline(f, l)) {
        auto v = splitCSV(l);
        if (v.size() < 14) continue;
        Passenger p{
            trim(v[0]), v[1], v[2], v[4], v[5],
            stoi(v[3]), v[6], v[7], v[8], v[9],
            v[10], v[11], v[12], v[13]
        };
        t.insert(p);
        lineCount++;
    }

    cout << "Loaded " << lineCount << " records.\n";
}


Passenger getInput(){
    Passenger p; string tmp;
    cout<<"ID: "; getline(cin,p.id); p.id=trim(p.id);
    cout<<"Name: "; getline(cin,p.name);
    cout<<"Gender: "; getline(cin,p.gender);
    cout<<"Age: "; getline(cin,tmp); p.age = stoi(tmp);
    cout<<"Nation: "; getline(cin,p.nation);
    cout<<"Passport: "; getline(cin,p.passport);
    cout<<"Flight: "; getline(cin,p.flight);
    cout<<"Airline: "; getline(cin,p.airline);
    cout<<"DepAir: "; getline(cin,p.depAir);
    cout<<"ArrAir: "; getline(cin,p.arrAir);
    cout<<"DepTime: "; getline(cin,p.depTime);
    cout<<"ArrTime: "; getline(cin,p.arrTime);
    cout<<"Seat: "; getline(cin,p.seat);
    cout<<"Class: "; getline(cin,p.seClass);
    return p;
}

void save(const string& file, const BPTree& t){
    ofstream f(file);
    if (!f.is_open()) {
        cerr << "Failed to open file for writing: " << file << endl;
        return;
    }

    f << "ID,Name,Gender,Age,Nation,Passport,Flight,Airline,DepAir,ArrAir,DepTime,ArrTime,Seat,Class\n";

    Node* l = t.getRoot();
    while (!l->leaf) l = l->child[0]; 

    while (l) {
        for (int i = 0; i < l->keys.size(); i++) {
            const Passenger& p = l->rec[i];
            f << p.id << ',' << p.name << ',' << p.gender << ',' << p.age << ',' << p.nation << ','
              << p.passport << ',' << p.flight << ',' << p.airline << ',' << p.depAir << ','
              << p.arrAir << ',' << p.depTime << ',' << p.arrTime << ',' << p.seat << ','
              << p.seClass << '\n';
        }
        l = l->next;
    }

    cout << "Data berhasil disimpan ke file.\n";
}


int main(){
    BPTree tree;
    load("C:/Users/devina/OneDrive/Documentos/FINALPROJECT/flight_passengers_1000_shuffled.csv", tree);

    logMem("setelah load");

    while (true) {
    cout<<"\n== MENU B+TREE ==\n"
        "1 Insert\n"
        "2 Update\n"
        "3 Delete\n"
        "4 Find\n"
        "5 Range\n"
        "6 Exit\n"
        "7 Save\n"
        "Choose: ";
    int c; cin>>c; cin.ignore();
    if (c == 6) break;

    long long st = now();

    if (c == 1) {
        Passenger p = getInput();
        tree.insert(p);
        cout<<"Insert OK\n";
    }
    else if (c == 2) {
        cout<<"ID: "; string id; getline(cin,id);
        Passenger* q = tree.find(id);
        if (q) {
            Passenger p = getInput(); p.id = id;
            tree.update(p);
            cout<<"Update OK\n";
        } else cout<<"Not found\n";
    }
    else if (c == 3) {
        cout<<"ID: "; string id; getline(cin,id);
        tree.erase(id);
        cout<<"Delete OK\n";
    }
    else if (c == 4) {
        cout<<"ID: "; string id; getline(cin,id);
        Passenger* q = tree.find(id);
        if (q) print(*q); else cout<<"Not found\n";
    }
    else if (c == 5) {
        string lo, hi;
        cout<<"Low ID: "; getline(cin,lo);
        cout<<"High ID: "; getline(cin,hi);
        auto v = tree.range(lo, hi);
        cout<<"Total "<<v.size()<<'\n';
        for (auto* p : v) print(*p);
    }
    else if (c == 7) {
        save("C:/Users/devina/OneDrive/Documentos/FINALPROJECT/flight_passengers_1000_shuffled.csv", tree);
    }

    logUS(now()-st, "operasi");
    logMem("operasi");
}

}
