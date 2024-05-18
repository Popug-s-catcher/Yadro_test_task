#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <ctime>
#include <list>
#include <vector>
#include <queue>
#include <map>
#include <utility>
#include <cstring>

using namespace std;

struct Note{
    unsigned time;
    int eventID;
    string eventBody;
    int tableNum = 0;
};

struct dayReport{
    int tables, price;
    unsigned start, end;
    list<Note*> notes;
};

struct tableRevenue{
    int revenue = 0;        
    unsigned total = 0;     // total time usage
    unsigned start = 0;     // variable of starting the usage by each client
};

unsigned strToSec(string time)
{
    string h = time.substr(0, 2);
    string m = time.substr(3, 2);
    return stoi(h) * 3600 + stoi(m) * 60;
}

Note* createNote(long time, int id, string body, int tNum = 0){
    Note *tmp = new Note;
    tmp -> time = time;
    tmp -> eventID = id; 
    tmp -> eventBody = body; 
    tmp -> tableNum = tNum;
    return tmp;
}

bool timeCheck(string time){
    
    if ((time.length() != 5) || 
    (time[2] != ':') ||
    (stoi(time.substr(0, 2)) > 25 ) ||
    (stoi(time.substr(3, 2)) > 59))
    {
        return true;
    }
    
    return false;
}

bool eventCheck(dayReport* report, long time){

    if (report->notes.size() != 0)
    {
        if (report->notes.back()->time > time)
            return true;
    }
    
    return false;
}

bool nameCheck(string name){
    
    for (char letter : name){
        if (!(97 <= (int) letter && (int) letter <= 122) && 
        !(48 <= (int) letter && (int) letter <= 57) && 
        !((int) letter == 45) &&
        !((int) letter == 95))
        {
            return true;
        }
    }
    
    return false;
}

dayReport* readFile(string fileName){
    
    ifstream file("../tests/" + fileName);
    string line;
    dayReport* report = new dayReport;

    if (file.is_open())
    {
        getline(file, line);        // number of tables
        report->tables = stoi(line);

        getline(file, line);                    // start end of comp-club work
        if (timeCheck(line.substr(0, line.find(" ")))){
            cout << line << "\n";
            return nullptr;
        }
        report->start = strToSec(line.substr(0, line.find(" ")));

        if (timeCheck(line.substr(line.find(" ") + 1))){
            cout << line << "\n";
            return nullptr;
        }
        report->end = strToSec(line.substr(line.find(" ") + 1));

        getline(file, line);                    // price of each hour
        report->price = stoi(line);

        while (getline(file, line)){
            if (timeCheck(line.substr(0, line.find(" ")))){
                cout << line << "\n";
                return nullptr;
            }
            long time = strToSec(line.substr(0, line.find(" ")));
            if (eventCheck(report, time)){
                cout << line << "\n";
                return nullptr;
            }
            int id = stoi(line.substr(6, 1));
            int tNum;
            string body;
            if (id == 2)                        // id 2 case with table number
            {
                int ls = line.rfind(" ");
                tNum = stoi(line.substr(ls));
                if ((tNum < 1) || (tNum > report->tables)){
                    cout << line << "\n";
                    return nullptr;
                }

                body = line.substr(8, ls - 8);
                
                if (nameCheck(body)){
                    cout << line << "\n";
                    return nullptr;
                }
                report->notes.push_back(createNote(time, id, body, tNum));
            }
            else                                // otherwise
            {
                string body = line.substr(8);
                if (nameCheck(body)){
                    cout << line << "\n";
                    return nullptr;
                }
                report->notes.push_back(createNote(time, id, body));
            }
        }
    }
    else {
        cout << "cannot open the file!\n";
        file.close();
        return nullptr;
    }

    file.close();

    return report;
}


string convTime(unsigned time){         // transforms seconds into "HH:MM" format
    
    int hour = floor(time / 3600.);
    int min = time / 60 - hour * 60;
    string h = to_string(hour).size() == 1 ? "0" + to_string(hour) : to_string(hour);
    string m = to_string(min).size() == 1 ? "0" + to_string(min) : to_string(min);
    return  h + ":" + m;
}


void eventOut(unsigned time, int id, string eventBody, string table = "")
{
    cout << convTime(time) << " " << id << " " << eventBody << " " << table << "\n";
}

void event1(Note* note, map<string, int> &clientsInClub, unsigned start, unsigned end){
    
    if ((note->time < start) || (note->time > end))
    {
        eventOut(note->time, 13, "NotOpenYet");
        return;
    }
    else if (clientsInClub.count(note->eventBody))
    {
        eventOut(note->time, 13, "YouShallNotPass");
        return;
    }
    clientsInClub[note->eventBody] = note->tableNum;
}

void event2(Note* note, map<string, int> &clientsInClub, map<int, string> &busyTable, vector<tableRevenue*> &tableUsingTime, int price){

    if (!clientsInClub.count(note->eventBody))
    {
        eventOut(note->time, 13, "ClientUnknown");
        return;
    }
    else if (busyTable.count(note->tableNum))
    {
        eventOut(note->time, 13, "PlaceIsBusy");
        return;
    }

    string client = note->eventBody;        // client's name
    int cur_table = clientsInClub[client];  // current table client's sitting at the moment (in case clients isn't sitting there would be zero)
    if (busyTable.count(cur_table))         // if some table was occupied by this client
    {
        busyTable.erase(cur_table);         // it is not anymore
        int tmp = note->time - tableUsingTime[cur_table - 1]->start;
        tableUsingTime[cur_table - 1]->total += tmp;    //!!!!!     the value of total_time_used recalculated
        tableUsingTime[cur_table - 1]->revenue += (price * ceil(tmp / 3600.));
    }    
    

    busyTable[note->tableNum] = note->eventBody;        // new table is occupied by client
    clientsInClub[client] = note->tableNum;             // client takes new table
    tableUsingTime[note->tableNum - 1]->start = note->time;  // start the using timer for the table
}

void event3(Note* note, map<string, int> &clientsInClub, map<int, string> &busyTable, queue<string> &queue, int tables){

    if (busyTable.size() != tables)
    {
        eventOut(note->time, 13, "ICanWaitNoLonger!");
        return;
    }                                                               // the client wants to get in line
    else if (queue.size() + 1 > tables)                             // as if client would get in line
    {
        clientsInClub.erase(note->eventBody);
        eventOut(note->time, 11, note->eventBody);
        return;
    }

    queue.push(note->eventBody);
}

void event4(Note* note, map<string, int> &clientsInClub, queue<string> &queue, map<int, string> &busyTable, vector<tableRevenue*> &tableUsingTime, int price){

    if (!clientsInClub.count(note->eventBody))
    {
        eventOut(note->time, 13, "ClientUnknown");
        return;
    }

    string client = note->eventBody;        // client name
    int table = clientsInClub[client];  // current table client's sitting at the moment (in case clients isn't sittin' there would be zero)
    clientsInClub.erase(client);            // client leaving
    busyTable.erase(table);             // table is not occupied
    int tmp = note->time - tableUsingTime[table - 1]->start;
    tableUsingTime[table - 1]->total += tmp;                                        //  the value of total_time_used recalculating
    tableUsingTime[table - 1]->revenue += (price * ceil(tmp / 3600.));     //  recalculating the revenue

    if (!queue.empty())
    {
        client = queue.front();     // first client of the queue
        queue.pop();                // is not in the queue anymore
        busyTable[table] = client;
        clientsInClub[client] = table;
        tableUsingTime[table - 1]->start = note->time;  // start the using timer for the table

        eventOut(note->time, 12, client, to_string(table));
    }
}

void calculate(dayReport* report){

    queue<string> clientsQueue;
    map<int, string> busyTable;                 // table number , client's name
    map<string, int> clientsInClub;             // client's name, table number
    vector<tableRevenue*> tableUsingTime;       // index = table_num - 1: stores usage info for each table

    for (int i = 0; i < report->tables; i++){
        tableRevenue* table = new tableRevenue;
        tableUsingTime.push_back(table);
    }

    cout << convTime(report->start) << "\n";
    
    for (Note* event : report->notes){
        
        string table = (event->tableNum != 0) ? to_string(event->tableNum) : "";
        cout << convTime(event->time) << " " << event->eventID << " " << event->eventBody << " " << table << "\n";
        
        if (event->eventID == 1)
        {
            event1(event, clientsInClub, report->start, report->end);
        }
        else if(event->eventID == 2)
        {
            event2(event, clientsInClub, busyTable, tableUsingTime, report->price);
        }
        else if(event->eventID == 3)
        {
            event3(event, clientsInClub, busyTable, clientsQueue, report->tables);
        }
        else if(event->eventID == 4)
        {
            event4(event, clientsInClub, clientsQueue, busyTable, tableUsingTime, report->price);   
        }
    }

    for (auto client : clientsInClub)
    {
        eventOut(report->end, 11, client.first);

        int table = client.second;  // current table client's sitting at the moment (in case clients isn't sittin' there would be zero)

        if (table) 
        {
            int tmp = report->end - tableUsingTime[table - 1]->start;
            tableUsingTime[table - 1]->total += tmp;                         // the value of total_time_used recalculated
            tableUsingTime[table - 1]->revenue += (report->price * ceil(tmp / 3600.));    //  recalculating the revenue 
        }
    }

    cout << convTime(report->end) << "\n";

    for (int i = 0; i < tableUsingTime.size(); i++)
    {
        cout << i + 1 << " " << tableUsingTime[i]->revenue << " " << convTime(tableUsingTime[i]->total) << "\n";
    }

}


int main(int argc, char **argv){
    
    if ((argc == 2) && (strcmp(argv[1], "")))
    {
        string fileName = argv[1];
        dayReport* report = readFile(fileName);

        if (report)
        {
            calculate(report);

            for (Note* elem : report->notes){
                delete elem;
            }
        }
        delete report;
    }
    else
    {
        cout << "Filename error!\n";
    }
    return 0;
}