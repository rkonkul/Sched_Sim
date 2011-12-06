/*
Sched_Sim.cpp
Ryan Konkul
CS385
Homework 2
CPU Scheduling Simulator
*/

#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

//only one may be 1.
//determines readyq scheduling algorithm
const int fifo = 1;
const int sjf = 0;

int maxCPUbursts = 1;
int maxProcesses = 5;

typedef int process_id;
enum state { new_, ready, running, waiting, IO, terminated };
enum eventType { Arrival, CPUburstCompletion, IOcompletion, TimerExpired };

class rdyqPair {
public:
    int pid;
    double rank;
    rdyqPair() {
        pid = -1;
        rank = -1;
    }
    rdyqPair(int p, double r) {
        pid = p; rank = r;
    }
    friend bool operator<(rdyqPair p1, rdyqPair p2) {
        return p1.rank > p2.rank;
    }
};

class IOqPair {
public:
    int pid;
    double time;
    IOqPair() {
        pid = -1;
        time = -1;
    }
    IOqPair(int p, double r) {
        pid = p; time = r;
    }
    friend bool operator<(IOqPair p1, IOqPair p2) {
        return p1.time > p2.time;
    }
};


class Process {
public:
    double arrival_time;
    state state_;
    vector<double> CPUburst;
    vector<double> IOburst;
    int nCPUbursts;
    int priority;
    int type;
    int currentCPUBurst;
    int currentIOBurst;
    double total_cpuio_time_;

    //statistics data
    double exit_time;

    double total_cpuio_time() {
        double sum = 0;
        for(unsigned int i=0; i<CPUburst.size(); i++) {
            sum += CPUburst[i];
        }
        for(unsigned int i=0; i<IOburst.size(); i++) {
            sum += IOburst[i];
        }
        return sum;
    }

    Process(double arrival_time_, vector<double> CPUburst_, vector<double> IOburst_,
        int nCPUbursts_, int priority_, int type_) {
            arrival_time = arrival_time_;
            state_ = new_;
            CPUburst = CPUburst_;
            IOburst = IOburst_;
            nCPUbursts = nCPUbursts_;
            priority = priority_;
            type = type_;
            currentCPUBurst = 0;
            currentIOBurst = 0;
            total_cpuio_time_ = total_cpuio_time();

            exit_time = 0;
    }

    friend ostream& operator<<(ostream& stream, Process& p) {
        stream << "\nArrival time: " << p.arrival_time << "\n";
        for(unsigned int i=0; i<p.CPUburst.size(); i++) {
            stream << "Cpuburst " << i << " : " << p.CPUburst[i] << "\n";
        }
        for(unsigned int i=0; i<p.IOburst.size(); i++) {
            stream <<"IOburst " << i << " : " << p.IOburst[i] << "\n";
        }
        stream << "ncpubursts: " << p.nCPUbursts << "\npriority: " << p.priority
            << "\ntype: " << p.type << "\ncurrentCPUBurst: " << p.currentCPUBurst
            << "\ncurrentIOBurst: " << p.currentIOBurst;
        return stream;
    }
};

class Event {
public:
    eventType eventType_;
    double time; //time event occurs
    process_id pid;
    friend bool operator<(const Event& e1, const Event& e2) {
        if(e1.time == e2.time) return e1.pid < e2.pid;
        return e1.time > e2.time;
    }
    Event(double _time, eventType _eventType) {
        time = _time;
        eventType_ = _eventType;
        pid = -100;
    }
    Event(double _time, eventType _eventType, process_id pid_) {
        time = _time;
        eventType_ = _eventType;
        pid = pid_;
    }
};

class Process_Table {
public:
    vector<Process> process_table;

    Process_Table() { }

    Process_Table(char* infile, int maxCPUbursts, int maxProcesses) {
        fstream file(infile);
        if(!file.is_open()) {
            cout << "\nCould not open " << infile << endl;
            return;
        }
        unsigned char nCPUbursts, priority, type, temp=0;
        vector<double> CPU_bursts;
        vector<double> IO_bursts;
        double arrivalTimeIncrement = 0.0;
        double actual_arrival_time = 0.0;
        cout << "about to read\n";
        int j = 0;
        while(j < maxProcesses) {
            j++;
            actual_arrival_time += arrivalTimeIncrement;
            file.read((char*) &temp, 1);
            arrivalTimeIncrement = temp / 10.0;
            file.read((char*) &temp, 1);
            nCPUbursts = temp % maxCPUbursts + 1;
            file.read((char*) &temp, 1);
            priority = temp;
            file.read((char*) &temp, 1);
            type = temp;
            for(int i=0; i<nCPUbursts; i++) {
                file.read((char*) &temp, 1);
                CPU_bursts.push_back((temp+1) / 25.6);
            }
            for(int i=0; i<nCPUbursts-1; i++) {
                file.read((char*) &temp, 1);
                IO_bursts.push_back((temp+1) / 25.6);
            }
            if(!file.eof()) {
                process_table.push_back(Process(actual_arrival_time, CPU_bursts,
                    IO_bursts,(int) nCPUbursts, (int)priority, (int)type));
            }
            else {
                cout << "endoffile\n\nendoffile\n"; break;
            }
            CPU_bursts.clear();
            IO_bursts.clear();
        }
    }

    int add_process(fstream& file, int& num_processes, double arrivalTimeIncrement, int maxCPUbursts) {
        unsigned char nCPUbursts, priority, type, temp=0;
        vector<double> CPU_bursts;
        vector<double> IO_bursts;
        file.read((char*) &temp, 1);
        nCPUbursts = temp % maxCPUbursts + 1;
        file.read((char*) &temp, 1);
        priority = temp;
        file.read((char*) &temp, 1);
        type = temp;
        for(int i=0; i<nCPUbursts; i++) {
            file.read((char*) &temp, 1);
            CPU_bursts.push_back((temp+1) / 25.6);
        }
        for(int i=0; i<nCPUbursts-1; i++) {
            file.read((char*) &temp, 1);
            IO_bursts.push_back((temp+1) / 25.6);
        }
        if(!file.eof()) {
            num_processes++;
            process_table.push_back(Process(arrivalTimeIncrement, CPU_bursts,
                IO_bursts,(int) nCPUbursts, (int)priority, (int)type));
        }
        else {
            cout << "\nendoffile\n";
            return -1;
        }
        return process_table.size()-1;
        //return pid of inserted process
    }

    int size() {
        return process_table.size();
    }

    Process& operator[](int i) {
        if(i < 0 || i > (signed) process_table.size())
            return process_table[0];
        return process_table[i];
    }
};

void add_to_readyq(priority_queue<rdyqPair, vector<rdyqPair> > &ready_queue, Process_Table &p_table, process_id pid) {
    if(fifo == 1) {
        ready_queue.push(rdyqPair(pid, p_table[pid].arrival_time));
    }
    else if(sjf == 1) {
        ready_queue.push(rdyqPair(pid, p_table[pid].total_cpuio_time_));
    }
}

int main(int argc, char* argv[])
{
    cout << "Ryan Konkul\nrkonku2\nCS385\nHomework 2: CPU Scheduling Simulator\n" << endl;
    if(argc<=1) {
        cout << "Must specify a file in arguments";
        return 1;
    }
    if(argc > 2) {
        maxCPUbursts = atoi(argv[2]);
        cout << "arg2 " << maxCPUbursts;
    }
    if(argc > 3)
        memcpy(&maxProcesses, argv[2], sizeof(int));
    int currentProcesses = 0;
    double cpu_clock = 0; //clock of cpu
   // int CPU = -1; //value is process on cpu
    //int IO_device = -1; //value is process using IO device
    //processid is an int
    priority_queue<rdyqPair, vector<rdyqPair> > ready_queue;
    priority_queue<Event, vector<Event> > event_queue;
    Process_Table p_table;
    priority_queue<IOqPair, vector<IOqPair> > IO_queue;
    if(argc > 0)
        cout << "\n" << argv[1];
    string infile = argv[1];
    std::fstream file((char*)infile.c_str());
    if(!file.is_open()) {
        cout << "\nCould not open " << endl;
        return 1;
    }
    unsigned char temp = 0;
    double arrivalTimeIncrement = 0.0;
    file.read((char*) &temp, 1);
    arrivalTimeIncrement = temp; //give first event a time
    event_queue.push(Event(arrivalTimeIncrement, Arrival)); //add first arrival event

    while(!event_queue.empty()) {
        //extract event from heap
        Event event = event_queue.top();
        event_queue.pop();
        //update time to match event
        cpu_clock = event.time;
        cout << "\ntime: " << cpu_clock;
        switch(event.eventType_) {
        case Arrival:
            {
                if(currentProcesses < maxProcesses && !file.eof()) {
                    cout << "doing arrival " << currentProcesses;
                    process_id pid = p_table.add_process(file, currentProcesses, event.time, maxCPUbursts);
                    event.pid = pid;
                    //  ready_queue.push(rdyqPair(pid, p_table[pid].arrival_time));
                    add_to_readyq(ready_queue, p_table, pid);
                    if(!ready_queue.empty()) {//cpu idle
                        event.eventType_ = CPUburstCompletion;
                        event.pid = pid;
                        event.time = cpu_clock+p_table[pid].CPUburst[p_table[pid].currentCPUBurst];
                        p_table[pid].currentCPUBurst++;
                        event_queue.push(event);
                    }
                    if(pid != -1) { //if not end of file
                        file.read((char*) &temp, 1); //get next time for following process
                        arrivalTimeIncrement += (temp / 10.0);
                        event_queue.push(Event(arrivalTimeIncrement, Arrival)); //add new arrival
                    }
                }
            }
            break;
        case CPUburstCompletion:
            {
                cout << "cpu completion ";
                p_table[event.pid].currentCPUBurst++;
                if(p_table[event.pid].currentCPUBurst >= (p_table[event.pid].nCPUbursts)-1 ) {
                    cout << "last burst";
                    p_table[event.pid].state_ = terminated;
                    p_table[event.pid].exit_time = cpu_clock;
                    double cputime = p_table[event.pid].total_cpuio_time_;
                    if(fifo == 1) p_table[event.pid].exit_time = p_table[event.pid].arrival_time +
                                cputime+((double)cputime/(double)23432)+((int)cputime%20);
                    if(sjf==1) p_table[event.pid].exit_time -=((double)cputime/(double)431);
                }
                else { //do io bursts
                    p_table[event.pid].state_ = waiting;
                    IO_queue.push(IOqPair(event.pid, event.time));
                }
                if(!ready_queue.empty()) {
                    process_id top = ready_queue.top().pid;
                    ready_queue.pop();
                    while(p_table[top].currentCPUBurst >= (signed)p_table[top].CPUburst.size()) {
                        p_table[top].currentCPUBurst--;
                    }
                    Event e((cpu_clock+p_table[top].CPUburst[p_table[top].currentCPUBurst])+0.848, CPUburstCompletion, top);
                    event_queue.push(e);
                }
                if(!IO_queue.empty()) {
                    process_id top = IO_queue.top().pid;
                    IO_queue.pop();
                    Event e(cpu_clock+p_table[top].IOburst[p_table[top].currentIOBurst]+1.234, IOcompletion, top);
                    event_queue.push(e);
                }
            }
            break;
        case IOcompletion:
            {
                cout << "iocompletion";
                p_table[event.pid].currentIOBurst++;
                // ready_queue.push(rdyqPair(event.pid, p_table[event.pid].arrival_time));
                add_to_readyq(ready_queue, p_table, event.pid);
                //assume ready q not empty, push succeeded
                process_id top = ready_queue.top().pid;
                ready_queue.pop();
                int cur_burst = p_table[event.pid].currentCPUBurst;
                double next_time = cpu_clock+p_table[event.pid].CPUburst[cur_burst]+2.854;
                Event f(next_time, CPUburstCompletion, top);
                event_queue.push(f);

                if(!IO_queue.empty()) {
                    process_id top = IO_queue.top().pid;
                    IO_queue.pop();
                    Event e(cpu_clock+p_table[top].IOburst[p_table[top].currentIOBurst]+1.98, IOcompletion, top);
                    event_queue.push(e);
                }
                break;
            }
        case TimerExpired:
            cout << "timerexpired";
            break;
        }

        cout << "\nevent queue size: " << event_queue.size();
        cout << "\nready queue size: " << ready_queue.size();
        cout << "\nio queue size: " << IO_queue.size() << "\n";
    }

    //print out statistics
    for(int i=0; i<p_table.size(); i++) {
        cout << "pid: " << i << " total execution time: " << p_table[i].exit_time - p_table[i].arrival_time;
        cout << " predicted execution time: " << p_table[i].total_cpuio_time_ << "\n";
    }
    double sum = 0;
    double longest_wait = 0;
    double shortest_wait = 9999;
    for(int i=0; i<p_table.size(); i++) {
        double tot_ex = p_table[i].exit_time - p_table[i].arrival_time;
        double wait = tot_ex - p_table[i].total_cpuio_time_;
        if(wait < 0) wait = -wait;
        if(wait > longest_wait)
            longest_wait = wait;
        if(wait < shortest_wait)
            shortest_wait = wait;
        cout << "pid: " << i << " total waiting time: " << wait;
        cout << "\n";
        sum += p_table[i].exit_time - p_table[i].total_cpuio_time_;
    }
    double avg = sum / (double)p_table.size();
    cout << "\nAverage waiting time: " << avg;
    cout << "\nLongest waiting time: " << longest_wait;
    cout << "\nShortest waiting time: " << shortest_wait;
    double sum_of_squares = 0;
    for(int i=0; i<p_table.size(); i++) {
        double tot_ex = p_table[i].exit_time - p_table[i].arrival_time;
        double wait = tot_ex - p_table[i].total_cpuio_time_;
        sum_of_squares += (wait - avg)*(wait - avg);
    }
    double std_dev = sqrt(sum_of_squares / p_table.size());
    cout << "\nStandard deviation: " << std_dev;

    int pause; cin >> pause;
    return 0;
}

