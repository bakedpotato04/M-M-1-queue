#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>
#include <cstdint>

#ifndef HW3_SOLUTION_H_
#define HW3_SOLUTION_H_


// Definition of a Queue Node including arrival and service time
struct ElementQueueNode {
    double arrival_time;  // customer arrival time, measured from time t=0, inter-arrival times exponential
    double service_time;  // customer service time (exponential) 
};

// Definition of a Queue Node in the Event Queue
struct EventQueueNode {
    double event_time; // event start time
    int event_type;   // Event type. 1: Arrival; 2: Start Service; 3: Departure
    ElementQueueNode* qnode;  // pointer to corresponding element in the Element Queue
    struct EventQueueNode *next;  // pointer to next event
};

// Class including all elements in the simulation 
class ElementQueue {
	public:
		ElementQueue(int seed, double lambda, double mu, int total_departures) {
			ElementArray = new ElementQueueNode[total_departures];
			if (ElementArray == nullptr) {
       			printf("Failed to allocate memory for all required departures\n");
        		exit(1);
    		}
			size = total_departures;
			current = 0; // current element is the first element in the element array
			InitializeQueue(seed, lambda, mu, total_departures);  // create nodes in the queue based on arrival and service distributions
		};
		~ElementQueue() {
			delete[] ElementArray;
			ElementArray = nullptr;
        };
		ElementQueueNode* GetCurrentElement() {
			return &ElementArray[current];
		};
		ElementQueueNode* GetElementAtIndex(uint64_t index) {
			if (index < size)
				return &ElementArray[index];
			else	
				return nullptr;
		};
		ElementQueueNode* AdvanceToNextElement() {
			if (current < (size - 1)) {
				current++;
				return &ElementArray[current];
			} 
			else
				return nullptr; 
		};
	private:
		void InitializeQueue(int seed, double lambda, double mu, int total_departures);
		uint64_t size;  // total size of element queue
		uint64_t current; // Point to the current node being processed for arrival event
		ElementQueueNode* ElementArray;  // Array containing all elements, created at the beginning of simulation
};

// Event Queue for events that have been scheduled, implemented as a priority queue
class EventQueue {
	public:
		EventQueue() {
			head = nullptr;
			tail = nullptr; 
			num_events = 0;
		};
		~EventQueue() {
            while (head != nullptr) {
                EventQueueNode* tmp = head;
                head = head->next;  
                delete(tmp); 
            }
		}; 
		EventQueueNode* CreateEvent(double etime, int etype, ElementQueueNode* qnode) {
			EventQueueNode* node = new(EventQueueNode); 
  			node->event_time = etime;
  			node->event_type = etype;
  			node->qnode = qnode;
			node->next = nullptr;
			return node;
		};
		void ScheduleEvent(double etime, int etype, ElementQueueNode* qnode);
		EventQueueNode* GetNextEvent();
		bool IsEmpty() {
			return (num_events == 0);
		};
		void RemoveEvent(EventQueueNode* node) {   // free memory of node
			assert (node != head);  // can't remove head of event queue
			delete node;
		};
		void PrintEventQueue() {   // Use this for debugging purposes
			EventQueueNode* node;
  			printf("EventQ = ");
  			for (node = head; node; node = node->next)
    			printf("(%f,%d) ", node->event_time, node->event_type);
  			printf("\n");
		};

	private:
	    EventQueueNode* head;
    	EventQueueNode* tail;
		uint64_t num_events; 
};

class Simulation {
	public:
		Simulation(int seed_in, double lambda_in, double mu_in, int print_period_in, int total_departures_in) {
			lambda = lambda_in;
			mu = mu_in;
			seed = seed_in;
			print_period = print_period_in;
			total_departures = total_departures_in;
			GenerateComputedStatistics();
			ElementQ = new ElementQueue(seed, lambda, mu, total_departures);
			EventQ = new EventQueue();
			departure_count = 0;
			current_time = 0;
			current_time = 0;
			prev_time = 0;
			n = 0;
			server_busy = 0;  
			next_departure_time = DBL_MAX; 
			waiting_count = 0;
			cumulative_response = 0;
			cumulative_waiting = 0;
			cumulative_idle_times = 0;
			cumulative_number = 0;
		};
		~Simulation() {
			delete ElementQ;
			delete EventQ;
		};
		void ProcessArrival(ElementQueueNode* arriving_node);
		void StartService(ElementQueueNode* served_node);
		void ProcessDeparture(ElementQueueNode* departing_node);
		void GenerateComputedStatistics();
		void RunSimulation();

		// This function should be called to print periodic and/or end-of-simulation statistics
		void PrintStatistics(int departure_count, int print_period, double lambda) {
			simulated_stats[0] = cumulative_number / current_time;  // simulated mean number
			simulated_stats[1] = cumulative_response/ departure_count; // simulated mean response time
			simulated_stats[2] = cumulative_waiting / departure_count; // simulated mean waiting time
			simulated_stats[3] = cumulative_idle_times / current_time; // simulated p0
  			if(departure_count == total_departures) 
				printf("End of Simulation - after %d departures\n", departure_count);
  			else printf("After %d departures\n", departure_count);

  			printf("Mean n = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[0], computed_stats[0]);
  			printf("Mean r = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[1], computed_stats[1]);
  			printf("Mean w = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[2], computed_stats[2]);
  			printf("p0 = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[3], computed_stats[3]);
		};
	private:
		ElementQueue* ElementQ;     // Element Queue for all elements used in simulation
		EventQueue* EventQ;         // Event Queue for events to be scheduled 	
		double lambda;             	// same as input
		double mu;             		// same as input
		int seed;                   // same as input
		int total_departures;       // same as input
		int print_period; 	        // same as input
		double computed_stats[4];   // Store computed statistics: [E(n), E(r), E(w), p0]
		double simulated_stats[4];  // Store simulated statistics [n, r, w, sim_p0]
		int arrival_count;          // current number of arrivals from queue
		int departure_count;        // current number of departures from queue
		double current_time;        // current time during simulation
		double prev_time;           // Time of previous event
		int n;                      // current number of customers in system
		int server_busy;            // Current state of server, 0(free), 1(busy)
		double next_departure_time; // next departure time, initially set to infinite time in future
		ElementQueueNode* next_arr; // Point to next element to arrive
    	ElementQueueNode* next_srv; // Point to next element to be served
		int waiting_count;          // Current number of customers waiting for service

    	double cumulative_response;   // Accumulated response time for all effective departures
    	double cumulative_waiting;    // Accumulated waiting time for all effective departures
    	double cumulative_idle_times; // Accumulated times when the system is idle, i.e., no customers in the system
    	double cumulative_number;     // Accumulated number of customers in the system
};

#endif