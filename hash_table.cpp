#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <mach/mach.h>

using namespace std;
using hrc = chrono::high_resolution_clock;

size_t getMemoryUsageKB() {
    task_basic_info info;
    mach_msg_type_number_t size = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size) == KERN_SUCCESS)
        return info.resident_size / 1024;
    return 0;
}

void logMem(const string& tag) {
    cout << "[LOG] " << tag << " memory: " << getMemoryUsageKB() << " KB\n";
}
long long now() {
    return chrono::duration_cast<chrono::microseconds>(hrc::now().time_since_epoch()).count();
}
void logUS(long long us, const string& tag) {
    cout << "[LOG] " << tag << " time  : " << us << " us\n";
}

struct Passenger {
    string id, name, gender, nation, passport;
    int age{};
    string flight, airline, depAir, arrAir, depTime, arrTime, seat, seClass;
};

void print(const Passenger& p) {
    cout << "-----------------------------\nID: " << p.id << "\nName: " << p.name
         << "\nGender: " << p.gender << "\nAge: " << p.age << "\nNation: " << p.nation
         << "\nPassport: " << p.passport << "\nFlight: " << p.flight << "\nAirline: " << p.airline
         << "\nDepAir: " << p.depAir << "\nArrAir: " << p.arrAir << "\nDepTime: " << p.depTime
         << "\nArrTime: " << p.arrTime << "\nSeat: " << p.seat << "\nClass: " << p.seClass << endl;
}

Passenger getInput() {
    Passenger p; string tmp;
    cout << "ID: "; getline(cin, p.id);
    cout << "Name: "; getline(cin, p.name);
    cout << "Gender: "; getline(cin, p.gender);
    cout << "Age: "; getline(cin, tmp); p.age = stoi(tmp);
    cout << "Nation: "; getline(cin, p.nation);
    cout << "Passport: "; getline(cin, p.passport);
    cout << "Flight: "; getline(cin, p.flight);
    cout << "Airline: "; getline(cin, p.airline);
    cout << "DepAir: "; getline(cin, p.depAir);
    cout << "ArrAir: "; getline(cin, p.arrAir);
    cout << "DepTime: "; getline(cin, p.depTime);
    cout << "ArrTime: "; getline(cin, p.arrTime);
    cout << "Seat: "; getline(cin, p.seat);
    cout << "Class: "; getline(cin, p.seClass);
    return p;
}


vector<string> splitCSV(const string& line) {
    vector<string> out;
    string cur;
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"') inQuotes = !inQuotes;
        else if (c == ',' && !inQuotes) {
            out.push_back(cur);
            cur.clear();
        } else cur += c;
    }
    out.push_back(cur);
    return out;
}


void loadCSV(const string& filename, unordered_map<string, Passenger>& hashmap) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[ERROR] File not found: " << filename << endl;
        return;
    }

    string line;
    getline(file, line); 
    while (getline(file, line)) {
        auto v = splitCSV(line);
        if (v.size() < 14) continue;
        Passenger p;
        p.id = v[0]; p.name = v[1]; p.gender = v[2]; p.age = stoi(v[3]);
        p.nation = v[4]; p.passport = v[5]; p.flight = v[6]; p.airline = v[7];
        p.depAir = v[8]; p.arrAir = v[9]; p.depTime = v[10]; p.arrTime = v[11];
        p.seat = v[12]; p.seClass = v[13];
        hashmap[p.id] = p;
    }

    file.close();
    cout << "[INFO] Loaded data from: " << filename << "\n";
}

void writeCSV(const string& filename, const unordered_map<string, Passenger>& hashmap) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "[ERROR] Gagal membuka file untuk menulis: " << filename << endl;
        return;
    }

    file << "id,name,gender,age,nation,passport,flight,airline,depAir,arrAir,depTime,arrTime,seat,seClass\n";

    for (const auto& [id, p] : hashmap) {
        file << '"' << p.id << "\","
             << '"' << p.name << "\","
             << '"' << p.gender << "\","
             << p.age << ","
             << '"' << p.nation << "\","
             << '"' << p.passport << "\","
             << '"' << p.flight << "\","
             << '"' << p.airline << "\","
             << '"' << p.depAir << "\","
             << '"' << p.arrAir << "\","
             << '"' << p.depTime << "\","
             << '"' << p.arrTime << "\","
             << '"' << p.seat << "\","
             << '"' << p.seClass << "\"\n";
    }

    file.close();
    cout << "[INFO] Data disimpan ke: " << filename << endl;
}

int main() {
    unordered_map<string, Passenger> hashmap;
    string filePath = "/Users/viahana/Documents/FP_STRUKDAT/flight_passengers_1000_shuffled.csv";

    loadCSV(filePath, hashmap);

    while (true) {
        cout << "\n== MENU HASH MAP ==\n"
             << "1 Insert\n"
             << "2 Update\n"
             << "3 Delete\n"
             << "4 Find\n"
             << "5 Range\n"
             << "6 Exit\n"
             << "7 Write to CSV\n"
             << "Choose: ";
        int c; cin >> c; cin.ignore();
        if (c == 6) break;

        long long st = now();

        if (c == 1) {
            Passenger p = getInput();
            hashmap[p.id] = p;
            cout << "Insert OK (belum disimpan ke file)\n";
        } else if (c == 2) {
            cout << "ID: ";
            string id; getline(cin, id);
            if (hashmap.count(id)) {
                Passenger p = getInput(); p.id = id;
                hashmap[id] = p;
                cout << "Update OK (belum disimpan ke file)\n";
            } else cout << "Not found\n";
        } else if (c == 3) {
            cout << "ID: ";
            string id; getline(cin, id);
            hashmap.erase(id);
            cout << "Delete OK (belum disimpan ke file)\n";
        } else if (c == 4) {
            cout << "ID: ";
            string id; getline(cin, id);
            if (hashmap.count(id)) print(hashmap[id]);
            else cout << "Not found\n";
        } else if (c == 5) {
            string lo, hi;
            cout << "Low ID: "; getline(cin, lo);
            cout << "High ID: "; getline(cin, hi);
            int cnt = 0;
            for (auto& [k, v] : hashmap)
                if (k >= lo && k <= hi) { print(v); cnt++; }
            cout << "Total: " << cnt << " found\n";
        } else if (c == 7) {
            writeCSV(filePath, hashmap);
        }

        logUS(now() - st, "operasi");
        logMem("operasi");
    }

    return 0;
}
